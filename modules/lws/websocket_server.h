#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "core/reference.h"
#include "websocket_multiplayer.h"
#include "websocket_peer.h"

class WebSocketServer : public WebSocketMultiplayerPeer {

	GDCLASS(WebSocketServer, WebSocketMultiplayerPeer);
	GDCICLASS(WebSocketServer);

protected:
	static void _bind_methods();

public:
	virtual void poll() = 0;
	virtual Error listen(int p_port, PoolVector<String> p_protocols = PoolVector<String>(), bool gd_mp_api = false) = 0;
	virtual void stop() = 0;
	virtual bool is_listening() const = 0;
	virtual bool has_peer(int p_id) const = 0;
	virtual Ref<WebSocketPeer> get_peer(int p_id) const = 0;
	virtual PoolVector<String> get_protocols() const = 0;
	virtual bool is_server() const;
	ConnectionStatus get_connection_status() const;

	void _on_peer_packet(int32_t p_peer_id);
	void _on_connect(int32_t p_peer_id, String p_protocol);
	void _on_disconnect(int32_t p_peer_id);

	WebSocketServer();
	~WebSocketServer();
};

#endif // WEBSOCKET_H
