# See LICENSE file for copyright and license details
# stag - simple tag generator
.POSIX:

include config.mk

REQ = util tags
PARSERS =\
	parsers/slimwiki

all: stag

$(PARSERS:=.o): config.mk $(REQ:=.h)
stag.o: stag.c config.h

.c.o:
	$(CC) -o $@ -c $(CPPFLAGS) $(CFLAGS) $<

config.h:
	cp config.def.h $@

stag: stag.o $(PARSERS:=.o) $(REQ:=.o)
	$(CC) -o $@ $(LDFLAGS) $(PARSERS:=.o) $(REQ:=.o) stag.o $(LDLIBS)

clean:
	rm -f stag stag.o $(PARSERS:=.o) $(REQ:=.o)

dist:
	rm -rf "stag-$(VERSION)"
	mkdir -p "stag-$(VERSION)/components"
	cp -R LICENSE Makefile README config.mk config.def.h \
	      arg.h stag.c $(PARSERS:=.c) $(REQ:=.c) $(REQ:=.h) \
	      stag.1 "stag-$(VERSION)"
	tar -cf - "stag-$(VERSION)" | gzip -c > "stag-$(VERSION).tar.gz"
	rm -rf "stag-$(VERSION)"

install: all
	mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	cp -f stag "$(DESTDIR)$(PREFIX)/bin"
	chmod 755 "$(DESTDIR)$(PREFIX)/bin/stag"
	mkdir -p "$(DESTDIR)$(MANPREFIX)/man1"
	cp -f stag.1 "$(DESTDIR)$(MANPREFIX)/man1"
	chmod 644 "$(DESTDIR)$(MANPREFIX)/man1/stag.1"

uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/bin/stag"
	rm -f "$(DESTDIR)$(MANPREFIX)/man1/stag.1"
