#ifdef JAVASCRIPT_ENABLED

#include "emws_server.h"
#include "core/os/os.h"

Error EMWSServer::listen(int p_port, PoolVector<String> p_protocols, bool gd_mp_api) {

	return FAILED;
}

bool EMWSServer::is_listening() const {
	return false;
}

void EMWSServer::stop() {
}

bool EMWSServer::has_peer(int p_id) const {
	return false;
}

Ref<WebSocketPeer> EMWSServer::get_peer(int p_id) const {
	return NULL;
}

PoolVector<String> EMWSServer::get_protocols() const {
	PoolVector<String> out;

	return out;
}

EMWSServer::EMWSServer() {
}

EMWSServer::~EMWSServer() {
}

#endif // JAVASCRIPT_ENABLED
