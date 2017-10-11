extends Node

const RAW = true

class Peer extends Object:

	var _stream = null
	var _packet_peer = null

	func _init(peer=null):
		_stream = peer
		if _stream != null:
			_packet_peer = PacketPeerStream.new()
			_packet_peer.set_stream_peer(peer)

	func send(data):
		if RAW: # Sends as packets
			_packet_peer.put_packet(var2bytes(data))
		else: # Sends as var
			_stream.put_var(data)

	func recv():
		if RAW:
			if _packet_peer.get_available_packet_count() > 0:
				return bytes2var(_packet_peer.get_packet())
		else:
			if _stream.get_available_bytes() > 0:
				return _stream.get_var()
		return null

func _log(node, msg):
	print(msg)
	node.add_text(str(msg) + "\n")