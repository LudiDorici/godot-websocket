#ifndef ESWSCLIENT_H
#define ESWSCLIENT_H

#ifdef JAVASCRIPT_ENABLED

#include "core/error_list.h"
#include "websocket_client.h"
#include "esws_peer.h"

class ESWSClient : public WebSocketClient {

	GDCIIMPL(ESWSClient, WebSocketClient);

	Ref<ESWSPeer> peer;
public:

	Error connect_to_host(String p_host, uint16_t p_port, PoolVector<String> p_protocol = PoolVector<String>());
	Ref<WebSocketPeer> get_peer() const;
	bool is_connected_to_host() const;
	bool is_connecting_to_host() const;
	void disconnect_from_host();
	IP_Address get_connected_host() const;
	uint16_t get_connected_port() const;
	virtual void poll();
	void hail(){WARN_PRINTS("Hello there, my friend!");}
	ESWSClient();
	~ESWSClient();

};

#endif // JAVASCRIPT_ENABLED

#endif // ESWSCLIENT_H
