/*************************************************************************/
/*  lws_server.cpp                                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                      GODOT WEBSOCKET MODULE                           */
/*            https://github.com/LudiDorici/godot-websocket              */
/*************************************************************************/
/* Copyright (c) 2017 Ludi Dorici, di Alessandrelli Fabio                */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#ifndef JAVASCRIPT_ENABLED

#include "lws_server.h"
#include "core/os/os.h"

Error LWSServer::listen(int p_port, PoolVector<String> p_protocols, bool gd_mp_api) {

	_is_multiplayer = gd_mp_api;

	struct lws_context_creation_info info;
	memset(&info, 0, sizeof info);

	stop();

	if (p_protocols.size() == 0) // default to binary protocol
		p_protocols.append(String("binary"));

	// Prepare lws protocol structs
	_make_protocols(p_protocols);
	PoolVector<struct lws_protocols>::Read pr = protocol_structs.read();

	info.port = p_port;
	info.user = get_lws_ref();
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
			int32_t id = _gen_unique_id();

			Ref<LWSPeer> peer = Ref<LWSPeer>(memnew(LWSPeer));
			peer->set_wsi(wsi);
			_peer_map[id] = peer;

			peer_data->peer_id = id;
			peer_data->in_size = 0;
			peer_data->in_count = 0;
			peer_data->out_count = 0;
			peer_data->rbw.resize(16);
			peer_data->rbr.resize(16);
			peer_data->force_close = false;

			_on_connect(id, lws_get_protocol(wsi)->name);
			break;
		}

		case LWS_CALLBACK_CLOSED: {
			int32_t id = peer_data->peer_id;
			if (_peer_map.has(id)) {
				_peer_map[id]->close();
				_peer_map.erase(id);
			}
			peer_data->in_count = 0;
			peer_data->out_count = 0;
			peer_data->rbr.resize(0);
			peer_data->rbw.resize(0);
			_on_disconnect(id);
			return 0; // we can end here
		}

		case LWS_CALLBACK_RECEIVE: {
			int32_t id = peer_data->peer_id;
			if (_peer_map.has(id)) {
				static_cast<Ref<LWSPeer> >(_peer_map[id])->read_wsi(in, len);
				if (_peer_map[id]->get_available_packet_count() > 0)
					_on_peer_packet(id);
			}
			break;
		}

		case LWS_CALLBACK_SERVER_WRITEABLE: {
			if (peer_data->force_close)
				return -1;

			int id = peer_data->peer_id;
			if (_peer_map.has(id))
				static_cast<Ref<LWSPeer> >(_peer_map[id])->write_wsi();
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

	_peer_map.clear();
	destroy_context();
	context = NULL;
}

bool LWSServer::has_peer(int p_id) const {
	return _peer_map.has(p_id);
}

Ref<WebSocketPeer> LWSServer::get_peer(int p_id) const {
	ERR_FAIL_COND_V(!has_peer(p_id), NULL);
	return _peer_map[p_id];
}

LWSServer::LWSServer() {
	context = NULL;
	_this_ref = NULL;
}

LWSServer::~LWSServer() {
	stop();
	invalidate_lws_ref();
}

#endif // JAVASCRIPT_ENABLED
