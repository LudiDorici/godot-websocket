#ifndef WEBSOCKETMACTOS_H
#define WEBSOCKETMACTOS_H

#define GDCICLASS(CNAME) \
public:\
	static CNAME *(*_create)();\
\
	static Ref<CNAME > create_ref() {\
\
		if (!_create)\
			return Ref<CNAME >();\
		return Ref<CNAME >(_create());\
	}\
\
	static CNAME *create() {\
\
		if (!_create)\
			return NULL;\
		return _create();\
	}\
protected:\

#define GDCINULL(CNAME) \
CNAME *(*CNAME::_create)() = NULL;

#define GDCIIMPL(IMPNAME, CNAME) \
public:\
	static CNAME *_create() { return memnew(IMPNAME); }\
	static void make_default() { CNAME::_create = IMPNAME::_create; }\
protected:\



#endif // WEBSOCKETMACTOS_H

