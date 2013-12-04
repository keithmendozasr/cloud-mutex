# vim: noet:ts=4:sw=4

.PHONY: cmserver cmclient

CXX=g++
CXXFLAGS=-Wall -Werror -std=c++0x -I../../include/ -Wno-unused-value
COMMON_DIR=../common

export CXX
export CXXFLAGS
export COMMON_DIR

all debug clean: cmserver cmclient

cmserver:
	make -C src/server $(MAKECMDGOALS)

cmclient:
	make -C src/client $(MAKECMDGOALS)
