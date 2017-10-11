#ifndef WEBSOCKET_HELPER_H
#define WEBSOCKET_HELPER_H

#include "core/io/stream_peer.h"
#include "core/os/os.h"
#include "core/reference.h"
#include "core/ring_buffer.h"
#include "websocket_peer.h"

static int _lws_gd_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

class WebSocketHelper : public Reference {

	GDCLASS(WebSocketHelper, Reference);

protected:
	struct lws_context *context;
	bool is_polling;
	bool free_context;
	PoolVector<CharString> protocol_names;
	PoolVector<struct lws_protocols> protocol_structs;
	CharString protocol_string;

	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("poll"), &WebSocketHelper::poll);
		ClassDB::bind_method(D_METHOD("get_protocols"), &WebSocketHelper::get_protocols);
	};

	/*
	 * prepare the protocol_structs to be fed to context
	 * also prepare the protocol string used by the client
	 */
	void _make_protocols(PoolVector<String> names) {
		int i;
		int len = names.size();
		size_t data_size = sizeof(struct WebSocketPeer::PeerData);
		protocol_structs.resize(len + 2);
		protocol_names.resize(len);

		// set the protocol string for client
		protocol_string = names.join(",").ascii();

		PoolVector<struct lws_protocols>::Write psw = protocol_structs.write();
		PoolVector<String>::Read pnr = names.read();
		PoolVector<CharString>::Write pnw = protocol_names.write();

		// the first protocol is always http-only
		psw[0].name = "http-only";
		psw[0].callback = _lws_gd_callback;
		psw[0].per_session_data_size = data_size;
		psw[0].rx_buffer_size = 0;
		// add user defined protocols
		for (i = 0; i < len; i++) {
			// the input strings might go away after this call,
			// we need to copy them. Will clear them when
			// detroying the context
			pnw[i] = pnr[i].ascii();
			psw[i + 1].name = pnw[i].get_data();
			psw[i + 1].callback = _lws_gd_callback;
			psw[i + 1].per_session_data_size = data_size;
			psw[i + 1].rx_buffer_size = 0;
		}
		// add protocols terminator
		psw[len + 1].name = NULL;
		psw[len + 1].callback = NULL;
		psw[len + 1].per_session_data_size = 0;
		psw[len + 1].rx_buffer_size = 0;
	}

	void destroy_context() {
		if (context == NULL)
			return;

		if (is_polling) {
			free_context = true;

		} else {
			struct lws_context *tmp = context;
			context = NULL;
			lws_context_destroy(tmp);
			free_context = false;
			protocol_structs.resize(0);
			protocol_names.resize(0);
			protocol_string.resize(0);
		}
	}

public:
	virtual int _handle_cb(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) = 0;

	void poll() {
		ERR_FAIL_COND(context == NULL);

		is_polling = true;
		lws_service(context, 0);
		is_polling = false;
		if (free_context) {
			destroy_context();
		}
	}

	PoolVector<String> get_protocols() {
		int i = 0;
		PoolVector<String> out;
		for (i = 0; i < protocol_names.size(); i++) {
			out.append(String(protocol_names[i].get_data()));
		}
		return out;
	}

	WebSocketHelper() {}
	~WebSocketHelper() { destroy_context(); }
};

static int _lws_gd_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {

	if (wsi == NULL) {
		return 0;
	}

	WebSocketHelper *helper = (WebSocketHelper *)lws_context_user(lws_get_context(wsi));
	return helper->_handle_cb(wsi, reason, user, in, len);
}

#endif // WEBSOCKET_HELPER_H
