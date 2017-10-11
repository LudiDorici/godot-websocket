#include "websocket_peer.h"
#include "core/io/ip.h"

void WebSocketPeer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_write_mode"), &WebSocketPeer::get_write_mode);
	ClassDB::bind_method(D_METHOD("set_write_mode", "mode"), &WebSocketPeer::set_write_mode);
	ClassDB::bind_method(D_METHOD("is_connected_to_host"), &WebSocketPeer::is_connected_to_host);
	ClassDB::bind_method(D_METHOD("is_binary_frame"), &WebSocketPeer::is_binary_frame);
	ClassDB::bind_method(D_METHOD("is_first_fragment"), &WebSocketPeer::is_first_fragment);
	ClassDB::bind_method(D_METHOD("is_final_fragment"), &WebSocketPeer::is_final_fragment);
	ClassDB::bind_method(D_METHOD("close"), &WebSocketPeer::close);

	BIND_ENUM_CONSTANT(WRITE_MODE_TEXT);
	BIND_ENUM_CONSTANT(WRITE_MODE_BINARY);
}

void WebSocketPeer::set_wsi(struct lws *p_wsi) {
	wsi = p_wsi;
};

void WebSocketPeer::set_write_mode(WriteMode p_mode) {
	write_mode = p_mode;
}

WebSocketPeer::WriteMode WebSocketPeer::get_write_mode() const {
	return write_mode;
}

Error WebSocketPeer::read_wsi(void *in, size_t len) {

	ERR_FAIL_COND_V(!is_connected_to_host(), FAILED);

	PeerData *peer_data = (PeerData *)(lws_wsi_user(wsi));

	if (peer_data->rbr.space_left() < len) {
		ERR_EXPLAIN("Buffer full! Dropping data");
		ERR_FAIL_V(FAILED);
	}

	peer_data->rbr.write((uint8_t *)in, len);

	return OK;
}

Error WebSocketPeer::write_wsi() {

	ERR_FAIL_COND_V(!is_connected_to_host(), FAILED);

	PeerData *peer_data = (PeerData *)(lws_wsi_user(wsi));
	PoolVector<uint8_t> tmp;
	int left = peer_data->rbw.data_left();

	if (left == 0)
		return OK;

	tmp.resize(LWS_PRE + left);
	peer_data->rbw.read(&(tmp.write()[LWS_PRE]), left);
	lws_write(wsi, &(tmp.write()[LWS_PRE]), left, (enum lws_write_protocol)write_mode);
	tmp.resize(0);

	return OK;
}

Error WebSocketPeer::write(const uint8_t *p_data, int p_bytes, int &r_sent, bool p_block) {

	ERR_FAIL_COND_V(!is_connected_to_host(), FAILED);

	PeerData *peer_data = (PeerData *)lws_wsi_user(wsi);
	r_sent = peer_data->rbw.write(p_data, MIN(p_bytes, peer_data->rbw.space_left()));

	lws_callback_on_writable(wsi); // notify that we want to write
	return OK;
};

Error WebSocketPeer::read(uint8_t *p_buffer, int p_bytes, int &r_received, bool p_block) {

	ERR_FAIL_COND_V(!is_connected_to_host(), FAILED);

	PeerData *peer_data = (PeerData *)lws_wsi_user(wsi);
	r_received = MIN(p_bytes, peer_data->rbr.data_left());
	peer_data->rbr.read(p_buffer, r_received);

	return OK;
};

bool WebSocketPeer::is_binary_frame() const {

	ERR_FAIL_COND_V(!is_connected_to_host(), false);

	return lws_frame_is_binary(wsi);
};

bool WebSocketPeer::is_final_fragment() const {

	ERR_FAIL_COND_V(!is_connected_to_host(), false);

	return lws_is_final_fragment(wsi);
};

bool WebSocketPeer::is_first_fragment() const {

	ERR_FAIL_COND_V(!is_connected_to_host(), false);

	return lws_is_first_fragment(wsi);
};

bool WebSocketPeer::is_connected_to_host() const {

	return wsi != NULL;
};

void WebSocketPeer::close() {
	if (wsi != NULL) {
		struct lws *tmp = wsi;
		PeerData *data = ((PeerData *)lws_wsi_user(wsi));
		data->force_close = true;
		wsi = NULL;
		lws_callback_on_writable(tmp); // notify that we want to disconnect
	}
};

Error WebSocketPeer::put_data(const uint8_t *p_data, int p_bytes) {

	int total;
	return write(p_data, p_bytes, total, true);
};

Error WebSocketPeer::put_partial_data(const uint8_t *p_data, int p_bytes, int &r_sent) {

	return write(p_data, p_bytes, r_sent, false);
};

Error WebSocketPeer::get_data(uint8_t *p_buffer, int p_bytes) {

	int total;
	return read(p_buffer, p_bytes, total, true);
};

Error WebSocketPeer::get_partial_data(uint8_t *p_buffer, int p_bytes, int &r_received) {

	return read(p_buffer, p_bytes, r_received, false);
};

int WebSocketPeer::get_available_bytes() const {

	ERR_FAIL_COND_V(!is_connected_to_host(), 0);

	return ((PeerData *)(lws_wsi_user(wsi)))->rbr.data_left();
}

IP_Address WebSocketPeer::get_connected_host() const {

	return IP_Address();
};

uint16_t WebSocketPeer::get_connected_port() const {

	return 1025;
};

WebSocketPeer::WebSocketPeer() {
	wsi = NULL;
	write_mode = WRITE_MODE_BINARY;
};

WebSocketPeer::~WebSocketPeer() {

	close();
};
