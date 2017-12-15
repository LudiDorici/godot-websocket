extends Node

onready var _log_dest = get_parent().get_node("Panel/VBoxContainer/RichTextLabel")

var _client = WebSocketClient.new()
var _peer = null
var wsc = WebSocketPeer.new()
var _connected = false
var _write_mode = WebSocketPeer.WRITE_MODE_BINARY

func _init():
	_client.connect("connection_established", self, "_client_connected")
	_client.connect("connection_error", self, "_client_disconnected")
	_client.connect("connection_closed", self, "_client_disconnected")
	_client.connect("data_received", self, "_client_received")

func _ready():
	var mode = get_node("../Panel/VBoxContainer/Send/Mode")
	mode.clear()
	mode.add_item("BINARY")
	mode.set_item_metadata(0, WebSocketPeer.WRITE_MODE_BINARY)
	mode.add_item("TEXT")
	mode.set_item_metadata(1, WebSocketPeer.WRITE_MODE_TEXT)

func _exit_tree():
	_client.disconnect_from_host()

func _process(delta):
	if _client.is_connected_to_host() or _client.is_connecting_to_host():
		_client.poll()

func _client_connected(protocol):
	Utils._log(_log_dest, "Client just connected with protocol: %s" % protocol)
	_peer = Utils.Peer.new(_client.get_peer())
	_peer.set_write_mode(_write_mode)

func _client_disconnected():
	Utils._log(_log_dest, "Client just disconnected")
	if _peer != null:
		_peer.free()
		_peer = null

func _client_received():
	Utils._log(_log_dest, "Client just received %s packets" % _peer.get_available_packet_count())
	var data = _peer.recv()
	Utils._log(_log_dest, "Received data: %s" % data)

func _on_ConnectBtn_toggled( pressed ):
	if pressed:
		var txt = get_node("../Panel/VBoxContainer/Connect/LineEdit").get_text()
		if txt != "":
			_client.connect_to_url(txt, PoolStringArray(["my-protocol-2", "my-protocol", "binary"]))
			Utils._log(_log_dest, "Connecting to host: %s" % [txt])
	else:
		if _peer != null:
			_peer.close()
		_client.disconnect_from_host()

func _on_SendBtn_pressed():
	if _peer == null:
		return
	var txt = get_node("../Panel/VBoxContainer/Send/LineEdit").get_text()
	if txt != "":
		_peer.send(txt)
		get_node("../Panel/VBoxContainer/Send/LineEdit").set_text("")
		Utils._log(_log_dest, "Sending data: %s" % txt)

func _on_Mode_item_selected( ID ):
	_write_mode = get_node("../Panel/VBoxContainer/Send/Mode").get_selected_metadata()
	if _peer != null:
		_peer.set_write_mode(_write_mode)
