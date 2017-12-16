#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include "core/error_list.h"
#include "websocket_multiplayer.h"
#include "websocket_peer.h"

class WebSocketClient : public WebSocketMultiplayerPeer {

	GDCLASS(WebSocketClient, WebSocketMultiplayerPeer);
	GDCICLASS(WebSocketClient);

protected:
	Ref<WebSocketPeer> _peer;

	static void _bind_methods();

public:
	Error connect_to_url(String p_url, PoolVector<String> p_protocols = PoolVector<String>(), bool gd_mp_api = false);

	virtual void poll() = 0;
	virtual Error connect_to_host(String p_host, String p_path, uint16_t p_port, bool p_ssl, PoolVector<String> p_protocol = PoolVector<String>()) = 0;
	virtual void disconnect_from_host() = 0;
	virtual IP_Address get_connected_host() const = 0;
	virtual uint16_t get_connected_port() const = 0;

	virtual bool is_server() const;
	virtual ConnectionStatus get_connection_status() const = 0;

	void _on_peer_packet();
	void _on_connect(String p_protocol);
	void _on_disconnect();
	void _on_error();

	WebSocketClient();
	~WebSocketClient();
};

#endif // WEBSOCKET_CLIENT_H
