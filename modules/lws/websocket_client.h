#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLINET_H

#include "core/error_list.h"
#include "websocket_helper.h"
#include "websocket_peer.h"

class WebSocketClient : public WebSocketHelper {

	GDCLASS(WebSocketClient, WebSocketHelper);

private:
	Ref<WebSocketPeer> peer;

protected:
	static void _bind_methods();

	virtual int _handle_cb(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

public:
	Error connect_to_host(String p_host, uint16_t p_port, PoolVector<String> p_protocol = PoolVector<String>());
	Ref<WebSocketPeer> get_peer() const;
	bool is_connected_to_host() const;
	bool is_connecting_to_host() const;
	void disconnect_from_host();
	IP_Address get_connected_host() const;
	uint16_t get_connected_port() const;

	WebSocketClient();
	~WebSocketClient();
};

#endif // WEBSOCKET_CLIENT_H
