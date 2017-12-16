#include "websocket_multiplayer.h"

WebSocketMultiplayerPeer::WebSocketMultiplayerPeer() {

	_is_multiplayer = false;
	_peer_id = 0;
	_target_peer = 0;
	_refusing = false;

	_current_packet.source = 0;
	_current_packet.destination = 0;
	_current_packet.size = 0;
	_current_packet.data = NULL;
}

WebSocketMultiplayerPeer::~WebSocketMultiplayerPeer() {

	_clear();
}

void WebSocketMultiplayerPeer::_clear() {

	_peer_map.clear();
	if (_current_packet.data != NULL)
		memfree(_current_packet.data);

	for (List<Packet>::Element *E = _incoming_packets.front(); E; E = E->next()) {
		memfree(E->get().data);
		E->get().data = NULL;
	}

	_incoming_packets.clear();
}

void WebSocketMultiplayerPeer::_bind_methods() {

	ClassDB::bind_method(D_METHOD("get_peer", "peer_id"), &WebSocketMultiplayerPeer::get_peer);
}

//
// PacketPeer
//
int WebSocketMultiplayerPeer::get_available_packet_count() const {

	ERR_FAIL_COND_V(!_is_multiplayer, ERR_UNCONFIGURED);

	return _incoming_packets.size();
}

int WebSocketMultiplayerPeer::get_max_packet_size() const {

	ERR_FAIL_COND_V(!_is_multiplayer, ERR_UNCONFIGURED);

	return MAX_PACKET_SIZE;
}

Error WebSocketMultiplayerPeer::get_packet(const uint8_t **r_buffer, int &r_buffer_size) {

	r_buffer_size = 0;
	ERR_FAIL_COND_V(!_is_multiplayer, ERR_UNCONFIGURED);

	if (_current_packet.data != NULL) {
		memfree(_current_packet.data);
		_current_packet.data = NULL;
	}

	_current_packet = _incoming_packets.front()->get();
	_incoming_packets.pop_front();

	*r_buffer = _current_packet.data;
	r_buffer_size = _current_packet.size;

	return OK;
}

Error WebSocketMultiplayerPeer::put_packet(const uint8_t *p_buffer, int p_buffer_size) {

	ERR_FAIL_COND_V(!_is_multiplayer, ERR_UNCONFIGURED);
	ERR_FAIL_COND_V(!get_peer(_target_peer).is_valid(), ERR_DOES_NOT_EXIST);

	PoolVector<uint8_t> buffer;
	uint32_t size = PROTO_SIZE + p_buffer_size;
	buffer.resize(size);
	{
		uint8_t type = 0; // 0 is payload data
		uint32_t from = get_unique_id();
		PoolVector<uint8_t>::Write bw = buffer.write();
		copymem(&bw[0], &type, 1);
		copymem(&bw[1], &from, 4);
		copymem(&bw[5], &_target_peer, 4);
		copymem(&bw[PROTO_SIZE], p_buffer, p_buffer_size);
	}
	return get_peer(_target_peer)->put_packet(&(buffer.read()[0]), size);

}

//
// NetworkedMultiplayerPeer
//
void WebSocketMultiplayerPeer::set_transfer_mode(TransferMode p_mode) {

	// Websocket uses TCP, reliable
}

void WebSocketMultiplayerPeer::set_target_peer(int p_target_peer) {

	_target_peer = p_target_peer;
}

int WebSocketMultiplayerPeer::get_packet_peer() const {

	ERR_FAIL_COND_V(!_is_multiplayer, 1);
	ERR_FAIL_COND_V(_incoming_packets.size() == 0, 1);

	return _incoming_packets.front()->get().source;
}

int WebSocketMultiplayerPeer::get_unique_id() const {

	return _peer_id;
}

void WebSocketMultiplayerPeer::set_refuse_new_connections(bool p_enable) {

	_refusing = p_enable;
}

bool WebSocketMultiplayerPeer::is_refusing_new_connections() const {

	return _refusing;
}

void WebSocketMultiplayerPeer::_send_sys(Ref<WebSocketPeer> p_peer, uint8_t p_type, uint32_t p_peer_id) {

	ERR_FAIL_COND(!p_peer.is_valid());
	ERR_FAIL_COND(!p_peer->is_connected_to_host());

	PoolVector<uint8_t> message;
	uint32_t from = 1;
	uint32_t to = 0;
	message.resize(SYS_PACKET_SIZE);
	{
		PoolVector<uint8_t>::Write mw = message.write();
		copymem(&mw[0], &p_type, 1);
		copymem(&mw[1], &from, 4);
		copymem(&mw[5], &to, 4);
		copymem(&mw[PROTO_SIZE], &p_peer_id, 4);
	}
	p_peer->put_packet(&(message.read()[0]), SYS_PACKET_SIZE);
}

void WebSocketMultiplayerPeer::_send_add(uint32_t p_peer_id) {

	for (Map<int, Ref<WebSocketPeer> >::Element *E = _peer_map.front(); E; E = E->next()) {
		uint32_t id = E->key();
		if (p_peer_id == id)
			_send_sys(get_peer(id), SYS_ID, p_peer_id);
		else
			_send_sys(get_peer(id), SYS_ADD, p_peer_id);
	}
}

void WebSocketMultiplayerPeer::_send_del(uint32_t p_peer_id) {
	for (Map<int, Ref<WebSocketPeer> >::Element *E = _peer_map.front(); E; E = E->next()) {
		uint32_t id = E->key();
		if (p_peer_id != id)
			_send_sys(get_peer(id), SYS_DEL, p_peer_id);
	}
}

void WebSocketMultiplayerPeer::_process_multiplayer(Ref<WebSocketPeer> p_peer) {

	ERR_FAIL_COND(!p_peer.is_valid());

	const uint8_t *in_buffer;
	int size = 0;

	Error err = p_peer->get_packet(&in_buffer, size);

	ERR_FAIL_COND(err != OK);
	ERR_FAIL_COND(size < PROTO_SIZE);

	uint8_t type = 0;
	uint32_t from = 0;
	uint32_t to = 0;
	copymem(&type, in_buffer, 1);
	copymem(&from, &in_buffer[1], 4);
	copymem(&to, &in_buffer[5], 4);
	size -= PROTO_SIZE;
	if (type == 0) { // payload
		Packet packet;
		packet.data = (uint8_t *)memalloc(size);
		packet.size = size;
		packet.source = from;
		packet.destination = to;
		copymem(packet.data, &in_buffer[PROTO_SIZE], size);
		_incoming_packets.push_back(packet);
	} else if (!is_server()) {

		ERR_FAIL_COND(size < 4);
		int id = 0;
		copymem(&id, &in_buffer[PROTO_SIZE], 4);

		switch(type) {

			case 1: // Add peer
				_peer_map[id] = Ref<WebSocketPeer>();
				emit_signal("peer_connected", id);
				break;

			case 2: // Remove peer
				_peer_map.erase(id);
				emit_signal("peer_disconnected", id);
				break;
			case 3: // Helo, server assigned ID
				_peer_id = id;
				emit_signal("connection_succeeded");
				break;
			default:
				ERR_EXPLAIN("Invalid multiplayer message");
				ERR_FAIL();
				break;
		}
	}
}
