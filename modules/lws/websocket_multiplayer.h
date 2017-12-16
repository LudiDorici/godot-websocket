#ifndef WEBSOCKET_MULTIPLAYER_PEER_H
#define WEBSOCKET_MULTIPLAYER_PEER_H

#include "core/error_list.h"
#include "core/io/networked_multiplayer_peer.h"
#include "core/list.h"
#include "websocket_peer.h"

class WebSocketMultiplayerPeer : public NetworkedMultiplayerPeer {

	GDCLASS(WebSocketMultiplayerPeer, NetworkedMultiplayerPeer);

protected:
	struct Packet {
		int source;
		int destination;
		uint8_t *data;
		uint32_t size;
	};

	List<Packet> _incoming_packets;
	Packet _current_packet;

	bool _is_multiplayer;
	int _target_peer;
	int _peer_id;
	int _refusing;

	static void _bind_methods();

public:
	/* NetworkedMultiplayerPeer */
	void set_transfer_mode(TransferMode p_mode);
	void set_target_peer(int p_peer_id);
	int get_packet_peer() const;
	int get_unique_id() const;
	bool is_server() const;
	void set_refuse_new_connections(bool p_enable);
	bool is_refusing_new_connections() const;
	virtual ConnectionStatus get_connection_status() const = 0;

	/* PacketPeer */
	virtual int get_available_packet_count() const;
	virtual int get_max_packet_size() const;
	virtual Error get_packet(const uint8_t **r_buffer, int &r_buffer_size);
	virtual Error put_packet(const uint8_t *p_buffer, int p_buffer_size);

	/* WebSocketPeer */
	virtual Ref<WebSocketPeer> get_peer(int p_peer_id) const = 0;

	void _process_multiplayer(Ref<WebSocketPeer> p_peer);
	void _on_peer_packet(int p_peer_id);
	void _on_connect(String p_protocol);
	void _on_disconnect();
	void _on_error();

	WebSocketMultiplayerPeer();
	~WebSocketMultiplayerPeer();
};

#endif // WEBSOCKET_MULTIPLAYER_PEER_H
