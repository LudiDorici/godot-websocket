#ifndef LWSCLIENT_H
#define LWSCLIENT_H

#ifndef JAVASCRIPT_ENABLED

#include "core/error_list.h"
#include "websocket_client.h"
#include "lws_helper.h"
#include "lws_peer.h"

class LWSClient : public WebSocketClient {

	GDCIIMPL(LWSClient, WebSocketClient);

	LWS_HELPER(LWSClient);

private:
	Ref<LWSPeer> peer;

public:

	Error connect_to_host(String p_host, String p_path, uint16_t p_port, bool p_ssl, PoolVector<String> p_protocol = PoolVector<String>());
	Ref<WebSocketPeer> get_peer() const;
	bool is_connected_to_host() const;
	bool is_connecting_to_host() const;
	void disconnect_from_host();
	IP_Address get_connected_host() const;
	uint16_t get_connected_port() const;
	virtual void poll() { _lws_poll(); }

	LWSClient();
	~LWSClient();

};

#endif // JAVASCRIPT_ENABLED

#endif // LWSCLIENT_H
