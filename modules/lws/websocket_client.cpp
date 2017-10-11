#include "websocket_client.h"
#include "core/io/ip.h"

void WebSocketClient::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_peer"), &WebSocketClient::get_peer);
	ClassDB::bind_method(D_METHOD("is_connected_to_host"), &WebSocketClient::is_connected_to_host);
	ClassDB::bind_method(D_METHOD("is_connecting_to_host"), &WebSocketClient::is_connecting_to_host);
	ClassDB::bind_method(D_METHOD("connect_to_host", "host", "port", "protocols"), &WebSocketClient::connect_to_host, DEFVAL(PoolVector<String>()));
	ClassDB::bind_method(D_METHOD("disconnect_from_host"), &WebSocketClient::disconnect_from_host);

	ADD_SIGNAL(MethodInfo("data_received"));
	ADD_SIGNAL(MethodInfo("connection_established", PropertyInfo(Variant::STRING, "protocol")));
	ADD_SIGNAL(MethodInfo("connection_closed"));
	ADD_SIGNAL(MethodInfo("connection_error"));
}

Error WebSocketClient::connect_to_host(String p_host, uint16_t p_port, PoolVector<String> p_protocols) {

	disconnect_from_host();

	String host = p_host;
	String path = "/";
	IP_Address addr;
	int p_len = -1;
	bool ssl = false;
	if (host.begins_with("wss://")) {
		ssl = true; // we should implement this
		host = host.substr(6, host.length() - 6);
	} else {
		ssl = false;
		if (host.begins_with("ws://"))
			host = host.substr(5, host.length() - 5);
	}

	p_len = host.find("/");
	if (p_len != -1) {
		path = host.substr(p_len, host.length() - p_len);
		host = host.substr(0, p_len);
	}

	if (!host.is_valid_ip_address()) {
		addr = IP::get_singleton()->resolve_hostname(host);
	} else {
		addr = host;
	}

	ERR_FAIL_COND_V(!addr.is_valid(), ERR_INVALID_PARAMETER);

	// prepare protocols
	if (p_protocols.size() == 0) // default to binary protocol
		p_protocols.append("binary");
	_make_protocols(p_protocols);

	// init lws client
	PoolVector<struct lws_protocols>::Read pr = protocol_structs.read();
	struct lws_context_creation_info info;
	struct lws_client_connect_info i;

	memset(&i, 0, sizeof i);
	memset(&info, 0, sizeof info);

	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = &pr[0];
	info.gid = -1;
	info.uid = -1;
	//info.ws_ping_pong_interval = 5;
	info.user = this;
	context = lws_create_context(&info);

	ERR_FAIL_COND_V(context == NULL, FAILED);

	char abuf[1024];
	char hbuf[1024];
	char pbuf[2048];
	String addr_str = (String)addr;
	strncpy(abuf, addr_str.ascii().get_data(), 1024);
	strncpy(hbuf, host.utf8().get_data(), 1024);
	strncpy(pbuf, path.utf8().get_data(), 2048);

	i.context = context;
	i.protocol = protocol_string.get_data();
	i.address = abuf;
	i.host = hbuf;
	i.path = pbuf;
	i.port = p_port;
	i.ssl_connection = ssl;

	lws_client_connect_via_info(&i);
	return OK;
};

int WebSocketClient::_handle_cb(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {

	WebSocketPeer::PeerData *peer_data = (WebSocketPeer::PeerData *)user;

	switch (reason) {

		case LWS_CALLBACK_CLIENT_ESTABLISHED:
			peer = Ref<WebSocketPeer>(memnew(WebSocketPeer));
			peer->set_wsi(wsi);
			peer_data->peer_id = 0;
			peer_data->rbw.resize(16);
			peer_data->rbr.resize(16);
			peer_data->force_close = false;
			emit_signal("connection_established", lws_get_protocol(wsi)->name);
			break;

		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
			emit_signal("connection_error");
			destroy_context();
			break;

		case LWS_CALLBACK_CLOSED:
			peer_data->rbw.resize(0);
			peer_data->rbr.resize(0);
			if (peer.is_valid())
				peer->close();
			peer = Ref<WebSocketPeer>();
			destroy_context();
			emit_signal("connection_closed");
			return 0; // we can end here

		case LWS_CALLBACK_CLIENT_RECEIVE:
			if (peer.is_valid())
				peer->read_wsi(in, len);
			emit_signal("data_received");
			break;

		case LWS_CALLBACK_CLIENT_WRITEABLE:
			if (peer_data->force_close)
				return -1;

			if (peer.is_valid())
				peer->write_wsi();
			break;

		default:
			break;
	}

	return 0;
}

Ref<WebSocketPeer> WebSocketClient::get_peer() const {

	return peer;
}

bool WebSocketClient::is_connected_to_host() const {

	return peer.is_valid();
};

bool WebSocketClient::is_connecting_to_host() const {

	return context != NULL;
};

void WebSocketClient::disconnect_from_host() {

	if (context == NULL)
		return;

	if (peer.is_valid())
		peer->close();
	peer = Ref<WebSocketPeer>();

	destroy_context();
};

IP_Address WebSocketClient::get_connected_host() const {

	return IP_Address();
};

uint16_t WebSocketClient::get_connected_port() const {

	return 1025;
};

WebSocketClient::WebSocketClient() {
	context = NULL;
	free_context = false;
	is_polling = false;
};

WebSocketClient::~WebSocketClient() {

	disconnect_from_host();
};
