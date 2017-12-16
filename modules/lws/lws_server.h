#ifndef LWSSERVER_H
#define LWSSERVER_H

#ifndef JAVASCRIPT_ENABLED

#include "core/reference.h"
#include "websocket_server.h"
#include "lws_helper.h"
#include "lws_peer.h"

class LWSServer : public WebSocketServer {

	GDCIIMPL(LWSServer, WebSocketServer);

	LWS_HELPER(LWSServer);

private:
	Map<int, Ref<LWSPeer> > peer_map;

	uint32_t _gen_unique_id() const;

public:
	Error listen(int p_port, PoolVector<String> p_protocols = PoolVector<String>(), bool gd_mp_api=false);
	void stop();
	bool is_listening() const;
	bool has_peer(int p_id) const;
	Ref<WebSocketPeer> get_peer(int p_id) const;
	virtual void poll() { _lws_poll(); }

	LWSServer();
	~LWSServer();
};

#endif // JAVASCRIPT_ENABLED

#endif // LWSSERVER_H
