#ifndef ESWSSERVER_H
#define ESWSSERVER_H

#ifdef JAVASCRIPT_ENABLED

#include "core/reference.h"
#include "websocket_server.h"
#include "esws_peer.h"

class ESWSServer : public WebSocketServer {

	GDCIIMPL(ESWSServer, WebSocketServer);

public:
	Error listen(int p_port, PoolVector<String> p_protocols = PoolVector<String>());
	void stop();
	bool is_listening() const;
	bool has_peer(int p_id) const;
	Ref<WebSocketPeer> get_peer(int p_id) const;
	virtual void poll();
	virtual PoolVector<String> get_protocols() const;

	ESWSServer();
	~ESWSServer();
};

#endif

#endif // LWSSERVER_H
