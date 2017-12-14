#ifndef ESWSPEER_H
#define ESWSPEER_H

#ifdef JAVASCRIPT_ENABLED

#include "core/error_list.h"
#include "core/io/packet_peer.h"
#include "core/ring_buffer.h"
#include "emscripten.h"
#include "websocket_peer.h"

class ESWSPeer : public WebSocketPeer {

	GDCIIMPL(ESWSPeer, WebSocketPeer);

private:

	enum {
		PACKET_BUFFER_SIZE = 65536 - 4 // 4 bytes for the size
	};

	int peer_sock;
	WriteMode write_mode;

	mutable uint8_t packet_buffer[PACKET_BUFFER_SIZE];
	mutable RingBuffer<uint8_t> in_buffer;
	mutable int queue_count;

public:

	void read_msg(uint8_t *p_data, uint32_t p_size);
	void set_sock(int sock);
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

	ESWSPeer();
	~ESWSPeer();
};

#endif // JAVASCRIPT_ENABLED

#endif // LSWPEER_H
