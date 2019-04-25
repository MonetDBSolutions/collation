# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0.  If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright 2013-2018 MonetDB B.V.

#LIBDIR = $(shell pkg-config --variable=libdir monetdb5)

LIBDIR = $(shell pkg-config --variable=libdir monetdb5)

CFLAGS += $(shell pkg-config --cflags monetdb5) -g
LDFLAGS += $(shell pkg-config --libs monetdb5)

all: lib_collation.so

lib_collation.so: collation.o
	$(CC) -fPIC -DPIC -o lib_collation.so -shared collation.o $(LDFLAGS) -Wl,-soname -Wl,lib_collation.so

collation.o: collation.c
	$(CC) -fPIC -DPIC $(CFLAGS) -c collation.c

clean:
	rm -f *.o *.so

install: lib_collation.so
	cp collation.mal lib_collation.so $(DESTDIR)$(LIBDIR)/monetdb5
	cp 82_collation.sql $(DESTDIR)$(LIBDIR)/monetdb5/createdb
	cp 82_collation.mal $(DESTDIR)$(LIBDIR)/monetdb5/autoload
