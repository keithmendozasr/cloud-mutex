AM_CPPFLAGS = -I$(top_builddir)/include -I ../include -I$(top_builddir)/client/include
check_PROGRAMS = cmclient

BUILT_SOURCES = $(top_builddir)/common/src/socketinfo.cpp

cmclient_SOURCES=cmclient.cpp 

cmclient_LDFLAGS = -L$(top_builddir)/client/src
cmclient_LDADD = -llog4cplus -lcloudmutex
