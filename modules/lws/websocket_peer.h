#ifndef WEBSOCKETPEER_H
#define WEBSOCKETPEER_H

#include "core/error_list.h"
#include "core/io/packet_peer.h"
#include "core/ring_buffer.h"
#include "lws_config.h"
#include "libwebsockets.h"
#include "websocket_macros.h"

class WebSocketPeer : public PacketPeer {

	GDCLASS(WebSocketPeer, PacketPeer);
	GDCICLASS(WebSocketPeer);

public:
	enum WriteMode {
		WRITE_MODE_TEXT,
		WRITE_MODE_BINARY,
	};

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("get_write_mode"), &WebSocketPeer::get_write_mode);
		ClassDB::bind_method(D_METHOD("set_write_mode", "mode"), &WebSocketPeer::set_write_mode);
		ClassDB::bind_method(D_METHOD("is_connected_to_host"), &WebSocketPeer::is_connected_to_host);
		ClassDB::bind_method(D_METHOD("is_binary_frame"), &WebSocketPeer::is_binary_frame);
		ClassDB::bind_method(D_METHOD("close"), &WebSocketPeer::close);

		BIND_ENUM_CONSTANT(WRITE_MODE_TEXT);
		BIND_ENUM_CONSTANT(WRITE_MODE_BINARY);
	}

public:

	virtual int get_available_packet_count() const = 0;
	virtual Error get_packet(const uint8_t **r_buffer, int &r_buffer_size) const = 0;
	virtual Error put_packet(const uint8_t *p_buffer, int p_buffer_size) = 0;
	virtual int get_max_packet_size() const = 0;

	virtual WriteMode get_write_mode() const = 0;
	virtual void set_write_mode(WriteMode p_mode) = 0;

	virtual void close() = 0;

	virtual bool is_connected_to_host() const = 0;
	virtual IP_Address get_connected_host() const = 0;
	virtual uint16_t get_connected_port() const = 0;
	virtual bool is_binary_frame() const = 0;

	WebSocketPeer() {};
	~WebSocketPeer() {};
};

VARIANT_ENUM_CAST(WebSocketPeer::WriteMode);
#endif // WEBSOCKETPEER_H
