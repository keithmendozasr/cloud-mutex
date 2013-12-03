# vim: noet:ts=4:sw=4

.PHONY: cmserver

all debug clean: cmserver

cmserver:
	make -C src/server $(MAKECMDGOALS)
