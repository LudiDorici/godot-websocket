#ifdef JAVASCRIPT_ENABLED

#include "esws_server.h"
#include "core/os/os.h"

Error ESWSServer::listen(int p_port, PoolVector<String> p_protocols) {

	return FAILED;
}

bool ESWSServer::is_listening() const {
	return false;
}

void ESWSServer::stop() {

}

bool ESWSServer::has_peer(int p_id) const {
	return false;
}

Ref<WebSocketPeer> ESWSServer::get_peer(int p_id) const {
	return NULL;
}

PoolVector<String> ESWSServer::get_protocols() const {
	PoolVector<String> out;

	return out;
}

ESWSServer::ESWSServer() {

}

ESWSServer::~ESWSServer() {

}

#endif // JAVASCRIPT_ENABLED
