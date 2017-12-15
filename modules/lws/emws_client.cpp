#ifdef JAVASCRIPT_ENABLED

#include "emscripten.h"
#include "emws_client.h"
#include "core/io/ip.h"

extern "C" {
void _esws_on_connect(void *obj, char *proto) {
	EMWSClient *client = static_cast<EMWSClient *>(obj);
	client->emit_signal("connection_established", String(proto));
}

void _esws_on_message(void *obj, uint8_t *p_data, int p_data_size, int p_is_string)  {
	EMWSClient *client = static_cast<EMWSClient *>(obj);

	static_cast<EMWSPeer *>(*client->get_peer())->read_msg(p_data, p_data_size, p_is_string == 1);
	client->emit_signal("data_received");
}

void _esws_on_error(void *obj)  {
	EMWSClient *client = static_cast<EMWSClient *>(obj);
	client->emit_signal("connection_error");
}

void _esws_on_close(void *obj, int code, char* reason, int was_clean)  {
	EMWSClient *client = static_cast<EMWSClient *>(obj);
	client->emit_signal("connection_closed");
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

	peer = Ref<EMWSPeer>(memnew(EMWSPeer));
	peer->set_sock(peer_sock);

	return OK;
};

void EMWSClient::poll() {

}

Ref<WebSocketPeer> EMWSClient::get_peer() const {

	return peer;
}

bool EMWSClient::is_connected_to_host() const {

	return peer.is_valid() && peer->is_connected_to_host();
};

bool EMWSClient::is_connecting_to_host() const {

	return false;
};

void EMWSClient::disconnect_from_host() {

	peer->close();
};

IP_Address EMWSClient::get_connected_host() const {

	return IP_Address();
};

uint16_t EMWSClient::get_connected_port() const {

	return 1025;
};

EMWSClient::EMWSClient() {
	peer = Ref<EMWSPeer>();
};

EMWSClient::~EMWSClient() {

	disconnect_from_host();
};

#endif // JAVASCRIPT_ENABLED
