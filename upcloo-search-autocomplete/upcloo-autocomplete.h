/*
 * upcloo-autocomplete.h
 *
 *  Created on: Mar 28, 2012
 *      Author: walterdalmut
 */

#ifndef UPCLOO_AUTOCOMPLETE_H_
#define UPCLOO_AUTOCOMPLETE_H_

#define SITEKEY_URL "sitekey="
#define WORD_URL "word="
#define CALLBACK_URL "callback="

//Memcached structure
struct UPCLOO_MEMCACHED_SERVER {
	char *host;
	int port;
};
typedef struct UPCLOO_MEMCACHED_SERVER upcloo_memcached_server;

//UpCloo configuration for autocomplete
struct UPCLOO_CONF {
	char *bind;
	int port;

	short int daemonize;

	upcloo_memcached_server **memcached_servers;
	int upcloo_memcached_server_count;
};
typedef struct UPCLOO_CONF upcloo_conf;

upcloo_conf *parse_user_conf(int, char **);

struct UPCLOO_REQUEST {
	char *sitekey;
	char *word;
	char *callback;
};
typedef struct UPCLOO_REQUEST upcloo_request;

upcloo_request *parse_uri(char *);
void daemonize(void);

#endif /* UPCLOO_AUTOCOMPLETE_H_ */
