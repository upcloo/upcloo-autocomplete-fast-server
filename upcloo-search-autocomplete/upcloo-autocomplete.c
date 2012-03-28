/*
 * upcloo-autocomplete.c
 *
 *  Created on: Mar 28, 2012
 *      Author: walterdalmut
 */

#include "upcloo-autocomplete.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <event.h>
#include <evhttp.h>
#include <libmemcached/memcached.h>

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include "upcloo-conf.h"

memcached_st *memcached_server;

//Main autocomplete hook (event-driven httpd)
void upcloo_autocomplete_handler(struct evhttp_request *req, void *arg) {
	struct evbuffer *buffer;
	buffer = evbuffer_new();

	if (buffer == NULL) {
		err(1, "failed to create response buffer");
	}

	upcloo_request *request = parse_uri(req->uri);

	if (request != NULL) {

		/* Fetch from memcached */
		memcached_return rc;

		char *proposals;
		size_t string_length;
		uint32_t flags;

		//Create the memcache key!
		char *key = (char *)malloc((strlen(request->sitekey) + strlen(request->word) +2)*sizeof(char));
		sprintf(key, "%s_%s", request->sitekey, request->word);

		proposals = memcached_get(memcached_server, key, strlen(key), &string_length, &flags, &rc);

		if (proposals) {
			char * jsonp = (char *)malloc((strlen(request->callback)+2+strlen(proposals))*sizeof(char));
			sprintf(jsonp, "%s(%s)", request->callback, proposals);

			evbuffer_add_printf(buffer, "%s", jsonp);
			evhttp_send_reply(req, HTTP_OK, "OK", buffer);

			free(jsonp);
		} else {
			evbuffer_add_printf(buffer, "%s", key);
			evhttp_send_reply(req, HTTP_NOTFOUND, "MISSING CACHE", NULL);
		}

		free(request);
		free(proposals);
		free(key);
	} else {
		evhttp_send_reply(req, HTTP_BADREQUEST, "You have to set sitekey, word pattern and the JSONP callback", NULL);
	}

	evbuffer_free(buffer);
}

/**
 * This function parse the uri and create a valid response.
 * If not found NULL is returned
 */
char *upcloo_parse_key(const char *string, const char *search) {
	char *result = (char *)malloc(strlen(string)*sizeof(char));
	strcpy(result, string);
	char *s = strstr(result, search);

	if (s != NULL) {
		strcpy(result, s);
		result = result+strlen(search);

		char *t = strchr(result, '&');
		if (t != NULL) {
			*t = '\0';
		}
	} else {
		free(result);
		result = NULL;
	}

	//TODO: handle url characters
	return result;
}

upcloo_request *parse_uri(char *uri)
{
	upcloo_request *request = (upcloo_request *)malloc(sizeof(upcloo_request));

	char *sitekey = upcloo_parse_key(uri, "sitekey=");
	char *word = upcloo_parse_key(uri, "word=");
	char *callback = upcloo_parse_key(uri, "callback=");

	if (sitekey == NULL || word == NULL || callback == NULL) {
		free(request);
		return NULL;
	} else {
		request->sitekey = sitekey;
		request->word = word;
		request->callback = callback;

		return request;
	}
}

int main(int argc, char **argv) {
	upcloo_conf *conf = parse_user_conf(argc, argv);

	if (conf->daemonize == 1) daemonize();

	memcached_server = memcached_create(NULL);

	struct evhttp *evhttp;

	//Start Memcached structure
	int index;
	//Add memcached servers
	for (index = 0; index<conf->upcloo_memcached_server_count; index++) {
		memcached_server_add(
				memcached_server,
				conf->memcached_servers[index]->host,
				conf->memcached_servers[index]->port);
	}

	//Start event-driven Server
	event_init();
	evhttp = evhttp_start(conf->bind, conf->port);
	evhttp_set_gencb(evhttp, upcloo_autocomplete_handler, NULL);

	event_dispatch();

	evhttp_free(evhttp);
	memcached_free(memcached_server);

	return EXIT_SUCCESS;
}

/*
 * Parse user configuration file
 */
upcloo_conf *parse_user_conf(int argc, char **argv)
{
	upcloo_conf *conf = (upcloo_conf *)malloc(sizeof(upcloo_conf));

	if (argc <= 1) {
		conf->bind = "127.0.0.1";
		conf->port = 8080;

		upcloo_memcached_server **servers = (upcloo_memcached_server **)malloc(sizeof(upcloo_memcached_server *));

		servers[0] = (upcloo_memcached_server *)malloc(sizeof(upcloo_memcached_server));
		servers[0]->host = "127.0.0.1";
		servers[0]->port = 11211;
		conf->memcached_servers = servers;
		conf->upcloo_memcached_server_count = 1;
	} else {
		read_conf_from_file(argv[1], conf);
	}

	return conf;
}

void daemonize(void)
{
	int fd;

	if (fork() != 0) exit(0); /* parent exits */
	setsid(); /* create a new session */

	/* Every output goes to /dev/null. If Redis is daemonized but
	 * the 'logfile' is set to 'stdout' in the configuration file
	 * it will not log at all. */
	if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		if (fd > STDERR_FILENO) close(fd);
	}
}
