extends Node

const PACKET = true

class Peer extends Object:

	var _packet_peer = null

	func _init(peer=null):
		_packet_peer = peer

	func send(data):
		if PACKET: # Sends as packets
			_packet_peer.put_packet(var2bytes(data))
		else: # Sends as var
			_packet_peer.put_packet(data.to_utf8())

	func recv():
		if _packet_peer.get_available_packet_count() == 0:
			return null
		if PACKET:
				return bytes2var(_packet_peer.get_packet())
		else:
				return _packet_peer.get_packet().get_string_from_utf8()

	func get_available_packet_count():
		return _packet_peer.get_available_packet_count()

	func close():
		_packet_peer.close()

func _log(node, msg):
	print(msg)
	node.add_text(str(msg) + "\n")