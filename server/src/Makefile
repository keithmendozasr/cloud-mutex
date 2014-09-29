.PHONY: all debug

OBJS=serversocket.o \
	clienthandler.o \
	cmserver.o

DEPS=$(patsubst %.o, %.d, $(OBJS))

all: CXXFLAGS+=-O2
all: cmserver

debug: CXXFLAGS+=-ggdb3 -O0
debug: cmserver

cmserver: $(DEPS) $(OBJS) $(COMMON_OBJS)
	$(CXX) $(OBJS) $(COMMON_OBJS) -o cmserver -llog4cplus

$(COMMON_OBJS):
	make -C $(COMMON_DIR) $(MAKECMDGOALS)

%.d: %.cpp
	$(CXX) $(CXXFLAGS) -MM -MF $@ $<

clean:
	-rm cmserver $(OBJS) $(DEPS)

-include $(DEPS)
