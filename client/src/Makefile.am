AM_CXXFLAGS += $(BASE_CXXFLAGS)
lib_LTLIBRARIES = libcloudmutex.la

BUILT_SOURCES = $(top_builddir)/common/src/socketinfo.cpp

libcloudmutex_la_SOURCES=cmclient.cpp \
    clientsocket.cpp \
    $(top_builddir)/common/src/socketinfo.cpp

libcloudmutex_la_LIBADD =-llog4cplus
libcloudmutex_la_LDFLAGS= -version-info 0:0:0
