# vim: noet:ts=4:sw=4

.PHONY: cmserver cmclient

CXX=g++
CXXFLAGS=-Wall -Werror -std=c++11 -I../../include/ -I../include/ -Wno-unused-value
COMMON_DIR=../../common/src
COMMON_OBJS=$(COMMON_DIR)/socketinfo.o

export CXX
export CXXFLAGS
export COMMON_DIR
export COMMON_OBJS

all debug clean: cmserver cmclient

cmserver:
	make -C server/src $(MAKECMDGOALS)

cmclient:
	make -C client/src $(MAKECMDGOALS)
