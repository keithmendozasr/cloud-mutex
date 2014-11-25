AM_CXXFLAGS+=$(GTEST_FLAGS) $(BASE_CXXFLAGS)
BASE_LDADD=-llog4cplus -L$(GTEST_DIR)/lib -lgtest_main
BASE_LDFLAGS=-static

check_PROGRAMS=socketinfo serversocket

TESTS=$(check_PROGRAMS)

socketinfo_SOURCES=\
    syswrap.cpp \
    socketinfo_test.cpp \
    $(top_builddir)/common/src/socketinfo.cpp \
    $(GTEST_DIR)/src/gtest-all.cc

socketinfo_CXXFLAGS=$(AM_CXXFLAGS) -I$(top_builddir)/include/common
socketinfo_LDADD=$(BASE_LDADD)
socketinfo_LDFLAGS=$(BASE_LDFLAGS) -Xlinker "--wrap=select,--wrap=read,--wrap=write,--wrap=socket"

serversocket_SOURCES=\
    serversocket_test.cpp \
    $(top_builddir)/server/src/serversocket.cpp \
    $(top_builddir)/common/src/socketinfo.cpp \
    $(GTEST_DIR)/src/gtest-all.cc

serversocket_CXXFLAGS=$(AM_CXXFLAGS) -I$(top_builddir)/server/include -I$(top_builddir)/common/include
serversocket_LDADD=$(BASE_LDADD)
serversocket_LDFLAGS=$(BASE_LDFLAGS)
