AM_CXXFLAGS+=$(GTEST_FLAGS) $(BASE_CXXFLAGS)
check_PROGRAMS=socketinfo

TESTS=$(check_PROGRAMS)

socketinfo_SOURCES=\
    socketinfo_test.cpp \
    $(top_builddir)/common/src/socketinfo.cpp \
    $(GTEST_DIR)/src/gtest-all.cc

socketinfo_CXXFLAGS=$(AM_CXXFLAGS) -I$(top_builddir)/include/common
socketinfo_LDADD=-llog4cplus -L$(GTEST_DIR)/lib -lgtest_main
socketinfo_LDFLAGS=-static -Xlinker "--wrap=select,--wrap=read,--wrap=write,--wrap=socket"
