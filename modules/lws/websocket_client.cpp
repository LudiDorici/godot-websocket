#include "websocket_client.h"

GDCINULL(WebSocketClient);

WebSocketClient::WebSocketClient() {

}

WebSocketClient::~WebSocketClient() {

}

Error WebSocketClient::connect_to_url(String p_url, PoolVector<String> p_protocols, bool p_mp_api) {
	_is_multiplayer = p_mp_api;

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
}

NetworkedMultiplayerPeer::ConnectionStatus WebSocketClient::get_connection_status() const {

	if (is_connected_to_host())
		return CONNECTION_CONNECTED;

	if (is_connecting_to_host())
		return CONNECTION_CONNECTING;

	return CONNECTION_DISCONNECTED;
}

void WebSocketClient::_on_peer_packet() {

	if (_is_multiplayer) {
		_process_multiplayer(get_peer(1));
	} else {
		emit_signal("data_received");
	}
}

void WebSocketClient::_on_connect(String p_protocol) {

	if (_is_multiplayer) {
		// need to wait for ID confirmation...
	} else {
		emit_signal("connection_established", p_protocol);
	}
}

void WebSocketClient::_on_disconnect() {

	if (_is_multiplayer) {
		emit_signal("connection_failed");
	} else {
		emit_signal("connection_closed");
	}
}

void WebSocketClient::_on_error() {

	if (_is_multiplayer) {
		emit_signal("connection_failed");
	} else {
		emit_signal("connection_error");
	}
}

void WebSocketClient::_bind_methods() {
	ClassDB::bind_method(D_METHOD("connect_to_url", "url", "protocols"), &WebSocketClient::connect_to_url, DEFVAL(PoolVector<String>()));
	ClassDB::bind_method(D_METHOD("disconnect_from_host"), &WebSocketClient::disconnect_from_host);
	ClassDB::bind_method(D_METHOD("is_connected_to_host"), &WebSocketClient::is_connected_to_host);
	ClassDB::bind_method(D_METHOD("is_connecting_to_host"), &WebSocketClient::is_connecting_to_host);

	ADD_SIGNAL(MethodInfo("data_received"));
	ADD_SIGNAL(MethodInfo("connection_established", PropertyInfo(Variant::STRING, "protocol")));
	ADD_SIGNAL(MethodInfo("connection_closed"));
	ADD_SIGNAL(MethodInfo("connection_error"));

}
