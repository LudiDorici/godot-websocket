#ifndef EMWSSERVER_H
#define EMWSSERVER_H

#ifdef JAVASCRIPT_ENABLED

#include "core/reference.h"
#include "websocket_server.h"
#include "emws_peer.h"

class EMWSServer : public WebSocketServer {

	GDCIIMPL(EMWSServer, WebSocketServer);

public:
	Error listen(int p_port, PoolVector<String> p_protocols = PoolVector<String>());
	void stop();
	bool is_listening() const;
	bool has_peer(int p_id) const;
	Ref<WebSocketPeer> get_peer(int p_id) const;
	virtual void poll();
	virtual PoolVector<String> get_protocols() const;

	EMWSServer();
	~EMWSServer();
};

#endif

#endif // LWSSERVER_H
