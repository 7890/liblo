CC = gcc
CFLAGS = -Wall -O2
PREFIX ?= /usr/local
INSTALLDIR ?= $(PREFIX)/bin

osc_continue: osc_continue.c
	which $(CC)
	which pkg-config
	pkg-config --exists liblo
	$(CC) -o osc_continue osc_continue.c -llo

install:
	mkdir -p $(INSTALLDIR)
	install -m755 osc_continue $(INSTALLDIR)

uninstall:
	-rm -f $(INSTALLDIR)/osc_continue

clean:
	rm -f osc_continue
