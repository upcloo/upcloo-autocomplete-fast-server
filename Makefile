#Makefile for UpCloo Autocomplete Search

PREFIX=/usr/local

CC=gcc
CFLAGS=-Wall -levent -lmemcached
autocomplete: upcloo-search-autocomplete/upcloo-autocomplete.o upcloo-search-autocomplete/upcloo-conf.o
	$(CC) $(CFLAGS) upcloo-search-autocomplete/upcloo-autocomplete.o upcloo-search-autocomplete/upcloo-conf.o -o autocomplete

clean: 
	rm -f upcloo-search-autocomplete/upcloo-autocomplete.o upcloo-search-autocomplete/upcloo-conf.o

install: all
	cp autocomplete $PREFIX/bin
