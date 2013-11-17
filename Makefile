# vim: noet:ts=4:sw=4

.PHONY: cmserver cmclientdemo all

all: cmserver

cmserver:
	make -C src/server
	
debug:
	make -C src/server debug

clean:
	make -C src/server clean
