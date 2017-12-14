#ifdef JAVASCRIPT_ENABLED

#include "emscripten.h"
#include "esws_client.h"
#include "core/io/ip.h"

extern "C" {
void _esws_on_connect(void *obj, char *proto) {
	ESWSClient *client = static_cast<ESWSClient *>(obj);
	client->emit_signal("connection_established", String(proto));
}

void _esws_on_message(void *obj, uint8_t *p_data, int p_data_size)  {
	ESWSClient *client = static_cast<ESWSClient *>(obj);

	static_cast<ESWSPeer *>(*client->get_peer())->read_msg(p_data, p_data_size);
	client->emit_signal("data_received");
}

void _esws_on_error(void *obj)  {
	ESWSClient *client = static_cast<ESWSClient *>(obj);
	client->emit_signal("connection_error");
}

void _esws_on_close(void *obj, int code, char* reason, int was_clean)  {
	ESWSClient *client = static_cast<ESWSClient *>(obj);
	client->emit_signal("connection_closed");
}
}

Error ESWSClient::connect_to_host(String p_host, uint16_t p_port, PoolVector<String> p_protocols) {
	/* clang-format off */
	int peer_sock = EM_ASM_INT({
		var socket = new WebSocket(UTF8ToString($0) + ":" + $1);

		// Connection opened
		socket.addEventListener("open", function (event) {
			Module.ccall("_esws_on_connect",
					"void",
					["number", "string"],
					[$2, socket.protocol]
			);
		});

		// Listen for messages
		socket.addEventListener("message", function (event) {
			var buffer;
			if (event.data instanceof ArrayBuffer) {

				buffer = new Uint8Array(event.data);

			} else if (event.data instanceof Blob) {

				alert("Blob type not supported");
				return;

			} else if (typeof event.data === "string") {

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
					["number", "number", "number"],
					[$2, out, len]
			);
			Module._free(out);
		});

		socket.addEventListener("error", function (event) {
			Module.ccall("_esws_on_error",
					"void",
					["number"],
					[$2]
			);
		});

		socket.addEventListener("close", function (event) {
			var was_clean = 0;
			if (event.was_clean)
				was_clean = 1;
			Module.ccall("_esws_on_close",
					"void",
					["number", "number", "string", "number"],
					[$2, event.code, event.reason, was_clean]
			);
		});

		return Module.IDHandler.add(socket);
	}, p_host.utf8().get_data(), p_port, this);
	/* clang-format on */

	peer = Ref<ESWSPeer>(memnew(ESWSPeer));
	peer->set_sock(peer_sock);

	return OK;
};

void ESWSClient::poll() {

}

Ref<WebSocketPeer> ESWSClient::get_peer() const {

	return peer;
}

bool ESWSClient::is_connected_to_host() const {

	return peer.is_valid() && peer->is_connected_to_host();
};

bool ESWSClient::is_connecting_to_host() const {

	return false;
};

void ESWSClient::disconnect_from_host() {

	peer->close();
};

IP_Address ESWSClient::get_connected_host() const {

	return IP_Address();
};

uint16_t ESWSClient::get_connected_port() const {

	return 1025;
};

ESWSClient::ESWSClient() {
	peer = Ref<ESWSPeer>();
};

ESWSClient::~ESWSClient() {

	disconnect_from_host();
};

#endif // JAVASCRIPT_ENABLED
