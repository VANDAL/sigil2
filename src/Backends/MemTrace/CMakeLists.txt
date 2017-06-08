# required CapnProto
set(SOURCES
	Handler.cpp
	CapnLogger.cpp
	MemTrace.capnp.c++
	${THIRD_PARTY}/zlib/contrib/iostream3/zfstream.cc)
add_library(MemTrace STATIC ${SOURCES})
