#ifndef LWSPEER_H
#define LWSPEER_H

#include "core/error_list.h"
#include "core/io/packet_peer.h"
#include "core/ring_buffer.h"
#include "websocket_peer.h"
#include "lws_config.h"
#include "libwebsockets.h"

class LWSPeer : public WebSocketPeer {

	GDCIIMPL(LWSPeer, WebSocketPeer);

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

	virtual void close();
	virtual bool is_connected_to_host() const;
	virtual IP_Address get_connected_host() const;
	virtual uint16_t get_connected_port() const;

	virtual WriteMode get_write_mode() const;
	virtual void set_write_mode(WriteMode p_mode);
	virtual bool is_binary_frame() const;

	void set_wsi(struct lws *wsi);
	Error read_wsi(void *in, size_t len);
	Error write_wsi();

	LWSPeer();
	~LWSPeer();
};

#endif // LSWPEER_H
