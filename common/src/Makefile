.PHONY: all debug

OBJS=socketinfo.o

DEPS=$(patsubst %.o, %.d, $(OBJS))

all: CXXFLAGS+=-O2
all: common

debug: CXXFLAGS+=-ggdb3 -O0
debug: common

common: $(DEPS) $(OBJS) ;

%.d: %.cpp
	$(CXX) $(CXXFLAGS) -MM -MF $@ $<

clean:
	-rm $(OBJS) $(DEPS)

-include $(DEPS)
