extends Node

onready var _log_dest = get_parent().get_node("Panel/VBoxContainer/RichTextLabel")

var _server = WebSocketServer.new()
var _clients = {}
var _raw = true

func _init():
	_server.connect("client_connected", self, "_client_connected")
	_server.connect("client_disconnected", self, "_client_disconnected")
	_server.connect("data_received", self, "_client_receive")

func _exit_tree():
	_clients.clear()
	_server.stop()

func _process(delta):
	if _server.is_listening():
		_server.poll()

func _client_connected(id, protocol):
	_clients[id] = Utils.Peer.new(_server.get_stream_peer(id))
	Utils._log(_log_dest, "Client %s connected with protocol %s" % [id, protocol])

func _client_disconnected(id):
	Utils._log(_log_dest, "Client %s disconnected" % id)
	if _clients.has(id):
		_clients[id].free()
		_clients.erase(id)

func _client_receive(id):
	var data = _clients[id].recv()
	Utils._log(_log_dest, "Data from %s: %s" % [id, str(data)])
	# Echo
	_clients[id].send(data)

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