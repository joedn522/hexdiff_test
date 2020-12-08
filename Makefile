#
#			VISUEL HEXDIFF
#			--------------
#
#		http://tboudet.free.fr/hexdiff/
#



#
#	if you define TRACE to a non zero value, you get a 
#	lot of debugging trace on _stderr_
#

CFLAGS+=-g -DVERSION=\"$(VERSION)\" -DTRACE=0  -ansi
LDFLAGS+=-lncurses 

PREFIX=/usr/local
VERSION=$(shell dirname $(PWD)/. | sed "s/^.*-//")
PROJECT=hexdiff
TARNAME=$(PROJECT)-$(VERSION).tar.gz
EXEC=$(PROJECT)
SRC=${wildcard *.c}
OBJECTS=${SRC:.c=.o}
FILES=$(shell cat MANIFEST)
#hexdiff.c hexdiff.fr_FR.1 Makefile README TODO BUGS fonctions.c CHANGES \
#	  COPYING hexdiff.h  parse_rc.c hexdiff.rc fileinfo.c sel_file.c \
#	  asciiview.c memcheck.sh

all: ${EXEC}

$(EXEC): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

install: ${EXEC}
	@echo "Stripping $(EXEC)..."
	@strip $(EXEC)
	@echo "Installing exec and man pages"
	@install -m 744 $(EXEC) $(PREFIX)/bin/
	@mkdir -p $(PREFIX)/share/man/man1
	@install -m 644 hexdiff.1 $(PREFIX)/share/man/man1

.PHONY: clean mrproper

clean:
	@rm -f $(OBJECTS)

mrproper: clean
	@rm -f $(EXEC)


tarball:  mrproper $(FILES) 
	@echo "Creating $(TARNAME)"
	tar -C .. -czvf ../$(TARNAME) hexdiff-$(VERSION)
	date >> tarball

lines:	$(FILES)
	wc $(FILES) | sort -n
