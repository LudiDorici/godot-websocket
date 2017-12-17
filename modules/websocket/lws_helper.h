/*************************************************************************/
/*  lws_helper.h                                                         */
/*************************************************************************/
/*                       This file is part of:                           */
/*                      GODOT WEBSOCKET MODULE                           */
/*            https://github.com/LudiDorici/godot-websocket              */
/*************************************************************************/
/* Copyright (c) 2017 Ludi Dorici, di Alessandrelli Fabio                */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#ifndef LWS_HELPER_H
#define LWS_HELPER_H

#include "core/io/stream_peer.h"
#include "core/os/os.h"
#include "core/reference.h"
#include "core/ring_buffer.h"
#include "lws_peer.h"

struct _LWSRef {
	bool free_context;
	bool is_polling;
	bool is_valid;
	bool is_destroying;
	void *obj;
};

static _LWSRef *_lws_create_ref(void *obj) {

	_LWSRef *out = (_LWSRef *)memalloc(sizeof(_LWSRef));
	out->is_destroying = false;
	out->free_context = false;
	out->is_polling = false;
	out->obj = obj;
	out->is_valid = true;
	return out;
}

static bool _lws_free_ref(_LWSRef *ref) {
	if (ref == NULL)
		return false;

	if (ref->is_polling) {
		ref->is_valid = false;
		return false;
	}

	memfree(ref);
	return true;
}

static bool _lws_destroy(struct lws_context *context, _LWSRef *ref) {
	if (context == NULL || ref->is_destroying)
		return false;

	if (ref->is_polling) {
		ref->free_context = true;
		return false;
	}

	ref->is_destroying = true;
	lws_context_destroy(context);
	_lws_free_ref(ref);
	return true;
}

static bool _lws_poll(struct lws_context *context, _LWSRef *ref) {

	ERR_FAIL_COND_V(context == NULL, false);
	ERR_FAIL_COND_V(ref == NULL, false);

	ref->is_polling = true;
	lws_service(context, 0);
	ref->is_polling = false;

	if (!ref->free_context)
		return false; // Nothing to do

	bool is_valid = ref->is_valid; // Might have been destroyed by poll

	_lws_destroy(context, ref); // Will destroy context and ref

	return is_valid; // If the object should NULL its context and ref
}

/* clang-format off */
#define LWS_HELPER(CNAME) \
protected:															\
	struct _LWSRef *_this_ref;												\
	struct lws_context *context;												\
	bool is_polling;													\
	bool free_context;													\
	PoolVector<CharString> protocol_names;											\
	PoolVector<struct lws_protocols> protocol_structs;									\
	CharString protocol_string;												\
																\
	static int _lws_gd_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {	\
																\
		if (wsi == NULL) {												\
			return 0;												\
		}														\
																\
		struct _LWSRef *ref = (struct _LWSRef *)lws_context_user(lws_get_context(wsi));					\
		if (!ref->is_valid)												\
			return 1;												\
		CNAME *helper = (CNAME *)ref->obj;										\
		return helper->_handle_cb(wsi, reason, user, in, len);								\
	}															\
																\
	void invalidate_lws_ref() {												\
		if (_this_ref != NULL)												\
			_this_ref->is_valid = false;										\
	}															\
																\
	_LWSRef *get_lws_ref() {												\
		if (_this_ref == NULL)												\
			_this_ref = _lws_create_ref(this);									\
		return _this_ref;												\
	}															\
																\
	/*															\
	 * prepare the protocol_structs to be fed to context									\
	 * also prepare the protocol string used by the client									\
	 */															\
	void _make_protocols(PoolVector<String> names) {									\
		int i;														\
		int len = names.size();												\
		size_t data_size = sizeof(struct LWSPeer::PeerData);								\
		protocol_structs.resize(len + 2);										\
		protocol_names.resize(len);											\
																\
		/* set the protocol string for client */									\
		protocol_string = names.join(",").ascii();									\
																\
		PoolVector<struct lws_protocols>::Write psw = protocol_structs.write();						\
		PoolVector<String>::Read pnr = names.read();									\
		PoolVector<CharString>::Write pnw = protocol_names.write();							\
																\
		/* the first protocol is always http-only */									\
		psw[0].name = "http-only";											\
		psw[0].callback = &CNAME::_lws_gd_callback;									\
		psw[0].per_session_data_size = data_size;									\
		psw[0].rx_buffer_size = 0;											\
		/* add user defined protocols */										\
		for (i = 0; i < len; i++) {											\
			/* the input strings might go away after this call,							\
			 * we need to copy them. Will clear them when								\
			 * detroying the context */										\
			pnw[i] = pnr[i].ascii();										\
			psw[i + 1].name = pnw[i].get_data();									\
			psw[i + 1].callback = &CNAME::_lws_gd_callback;								\
			psw[i + 1].per_session_data_size = data_size;								\
			psw[i + 1].rx_buffer_size = 0;										\
		}														\
		/* add protocols terminator */											\
		psw[len + 1].name = NULL;											\
		psw[len + 1].callback = NULL;											\
		psw[len + 1].per_session_data_size = 0;										\
		psw[len + 1].rx_buffer_size = 0;										\
	}															\
																\
	void destroy_context() {												\
		if (_lws_destroy(context, _this_ref)) {										\
			protocol_structs.resize(0);										\
			protocol_names.resize(0);										\
			protocol_string.resize(0);										\
			context = NULL;												\
			_this_ref = NULL;											\
		}														\
	}															\
																\
public:																\
	virtual int _handle_cb(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);		\
																\
	void _lws_poll() {													\
		ERR_FAIL_COND(context == NULL);											\
																\
		if (::_lws_poll(context, _this_ref)) {										\
			context = NULL;												\
			_this_ref = NULL;											\
		}														\
	}															\
																\
	PoolVector<String> get_protocols() const {										\
		int i = 0;													\
		PoolVector<String> out;												\
		for (i = 0; i < protocol_names.size(); i++) {									\
			out.append(String(protocol_names[i].get_data()));							\
		}														\
		return out;													\
	}															\
																\
protected:

/* clang-format on */

#endif // LWS_HELPER_H
