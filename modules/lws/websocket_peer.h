#ifndef WEBSOCKETPEER_H
#define WEBSOCKETPEER_H

#include "core/error_list.h"
#include "core/io/packet_peer.h"
#include "core/ring_buffer.h"
#include "lws_config.h"
#include "libwebsockets.h"

class WebSocketPeer : public PacketPeer {

	GDCLASS(WebSocketPeer, PacketPeer);

public:
	enum WriteMode {
		WRITE_MODE_TEXT = LWS_WRITE_TEXT,
		WRITE_MODE_BINARY = LWS_WRITE_BINARY,
	};

protected:
	static void _bind_methods();

private:

	enum {
		PACKET_BUFFER_SIZE = 65536 - 4 // 4 bytes for the size
	};

	mutable uint8_t packet_buffer[PACKET_BUFFER_SIZE];
	struct lws *wsi;
	WriteMode write_mode;

public:
	struct PeerData {
		uint32_t peer_id;
		bool force_close;
		RingBuffer<uint8_t> rbw;
		RingBuffer<uint8_t> rbr;
		mutable uint8_t input_buffer[PACKET_BUFFER_SIZE];
		uint32_t in_size;
		int in_count;
		int out_count;
	};

	virtual int get_available_packet_count() const;
	virtual Error get_packet(const uint8_t **r_buffer, int &r_buffer_size) const;
	virtual Error put_packet(const uint8_t *p_buffer, int p_buffer_size);

	virtual int get_max_packet_size() const { return PACKET_BUFFER_SIZE; };

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
