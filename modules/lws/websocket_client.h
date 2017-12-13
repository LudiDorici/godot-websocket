#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLINET_H

#include "core/error_list.h"
#include "websocket_peer.h"

class WebSocketClient : public Reference {

	GDCLASS(WebSocketClient, Reference);
	GDCICLASS(WebSocketClient);

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("poll"), &WebSocketClient::poll);
		ClassDB::bind_method(D_METHOD("get_peer"), &WebSocketClient::get_peer);
		ClassDB::bind_method(D_METHOD("is_connected_to_host"), &WebSocketClient::is_connected_to_host);
		ClassDB::bind_method(D_METHOD("is_connecting_to_host"), &WebSocketClient::is_connecting_to_host);
		ClassDB::bind_method(D_METHOD("connect_to_host", "host", "port", "protocols"), &WebSocketClient::connect_to_host, DEFVAL(PoolVector<String>()));
		ClassDB::bind_method(D_METHOD("disconnect_from_host"), &WebSocketClient::disconnect_from_host);

		ADD_SIGNAL(MethodInfo("data_received"));
		ADD_SIGNAL(MethodInfo("connection_established", PropertyInfo(Variant::STRING, "protocol")));
		ADD_SIGNAL(MethodInfo("connection_closed"));
		ADD_SIGNAL(MethodInfo("connection_error"));

	}

public:
	virtual void poll() = 0;
	virtual Error connect_to_host(String p_host, uint16_t p_port, PoolVector<String> p_protocol = PoolVector<String>()) = 0;
	virtual Ref<WebSocketPeer> get_peer() const = 0;
	virtual bool is_connected_to_host() const = 0;
	virtual bool is_connecting_to_host() const = 0;
	virtual void disconnect_from_host() = 0;
	virtual IP_Address get_connected_host() const = 0;
	virtual uint16_t get_connected_port() const = 0;

	WebSocketClient() {};
	~WebSocketClient() {};
};

#endif // WEBSOCKET_CLIENT_H
