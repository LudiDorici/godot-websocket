#ifdef JAVASCRIPT_ENABLED

#include "emscripten.h"
#include "emws_client.h"
#include "core/io/ip.h"

extern "C" {
EMSCRIPTEN_KEEPALIVE void _esws_on_connect(void *obj, char *proto) {
	EMWSClient *client = static_cast<EMWSClient *>(obj);
	client->_is_connecting = false;
	client->_on_connect(String(proto));
}

EMSCRIPTEN_KEEPALIVE void _esws_on_message(void *obj, uint8_t *p_data, int p_data_size, int p_is_string)  {
	EMWSClient *client = static_cast<EMWSClient *>(obj);

	static_cast<EMWSPeer *>(*client->get_peer(1))->read_msg(p_data, p_data_size, p_is_string == 1);
	client->_on_peer_packet();
}

EMSCRIPTEN_KEEPALIVE void _esws_on_error(void *obj)  {
	EMWSClient *client = static_cast<EMWSClient *>(obj);
	client->_is_connecting = false;
	client->_on_error();
}

EMSCRIPTEN_KEEPALIVE void _esws_on_close(void *obj, int code, char* reason, int was_clean)  {
	EMWSClient *client = static_cast<EMWSClient *>(obj);
	client->_is_connecting = false;
	client->_on_disconnect();
}
}

Error EMWSClient::connect_to_host(String p_host, String p_path, uint16_t p_port, bool p_ssl, PoolVector<String> p_protocols) {

	String str = "ws://";
	String proto_string = "";
	int i = 0;

	if(p_ssl)
		str = "wss://";
	str += p_host + ":" + itos(p_port) + p_path;
	for (int i = 0; i < p_protocols.size(); i++) {
		proto_string += p_protocols[i];
		proto_string += ",";
	}
	if (proto_string == "")
		proto_string = "binary,";

	proto_string = proto_string.substr(0, proto_string.length()-1);

	_is_connecting = true;
	/* clang-format off */
	int peer_sock = EM_ASM_INT({
		var socket = new WebSocket(UTF8ToString($1), UTF8ToString($2).split(","));
		socket.binaryType = "arraybuffer";

		// Connection opened
		socket.addEventListener("open", function (event) {
			Module.ccall("_esws_on_connect",
					"void",
					["number", "string"],
					[$0, socket.protocol]
			);
		});

		// Listen for messages
		socket.addEventListener("message", function (event) {
			var buffer;
			var is_string = 0;
			if (event.data instanceof ArrayBuffer) {

				buffer = new Uint8Array(event.data);

			} else if (event.data instanceof Blob) {

				alert("Blob type not supported");
				return;

			} else if (typeof event.data === "string") {

				is_string = 1;
				var enc = new TextEncoder("utf-8");
				buffer = new Uint8Array(enc.encode(event.data));

			} else {

				alert("Unknown message type");
				return;

			}
			var len = buffer.length*buffer.BYTES_PER_ELEMENT;
			var out = Module._malloc(len);
			Module.HEAPU8.set(buffer, out);
			Module.ccall("_esws_on_message",
					"void",
					["number", "number", "number", "number"],
					[$0, out, len, is_string]
			);
			Module._free(out);
		});

		socket.addEventListener("error", function (event) {
			Module.ccall("_esws_on_error",
					"void",
					["number"],
					[$0]
			);
		});

		socket.addEventListener("close", function (event) {
			var was_clean = 0;
			if (event.was_clean)
				was_clean = 1;
			Module.ccall("_esws_on_close",
					"void",
					["number", "number", "string", "number"],
					[$0, event.code, event.reason, was_clean]
			);
		});

		return Module.IDHandler.add(socket);
	}, this, str.utf8().get_data(), proto_string.utf8().get_data());
	/* clang-format on */

	static_cast<Ref<EMWSPeer> >(_peer)->set_sock(peer_sock);

	return OK;
};

void EMWSClient::poll() {

}

Ref<WebSocketPeer> EMWSClient::get_peer(int p_peer_id) const {

	return _peer;
}

NetworkedMultiplayerPeer::ConnectionStatus EMWSClient::get_connection_status() const {

	if (_peer->is_connected_to_host())
		return CONNECTION_CONNECTED;

	if (_is_connecting)
		return CONNECTION_CONNECTING;

	return CONNECTION_DISCONNECTED;
};

void EMWSClient::disconnect_from_host() {

	_peer->close();
};

IP_Address EMWSClient::get_connected_host() const {

	return IP_Address();
};

uint16_t EMWSClient::get_connected_port() const {

	return 1025;
};

EMWSClient::EMWSClient() {
	_is_connecting = false;
	_peer = Ref<EMWSPeer>(memnew(EMWSPeer));
};

EMWSClient::~EMWSClient() {

	disconnect_from_host();
	_peer = Ref<EMWSPeer>();
};

#endif // JAVASCRIPT_ENABLED
