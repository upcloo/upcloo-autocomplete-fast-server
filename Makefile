#Makefile for UpCloo Autocomplete Search

CC=gcc
CFLAGS=-Wall -levent -lmemcached
autocomplete: upcloo-search-autocomplete/upcloo-autocomplete.o upcloo-search-autocomplete/upcloo-conf.o
                gcc $(CFLAGS) upcloo-search-autocomplete/upcloo-autocomplete.o upcloo-search-autocomplete/upcloo-conf.o -o autocomplete

clean:
        rm -f upcloo-search-autocomplete/upcloo-autocomplete.o upcloo-search-autocomplete/upcloo-conf.o
