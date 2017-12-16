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

public:

	Error connect_to_host(String p_host, String p_path, uint16_t p_port, bool p_ssl, PoolVector<String> p_protocol = PoolVector<String>());
	Ref<WebSocketPeer> get_peer(int p_peer_id) const;
	void disconnect_from_host();
	IP_Address get_connected_host() const;
	uint16_t get_connected_port() const;
	virtual ConnectionStatus get_connection_status() const;
	virtual void poll();

	LWSClient();
	~LWSClient();

};

#endif // JAVASCRIPT_ENABLED

#endif // LWSCLIENT_H
