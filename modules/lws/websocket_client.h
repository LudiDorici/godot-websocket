#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

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
		ClassDB::bind_method(D_METHOD("connect_to_url", "url", "protocols"), &WebSocketClient::connect_to_url, DEFVAL(PoolVector<String>()));
		ClassDB::bind_method(D_METHOD("disconnect_from_host"), &WebSocketClient::disconnect_from_host);

		ADD_SIGNAL(MethodInfo("data_received"));
		ADD_SIGNAL(MethodInfo("connection_established", PropertyInfo(Variant::STRING, "protocol")));
		ADD_SIGNAL(MethodInfo("connection_closed"));
		ADD_SIGNAL(MethodInfo("connection_error"));

	}

public:
	virtual void poll() = 0;
	virtual Error connect_to_host(String p_host, String p_path, uint16_t p_port, bool p_ssl, PoolVector<String> p_protocol = PoolVector<String>()) = 0;
	virtual Error connect_to_url(String p_url, PoolVector<String> p_protocols = PoolVector<String>()) {
		String host = p_url;
		String path = "/";
		int p_len = -1;
		int port = 80;
		bool ssl = false;
		if (host.begins_with("wss://")) {
			ssl = true; // we should implement this
			host = host.substr(6, host.length() - 6);
			port = 443;
		} else {
			ssl = false;
			if (host.begins_with("ws://"))
				host = host.substr(5, host.length() - 5);
		}

		// Path
		p_len = host.find("/");
		if (p_len != -1) {
			path = host.substr(p_len, host.length() - p_len);
			host = host.substr(0, p_len);
		}

		// Port
		p_len = host.find_last(":");
		if (p_len != -1 && p_len == host.find(":")) {
			port = host.substr(p_len, host.length() - p_len).to_int();
			host = host.substr(0, p_len);
		}

		connect_to_host(host, path, port, ssl, p_protocols);
	};
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
