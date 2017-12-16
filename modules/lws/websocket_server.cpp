#include "websocket_server.h"

GDCINULL(WebSocketServer);

WebSocketServer::WebSocketServer() {

}

WebSocketServer::~WebSocketServer() {

}

void WebSocketServer::_bind_methods() {

	ClassDB::bind_method(D_METHOD("is_listening"), &WebSocketServer::is_listening);
	ClassDB::bind_method(D_METHOD("listen", "port", "protocols"), &WebSocketServer::listen, DEFVAL(PoolVector<String>()));
	ClassDB::bind_method(D_METHOD("stop"), &WebSocketServer::stop);
	ClassDB::bind_method(D_METHOD("has_peer", "id"), &WebSocketServer::has_peer);
	ClassDB::bind_method(D_METHOD("get_protocols"), &WebSocketServer::get_protocols);

	ADD_SIGNAL(MethodInfo("client_disconnected", PropertyInfo(Variant::INT, "id")));
	ADD_SIGNAL(MethodInfo("client_connected", PropertyInfo(Variant::INT, "id"), PropertyInfo(Variant::STRING, "protocol")));
	ADD_SIGNAL(MethodInfo("data_received", PropertyInfo(Variant::INT, "id")));

}

NetworkedMultiplayerPeer::ConnectionStatus WebSocketServer::get_connection_status() const {
	if (is_listening())
		return CONNECTION_CONNECTED;

	return CONNECTION_DISCONNECTED;
};

void WebSocketServer::_on_peer_packet(uint32_t p_peer_id) {

	if (_is_multiplayer) {
		_process_multiplayer(get_peer(p_peer_id));
	} else {
		emit_signal("data_received", p_peer_id);
	}
}

void WebSocketServer::_on_connect(uint32_t p_peer_id, String p_protocol) {

	if (_is_multiplayer) {
		// Send add to clients
		_send_add(p_peer_id);
		emit_signal("peer_connected", p_peer_id);
	} else {
		emit_signal("client_connected", p_peer_id, p_protocol);
	}
}

void WebSocketServer::_on_disconnect(uint32_t p_peer_id) {

	if (_is_multiplayer) {
		// Send delete to clients
		_send_del(p_peer_id);
		emit_signal("peer_disconnected", p_peer_id);
	} else {
		emit_signal("client_disconnected", p_peer_id);
	}
}
