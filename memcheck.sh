#!/bin/sh

#
#	seek for memory leaks
#	---------------------
#	you need a recent version of http://www.valgrind.org/
#
make hexdiff

rm valgr-plop* 

valgrind --tool=memcheck --log-file=valgr-plop		\
	 --leak-check=yes --show-reachable=yes \
	 ./hexdiff hexdiff.o fonctions.o

