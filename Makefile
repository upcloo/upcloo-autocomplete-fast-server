#Makefile for UpCloo Autocomplete Search

PREFIX=/usr/local
PREFIX_BIN=${PREFIX}/bin

CC=gcc
CFLAGS=-Wall -levent -lmemcached
all: upcloo-search-autocomplete/upcloo-autocomplete.o upcloo-search-autocomplete/upcloo-conf.o
	$(CC) $(CFLAGS) upcloo-search-autocomplete/upcloo-autocomplete.o upcloo-search-autocomplete/upcloo-conf.o -o autocomplete

clean: 
	rm -f upcloo-search-autocomplete/upcloo-autocomplete.o upcloo-search-autocomplete/upcloo-conf.o

install: all
	cp autocomplete ${PREFIX_BIN}
