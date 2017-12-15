extends Node

class Peer extends Object:

	var _packet_peer = null
	var _write_mode = WebSocketPeer.WRITE_MODE_BINARY

	func _init(peer=null):
		_packet_peer = peer

	func send(data):
		if _write_mode == WebSocketPeer.WRITE_MODE_BINARY:
			# Sends as Godot encoded variables
			_packet_peer.put_var(data)
		else:
			# Sends as UTF-8 string
			_packet_peer.put_packet(data.to_utf8())

	func recv():
		if _packet_peer.get_available_packet_count() == 0:
			return null
		var packet = _packet_peer.get_packet()
		if _packet_peer.was_string_packet():
			# Interpret as a string
			return packet.get_string_from_utf8()
		else:
			# Interpret as a Godot encoded var (it could actually be any binary data)
			return bytes2var(packet)

	func get_available_packet_count():
		return _packet_peer.get_available_packet_count()

	func set_write_mode(mode):
		_write_mode = mode
		_packet_peer.set_write_mode(mode)

	func was_string_packet():
		return _packet_peer.was_string_packet()

	func close():
		_packet_peer.close()

func _log(node, msg):
	print(msg)
	node.add_text(str(msg) + "\n")