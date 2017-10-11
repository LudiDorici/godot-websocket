#ifndef WEBSOCKETPEER_H
#define WEBSOCKETPEER_H

#include "core/error_list.h"
#include "core/io/stream_peer.h"
#include "core/ring_buffer.h"
#include "lws_config.h"
#include "libwebsockets.h"

class WebSocketPeer : public StreamPeer {

	GDCLASS(WebSocketPeer, StreamPeer);

public:
	enum WriteMode {
		WRITE_MODE_TEXT = LWS_WRITE_TEXT,
		WRITE_MODE_BINARY = LWS_WRITE_BINARY,
	};

	struct PeerData {
		uint32_t peer_id;
		bool force_close;
		RingBuffer<uint8_t> rbw;
		RingBuffer<uint8_t> rbr;
	};

private:
	struct lws *wsi;
	WriteMode write_mode;

protected:
	static void _bind_methods();

public:
	virtual Error put_data(const uint8_t *p_data, int p_bytes);
	virtual Error put_partial_data(const uint8_t *p_data, int p_bytes, int &r_sent);
	virtual Error get_data(uint8_t *p_buffer, int p_bytes);
	virtual Error get_partial_data(uint8_t *p_buffer, int p_bytes, int &r_received);
	virtual int get_available_bytes() const;

	WriteMode get_write_mode() const;
	void set_write_mode(WriteMode p_mode);
	void set_wsi(struct lws *wsi);

	Error read_wsi(void *in, size_t len);
	Error write_wsi();

	Error read(uint8_t *p_buffer, int p_bytes, int &r_received, bool p_block);
	Error write(const uint8_t *p_data, int p_bytes, int &r_sent, bool p_block);

	Error connect_to_host(String p_host, uint16_t p_port);
	void close();

	bool is_connected_to_host() const;
	bool is_binary_frame() const;
	bool is_final_fragment() const;
	bool is_first_fragment() const;
	IP_Address get_connected_host() const;
	uint16_t get_connected_port() const;

	WebSocketPeer();
	~WebSocketPeer();
};

VARIANT_ENUM_CAST(WebSocketPeer::WriteMode);
#endif // WEBSOCKETPEER_H
