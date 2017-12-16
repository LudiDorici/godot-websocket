#include "websocket_multiplayer.h"

WebSocketMultiplayerPeer::WebSocketMultiplayerPeer() {

	_is_multiplayer = false;
	_peer_id = 0;
	_target_peer = 0;
	_refusing = false;
}

WebSocketMultiplayerPeer::~WebSocketMultiplayerPeer() {

}

void WebSocketMultiplayerPeer::_bind_methods() {

	ClassDB::bind_method(D_METHOD("get_peer", "peer_id"), &WebSocketMultiplayerPeer::get_peer);
}

//
// PacketPeer
//
int WebSocketMultiplayerPeer::get_available_packet_count() const {

	ERR_FAIL_COND_V(!_is_multiplayer, ERR_UNCONFIGURED);
	ERR_FAIL_COND_V(!get_peer(_target_peer).is_valid(), 0);

	return get_peer(_target_peer)->get_available_packet_count();
}

int WebSocketMultiplayerPeer::get_max_packet_size() const {

	ERR_FAIL_COND_V(!_is_multiplayer, ERR_UNCONFIGURED);
	ERR_FAIL_COND_V(!get_peer(_target_peer).is_valid(), ERR_UNCONFIGURED);

	return get_peer(_target_peer)->get_max_packet_size() - 12;
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
	ERR_FAIL_COND_V(!get_peer(_target_peer).is_valid(), ERR_UNCONFIGURED);

	PoolVector<uint8_t> buffer;
	buffer.resize(p_buffer_size+12);
	{
		uint32_t from = get_unique_id();
		uint32_t type = 0; // 0 is payload data
		PoolVector<uint8_t>::Write bw = buffer.write();
		copymem(&bw[0], &from, 4);
		copymem(&bw[4], &_target_peer, 4);
		copymem(&bw[8], &type, 4);
		copymem(&bw[12], p_buffer, p_buffer_size);
	}
	return get_peer(_target_peer)->put_packet(&(buffer.read()[0]), p_buffer_size);

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

bool WebSocketMultiplayerPeer::is_server() const {

	return _peer_id == 1;
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

void WebSocketMultiplayerPeer::_process_multiplayer(Ref<WebSocketPeer> p_peer) {

	ERR_FAIL_COND(p_peer.is_valid());

	const uint8_t *in_buffer;
	int size = 0;

	Error err = p_peer->get_packet(&in_buffer, size);

	ERR_FAIL_COND(err != OK);
	ERR_FAIL_COND(size < 12);

	uint32_t type = 0;
	uint32_t from = 0;
	uint32_t to = 0;
	copymem(&type, in_buffer, 4);
	copymem(&from, &in_buffer[4], 4);
	copymem(&to, &in_buffer[8], 4);
	if (type == 0) { // payload
		Packet packet;
		packet.data = (uint8_t *)memalloc(size-12);
		packet.size = size;
		packet.source = from;
		packet.destination = to;
		_incoming_packets.push_back(packet);
	}
}

void WebSocketMultiplayerPeer::_on_peer_packet(int p_peer_id) {

	if (_is_multiplayer) {
		// handle multiplayer here
		_process_multiplayer(get_peer(p_peer_id));
	} else {
		emit_signal("data_received");
	}
}

void WebSocketMultiplayerPeer::_on_connect(String p_protocol) {

	if (_is_multiplayer) {
		emit_signal("connection_succeeded");
	} else {
		emit_signal("connection_established", p_protocol);
	}
}

void WebSocketMultiplayerPeer::_on_disconnect() {

	if (_is_multiplayer) {
		emit_signal("connection_failed");
	} else {
		emit_signal("connection_closed");
	}
}

void WebSocketMultiplayerPeer::_on_error() {

	if (_is_multiplayer) {
		emit_signal("connection_failed");
	} else {
		emit_signal("connection_error");
	}
}
