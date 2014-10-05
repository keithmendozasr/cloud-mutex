AM_CPPFLAGS = -I$(top_builddir)/include -I ../include
bin_PROGRAMS = cmclient

BUILT_SOURCES = $(top_builddir)/common/src/socketinfo.cpp

cmclient_SOURCES=cmclient_test.cpp \
    cmclient.cpp \
    clientsocket.cpp \
    $(top_builddir)/common/src/socketinfo.cpp

cmclient_LDADD = -llog4cplus
