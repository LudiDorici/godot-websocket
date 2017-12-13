#include "lws_server.h"
#include "core/os/os.h"

uint32_t LWSServer::_gen_unique_id() const {

	uint32_t hash = 0;

	while (hash == 0 || hash == 1) {

		hash = hash_djb2_one_32(
				(uint32_t)OS::get_singleton()->get_ticks_usec());
		hash = hash_djb2_one_32(
				(uint32_t)OS::get_singleton()->get_unix_time(), hash);
		hash = hash_djb2_one_32(
				(uint32_t)OS::get_singleton()->get_data_path().hash64(), hash);
		hash = hash_djb2_one_32(
				(uint32_t)((uint64_t)this), hash); //rely on aslr heap
		hash = hash_djb2_one_32(
				(uint32_t)((uint64_t)&hash), hash); //rely on aslr stack
		hash = hash & 0x7FFFFFFF; // make it compatible with unsigned, since negatie id is used for exclusion
	}

	return hash;
}

Error LWSServer::listen(int p_port, PoolVector<String> p_protocols) {

	struct lws_context_creation_info info;
	memset(&info, 0, sizeof info);

	stop();

	if (p_protocols.size() == 0) // default to binary protocol
		p_protocols.append(String("binary"));

	// Prepare lws protocol structs
	_make_protocols(p_protocols);
	PoolVector<struct lws_protocols>::Read pr = protocol_structs.read();

	info.port = p_port;
	info.user = this;
	info.protocols = &pr[0];
	info.gid = -1;
	info.uid = -1;
	//info.ws_ping_pong_interval = 5;

	context = lws_create_context(&info);

	if (context != NULL)
		return OK;

	return FAILED;
}

bool LWSServer::is_listening() const {
	return context != NULL;
}

int LWSServer::_handle_cb(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {

	LWSPeer::PeerData *peer_data = (LWSPeer::PeerData *)user;

	switch (reason) {
		case LWS_CALLBACK_HTTP:
			// no http for now
			// closing immediately returning 1;
			return 1;

		case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
			// check header here?
			break;

		case LWS_CALLBACK_ESTABLISHED: {
			int id = _gen_unique_id();

			Ref<LWSPeer> peer = Ref<LWSPeer>(memnew(LWSPeer));
			peer->set_wsi(wsi);
			peer_map[id] = peer;

			peer_data->peer_id = id;
			peer_data->in_size = 0;
			peer_data->in_count = 0;
			peer_data->out_count = 0;
			peer_data->rbw.resize(16);
			peer_data->rbr.resize(16);
			peer_data->force_close = false;

			emit_signal("client_connected", peer_data->peer_id, lws_get_protocol(wsi)->name);
			break;
		}

		case LWS_CALLBACK_CLOSED: {
			int id = peer_data->peer_id;
			if (peer_map.has(id)) {
				peer_map[id]->close();
				peer_map.erase(id);
			}
			peer_data->in_count = 0;
			peer_data->out_count = 0;
			peer_data->rbr.resize(0);
			peer_data->rbw.resize(0);
			emit_signal("client_disconnected", id);
			return 0; // we can end here
		}

		case LWS_CALLBACK_RECEIVE: {
			int id = peer_data->peer_id;
			if (peer_map.has(id))
				peer_map[id]->read_wsi(in, len);
			emit_signal("data_received", id);
			break;
		}

		case LWS_CALLBACK_SERVER_WRITEABLE: {
			if (peer_data->force_close)
				return -1;

			int id = peer_data->peer_id;
			if (peer_map.has(id))
				peer_map[id]->write_wsi();
			break;
		}

		default:
			break;
	}

	return 0;
}

void LWSServer::stop() {
	if (context == NULL)
		return;

	peer_map.clear();
	destroy_context();
	context = NULL;
}

bool LWSServer::has_peer(int p_id) const {
	return peer_map.has(p_id);
}

Ref<WebSocketPeer> LWSServer::get_peer(int p_id) const {
	ERR_FAIL_COND_V(!has_peer(p_id), NULL);
	return peer_map[p_id];
}

LWSServer::LWSServer() {
	context = NULL;
	free_context = false;
	is_polling = false;
}

LWSServer::~LWSServer() {
	stop();
}
