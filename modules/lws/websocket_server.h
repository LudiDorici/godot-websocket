#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "core/reference.h"
#include "websocket_helper.h"
#include "websocket_peer.h"

class WebSocketServer : public WebSocketHelper {

	GDCLASS(WebSocketServer, WebSocketHelper);

private:
	Map<int, Ref<WebSocketPeer> > peer_map;

	uint32_t _gen_unique_id() const;

protected:
	static void _bind_methods();

	virtual int _handle_cb(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

public:
	Error listen(int p_port, PoolVector<String> p_protocols = PoolVector<String>());
	void stop();
	bool is_listening() const;
	bool has_peer(int p_id) const;
	Ref<WebSocketPeer> get_peer(int p_id) const;

	WebSocketServer();
	~WebSocketServer();
};

#endif // WEBSOCKET_H
