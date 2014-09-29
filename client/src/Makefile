.PHONY: all debug

OBJS = clientsocket.o \
    cmclient.o

DEPS=$(patsubst %.o, %.d, $(OBJS))

all: CXXFLAGS+=-O2
all: cmclient

debug: CXXFLAGS+=-ggdb3 -O0
debug: cmclient

cmclient: $(DEPS) $(OBJS) $(COMMON_OBJS)
	$(CXX) $(OBJS) $(COMMON_OBJS) -o cmclient -llog4cplus

%.d: %.cpp
	$(CXX) $(CXXFLAGS) -MM -MF $@ $<

clean:
	-rm cmclient $(OBJS) $(DEPS)

-include $(DEPS)
