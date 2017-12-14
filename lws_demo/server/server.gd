extends Node

onready var _log_dest = get_parent().get_node("Panel/VBoxContainer/RichTextLabel")

var _server = WebSocketServer.new()
var _clients = {}
var _raw = true
var _write_mode = WebSocketPeer.WRITE_MODE_BINARY

func _init():
	_server.connect("client_connected", self, "_client_connected")
	_server.connect("client_disconnected", self, "_client_disconnected")
	_server.connect("data_received", self, "_client_receive")

func _ready():
	var mode = get_node("../Panel/VBoxContainer/HBoxContainer2/OptionButton")
	mode.clear()
	mode.add_item("BINARY")
	mode.set_item_metadata(0, WebSocketPeer.WRITE_MODE_BINARY)
	mode.add_item("TEXT")
	mode.set_item_metadata(1, WebSocketPeer.WRITE_MODE_TEXT)

func _exit_tree():
	_clients.clear()
	_server.stop()

func _process(delta):
	if _server.is_listening():
		_server.poll()

func _client_connected(id, protocol):
	_clients[id] = Utils.Peer.new(_server.get_peer(id))
	_clients[id].set_write_mode(_write_mode)
	Utils._log(_log_dest, "Client %s connected with protocol %s" % [id, protocol])

func _client_disconnected(id):
	Utils._log(_log_dest, "Client %s disconnected" % id)
	if _clients.has(id):
		_clients[id].free()
		_clients.erase(id)

func _client_receive(id):
	var data = _clients[id].recv()
	Utils._log(_log_dest, "Data from %s BINARY: %s: %s" % [id, _clients[id].is_binary_frame(), str(data)])

func _on_ListenBtn_toggled( pressed ):
	if pressed:
		var port = int(get_node("../Panel/VBoxContainer/HBoxContainer/Port").get_value())
		var supported_protocols = PoolStringArray(["my-protocol", "binary"])
		if _server.listen(port, supported_protocols) == OK:
			Utils._log(_log_dest, "Listing on port %s" % port)
			Utils._log(_log_dest, "Supported protocols: %s" % _server.get_protocols().join(", "))
		else:
			Utils._log(_log_dest, "Error listening on port %s" % port)
	else:
		_server.stop()
		Utils._log(_log_dest, "Server stopped")

func _on_Button_pressed():
	var data = get_node("../Panel/VBoxContainer/HBoxContainer2/LineEdit").text
	for id in _clients:
		_clients[id].set_write_mode(_write_mode)
		_clients[id].send(data)
	get_node("../Panel/VBoxContainer/HBoxContainer2/LineEdit").text = ""

func _on_OptionButton_item_selected( ID ):
	_write_mode = get_node("../Panel/VBoxContainer/HBoxContainer2/OptionButton").get_selected_metadata()
	for c in _clients:
		_clients[c].set_write_mode(_write_mode)
