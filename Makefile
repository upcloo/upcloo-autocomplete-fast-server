#Makefile for UpCloo Autocomplete Search

PORT=8080

PREFIX=/usr/local
PREFIX_BIN=${PREFIX}/bin

CONF_DIR=/etc/autocomplete

INIT_DIR=/etc/init.d

CC=gcc
CFLAGS=-Wall -levent -lmemcached
all: upcloo-search-autocomplete/upcloo-autocomplete.o upcloo-search-autocomplete/upcloo-conf.o
	$(CC) $(CFLAGS) upcloo-search-autocomplete/upcloo-autocomplete.o upcloo-search-autocomplete/upcloo-conf.o -o autocomplete

clean: 
	rm -f upcloo-search-autocomplete/upcloo-autocomplete.o upcloo-search-autocomplete/upcloo-conf.o

install: all
	cp autocomplete ${PREFIX_BIN}
	mkdir -p ${CONF_DIR}
	cp upcloo-search.conf ${CONF_DIR}/${PORT}.conf
	cp tools/autocomplete_init_script ${INIT_DIR}/autocomplete
	chmod a+x ${INIT_DIR}/autocomplete
	update-rc.d autocomplete defaults
