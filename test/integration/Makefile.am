AM_CXXFLAGS+=$(BASE_CXXFLAGS) -I$(top_builddir)/client/include
check_PROGRAMS = cmclient

cmclient_SOURCES=cmclient.cpp 

cmclient_LDFLAGS = -L$(top_builddir)/client/src/
cmclient_LDADD = -llog4cplus -lcloudmutex
