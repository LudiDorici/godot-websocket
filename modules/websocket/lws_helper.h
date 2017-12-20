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
	struct lws_protocols *lws_structs;
	char *lws_strings;
};

static _LWSRef *_lws_create_ref(void *obj, struct lws_protocols *lws_structs, char *lws_strings) {

	_LWSRef *out = (_LWSRef *)memalloc(sizeof(_LWSRef));
	out->is_destroying = false;
	out->free_context = false;
	out->is_polling = false;
	out->obj = obj;
	out->is_valid = true;
	out->lws_structs = lws_structs;
	out->lws_strings = lws_strings;
	return out;
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
	// Free strings and structs
	memfree(ref->lws_structs);
	memfree(ref->lws_strings);
	// Free ref
	memfree(ref);
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
	struct lws_protocols *_lws_structs;											\
	char *_lws_strings;													\
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
			_this_ref = _lws_create_ref(this, _lws_structs, _lws_strings);						\
		return _this_ref;												\
	}															\
																\
	/*															\
	 * prepare the protocol_structs to be fed to context									\
	 * also prepare the protocol string used by the client									\
	 */															\
	void _make_protocols(PoolVector<String> p_names) {									\
		/* the input strings might go away after this call,								\
		 * we need to copy them. Will clear them when									\
		 * detroying the context */											\
		int i;														\
		int len = p_names.size();											\
		size_t data_size = sizeof(struct LWSPeer::PeerData);								\
		PoolVector<String>::Read pnr = p_names.read();									\
																\
		/* LWS protocol structs (for server) */										\
		_lws_structs = (struct lws_protocols *)memalloc(sizeof(struct lws_protocols)*(len+2));				\
																\
		CharString strings = p_names.join(",").ascii();									\
		int str_len = strings.length();											\
		/* Joined string of protocols, double the size: comma separated first, NULL separated last */			\
		_lws_strings = (char *)memalloc((str_len + 1) * 2); /* plus the terminator */					\
		copymem(_lws_strings, strings.get_data(), str_len);								\
		_lws_strings[str_len] = '\0'; /* NULL terminator */								\
		/* NULL terminated strings to be used in protocol structs */							\
		copymem(&_lws_strings[str_len+1], strings.get_data(), str_len);							\
		_lws_strings[(str_len*2)+1] = '\0'; /* NULL terminator */							\
		int pos = str_len+1;												\
																\
		/* the first protocol is always http-only */									\
		_lws_structs[0].name = "http-only";										\
		_lws_structs[0].callback = &CNAME::_lws_gd_callback;								\
		_lws_structs[0].per_session_data_size = data_size;								\
		_lws_structs[0].rx_buffer_size = 0;										\
		/* add user defined protocols */										\
		for (i = 0; i < len; i++) {											\
			_lws_structs[i + 1].name = (const char *)&_lws_strings[pos];						\
			_lws_structs[i + 1].callback = &CNAME::_lws_gd_callback;						\
			_lws_structs[i + 1].per_session_data_size = data_size;							\
			_lws_structs[i + 1].rx_buffer_size = 0;									\
			pos += pnr[i].ascii().length()+1;									\
			_lws_strings[pos-1] = '\0';										\
		}														\
		/* add protocols terminator */											\
		_lws_structs[len + 1].name = NULL;										\
		_lws_structs[len + 1].callback = NULL;										\
		_lws_structs[len + 1].per_session_data_size = 0;								\
		_lws_structs[len + 1].rx_buffer_size = 0;									\
	}															\
																\
	void destroy_context() {												\
		if (_lws_destroy(context, _this_ref)) {										\
			context = NULL;												\
			_this_ref = NULL;											\
			_lws_structs = NULL;											\
			_lws_strings = NULL;											\
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
protected:

/* clang-format on */

#endif // LWS_HELPER_H
