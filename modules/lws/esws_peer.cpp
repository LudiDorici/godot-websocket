#ifdef JAVASCRIPT_ENABLED

#include "esws_peer.h"
#include "core/io/ip.h"

void ESWSPeer::set_write_mode(WriteMode p_mode) {
	write_mode = p_mode;
}

ESWSPeer::WriteMode ESWSPeer::get_write_mode() const {
	return write_mode;
}

Error ESWSPeer::put_packet(const uint8_t *p_buffer, int p_buffer_size) {

	return OK;
};

Error ESWSPeer::get_packet(const uint8_t **r_buffer, int &r_buffer_size) const {

	return OK;
};

int ESWSPeer::get_available_packet_count() const {

	if (!is_connected_to_host())
		return 0;

	return 0;
};

bool ESWSPeer::is_binary_frame() const {

	ERR_FAIL_COND_V(!is_connected_to_host(), false);

	return false;
};

bool ESWSPeer::is_connected_to_host() const {

	return false;
};

void ESWSPeer::close() {

};

IP_Address ESWSPeer::get_connected_host() const {

	return IP_Address();
};

uint16_t ESWSPeer::get_connected_port() const {

	return 1025;
};

ESWSPeer::ESWSPeer() {
	write_mode = WRITE_MODE_BINARY;
};

ESWSPeer::~ESWSPeer() {

	close();
};

#endif // JAVASCRIPT_ENABLED
