#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "core/reference.h"
#include "websocket_peer.h"

class WebSocketServer : public Reference {

	GDCLASS(WebSocketServer, Reference);
	GDCICLASS(WebSocketServer);

protected:
	static void _bind_methods() {

		ClassDB::bind_method(D_METHOD("poll"), &WebSocketServer::poll);
		ClassDB::bind_method(D_METHOD("is_listening"), &WebSocketServer::is_listening);
		ClassDB::bind_method(D_METHOD("listen", "port", "protocols"), &WebSocketServer::listen, DEFVAL(PoolVector<String>()));
		ClassDB::bind_method(D_METHOD("stop"), &WebSocketServer::stop);
		ClassDB::bind_method(D_METHOD("has_peer", "id"), &WebSocketServer::has_peer);
		ClassDB::bind_method(D_METHOD("get_peer", "id"), &WebSocketServer::get_peer);
		ClassDB::bind_method(D_METHOD("get_protocols"), &WebSocketServer::get_protocols);

		ADD_SIGNAL(MethodInfo("client_disconnected", PropertyInfo(Variant::INT, "id")));
		ADD_SIGNAL(MethodInfo("client_connected", PropertyInfo(Variant::INT, "id"), PropertyInfo(Variant::STRING, "protocol")));
		ADD_SIGNAL(MethodInfo("data_received", PropertyInfo(Variant::INT, "id")));

	}

public:

	virtual void poll() = 0;
	virtual Error listen(int p_port, PoolVector<String> p_protocols = PoolVector<String>()) = 0;
	virtual void stop() = 0;
	virtual bool is_listening() const = 0;
	virtual bool has_peer(int p_id) const = 0;
	virtual Ref<WebSocketPeer> get_peer(int p_id) const = 0;
	virtual PoolVector<String> get_protocols() const = 0;

	WebSocketServer() {};
	~WebSocketServer() {};
};

#endif // WEBSOCKET_H
