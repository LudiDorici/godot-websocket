extends Node

onready var _log_dest = get_parent().get_node("Panel/VBoxContainer/RichTextLabel")

var _client = WebSocketClient.new()
var _peer = null
var wsc = WebSocketPeer.new()
var _connected = false

func _init():
	_client.connect("connection_established", self, "_client_connected")
	_client.connect("connection_error", self, "_client_disconnected")
	_client.connect("connection_closed", self, "_client_disconnected")
	_client.connect("data_received", self, "_client_received")

func _exit_tree():
	_client.disconnect_from_host()

func _process(delta):
	if _client.is_connected_to_host() or _client.is_connecting_to_host():
		_client.poll()

func _client_connected(protocol):
	Utils._log(_log_dest, "Client just connected with protocol: %s" % protocol)
	_peer = Utils.Peer.new(_client.get_peer())

func _client_disconnected():
	Utils._log(_log_dest, "Client just disconnected")
	if _peer != null:
		_peer.free()
		_peer = null

func _client_received():
	Utils._log(_log_dest, "Client just received %s bytes" % _peer._stream.get_available_bytes())
	var data = _peer.recv()
	Utils._log(_log_dest, "Received data: %s" % data)

func _on_ConnectBtn_toggled( pressed ):
	if pressed:
		var txt = get_node("../Panel/VBoxContainer/Connect/LineEdit").get_text()
		var port = get_node("../Panel/VBoxContainer/Connect/SpinBox").get_value()
		if txt != "":
			_client.connect_to_host(txt, int(port), PoolStringArray(["my-protocol-2", "my-protocol", "binary"]))
			Utils._log(_log_dest, "Connecting to host: %s : %s" % [txt, port])
	else:
		if _peer != null:
			_peer._stream.close()
		_client.disconnect_from_host()

func _on_SendBtn_pressed():
	if _peer == null:
		return
	var txt = get_node("../Panel/VBoxContainer/Send/LineEdit").get_text()
	if txt != "":
		_peer.send(txt)
		get_node("../Panel/VBoxContainer/Send/LineEdit").set_text("")
		Utils._log(_log_dest, "Sending data: %s" % txt)