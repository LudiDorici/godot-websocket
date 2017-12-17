#include "websocket_peer.h"

GDCINULL(WebSocketPeer);

WebSocketPeer::WebSocketPeer() {
}

WebSocketPeer::~WebSocketPeer() {
}

void WebSocketPeer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_write_mode"), &WebSocketPeer::get_write_mode);
	ClassDB::bind_method(D_METHOD("set_write_mode", "mode"), &WebSocketPeer::set_write_mode);
	ClassDB::bind_method(D_METHOD("is_connected_to_host"), &WebSocketPeer::is_connected_to_host);
	ClassDB::bind_method(D_METHOD("was_string_packet"), &WebSocketPeer::was_string_packet);
	ClassDB::bind_method(D_METHOD("close"), &WebSocketPeer::close);

	BIND_ENUM_CONSTANT(WRITE_MODE_TEXT);
	BIND_ENUM_CONSTANT(WRITE_MODE_BINARY);
}
