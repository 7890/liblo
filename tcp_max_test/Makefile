CC = gcc
CFLAGS = -Wall -O2
PREFIX ?= /usr/local

LIBLO_FLAGS ?= `pkg-config --cflags --libs liblo`

default: all
all: send_large_tcp_osc example_tcp_server

send_large_tcp_osc: send_large_tcp_osc.c
	which $(CC)
	which pkg-config
	pkg-config --exists liblo
	$(CC) $(CFLAGS) -o send_large_tcp_osc send_large_tcp_osc.c $(LIBLO_FLAGS)

example_tcp_server: example_tcp_server.c
	which $(CC)
	which pkg-config
	pkg-config --exists liblo
	$(CC) $(CFLAGS) -o example_tcp_server example_tcp_server.c $(LIBLO_FLAGS)

clean:
	rm -f send_large_tcp_osc
	rm -f example_tcp_server
