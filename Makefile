# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0.  If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright 2013-2018 MonetDB B.V.

LIBDIR = $(shell pkg-config --variable=libdir monetdb5)

CFLAGS  += -Wall $(shell pkg-config --cflags monetdb5) $(shell pkg-config --cflags icu-i18n) $(shell pkg-config --cflags icu-io) $(shell pkg-config --cflags icu-lx) $(shell pkg-config --cflags icu-uc) -g
LDFLAGS += $(shell pkg-config --libs monetdb5)   $(shell pkg-config --libs icu-i18n)   $(shell pkg-config --libs icu-io)   $(shell pkg-config --libs icu-lx)   $(shell pkg-config --libs icu-uc)

all: lib_collation.so

lib_collation.so: like_match.o dfa.o sort_key.o locales.o
	$(CC) -fPIC -DPIC -shared -o lib_collation.so dfa.o -shared like_match.o -shared locales.o -shared sort_key.o  $(LDFLAGS) -Wl,-soname -Wl,lib_collation.so

locales.o: locales.c
	$(CC) -fPIC -DPIC $(CFLAGS) -c locales.c

dfa.o: dfa.c
	$(CC) -fPIC -DPIC $(CFLAGS) -c dfa.c

sort_key.o: sort_key.c
	$(CC) -fPIC -DPIC $(CFLAGS) -c sort_key.c

like_match.o: like_match.c
	$(CC) -fPIC -DPIC $(CFLAGS) -c like_match.c

clean:
	rm -f *.o *.so

install: lib_collation.so
	cp collation.mal lib_collation.so $(DESTDIR)$(LIBDIR)/monetdb5
	cp 82_collation.sql $(DESTDIR)$(LIBDIR)/monetdb5/createdb
	cp 82_collation.mal $(DESTDIR)$(LIBDIR)/monetdb5/autoload
