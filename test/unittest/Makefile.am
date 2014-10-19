AM_CXXFLAGS+=$(GTEST_FLAGS) $(BASE_CXXFLAGS)
check_PROGRAMS = socketinfo

TESTS=$(check_PROGRAMS)

socketinfo_SOURCES=$(GTEST_DIR)/src/gtest-all.cc \
    socketinfo_test.cpp \
    $(top_builddir)/common/src/socketinfo.cpp \
    $(top_builddir)/include/common/socketinfo.h

socketinfo_CXXFLAGS=$(AM_CXXFLAGS) -I$(top_builddir)/include/common
LDADD=-llog4cplus -L$(GTEST_DIR)/lib -lgtest_main
AM_LDFLAGS=-static -Xlinker "--wrap=select"
