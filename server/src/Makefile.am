AM_CPPFLAGS = -I$(top_builddir)/include -I ../include
bin_PROGRAMS = cmserver

BUILT_SOURCES = $(top_builddir)/common/src/socketinfo.cpp

cmserver_SOURCES=cmserver.cpp \
    serversocket.cpp \
    clienthandler.cpp \
    $(top_builddir)/common/src/socketinfo.cpp

cmserver_LDADD = -llog4cplus
