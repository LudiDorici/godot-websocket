#ifdef JAVASCRIPT_ENABLED

#include "esws_client.h"
#include "core/io/ip.h"

Error ESWSClient::connect_to_host(String p_host, uint16_t p_port, PoolVector<String> p_protocols) {

	return OK;
};

void ESWSClient::poll() {

}

Ref<WebSocketPeer> ESWSClient::get_peer() const {

	return NULL;
}

bool ESWSClient::is_connected_to_host() const {

	return false;
};

bool ESWSClient::is_connecting_to_host() const {

	return false;
};

void ESWSClient::disconnect_from_host() {

};

IP_Address ESWSClient::get_connected_host() const {

	return IP_Address();
};

uint16_t ESWSClient::get_connected_port() const {

	return 1025;
};

ESWSClient::ESWSClient() {

};

ESWSClient::~ESWSClient() {

	disconnect_from_host();
};

#endif // JAVASCRIPT_ENABLED
