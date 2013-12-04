.PHONY: cmserver

OBJS=../common/socketinfo.o \
	clientsocket.o \
    cmclient.o

DEPS=$(patsubst %.o, %.d, $(OBJS))

all: CXXFLAGS+=-O2
all: cmclient

debug: CXXFLAGS+=-ggdb3 -O0
debug: cmclient

analyze: CXXFLAGS+=--analyze
analyze: $(DEPS) $(OBJS)

cmclient: $(DEPS) $(OBJS)
	$(CXX) $(OBJS) -o cmclient -llog4cplus

%.d: %.cpp
	$(CXX) $(CXXFLAGS) -MM -MF $@ $<

clean:
	-rm cmclient $(OBJS) $(DEPS)

-include $(DEPS)
