.PHONY: cmserver

CXX=g++
CXXFLAGS=-Wall -Werror -std=c++0x -I../../include/ -Wno-unused-value

COMMON_DIR=../common

OBJS=../common/socketinfo.o \
	serversocket.o \
	clienthandler.o \
	cmserver.o

DEPS=$(patsubst %.o, %.d, $(OBJS))

all: CXXFLAGS+=-O2
all: cmserver

debug: CXXFLAGS+=-ggdb3 -O0
debug: cmserver

cmserver: $(DEPS) $(OBJS)
	$(CXX) $(OBJS) -o cmserver -llog4cplus

%.d: %.cpp
	$(CXX) $(CXXFLAGS) -MM -MF $@ $<

clean:
	-rm cmserver $(OBJS) $(DEPS)

-include $(DEPS)
