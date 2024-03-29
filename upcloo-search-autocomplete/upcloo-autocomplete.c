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

#include "syslog.h"

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
		//TODO: fix fixed length
		char *key = (char *)malloc(256*sizeof(char));
		sprintf(key, "%s_%s", request->sitekey, request->word);

		proposals = memcached_get(memcached_server, key, strlen(key), &string_length, &flags, &rc);

		if (proposals) {
			char * jsonp = (char *)malloc(2048*sizeof(char));
			if (request->callback != NULL)  {
				sprintf(jsonp, "%s(%s)", request->callback, proposals);
			} else {
				sprintf(jsonp, "%s", proposals);
			}

			autocompleteLogRaw(LOG_INFO, "User %s:%d request served successfully.", req->remote_host, req->remote_port);

			evbuffer_add_printf(buffer, "%s", jsonp);
			evhttp_send_reply(req, HTTP_OK, "OK", buffer);

			free(jsonp);
		} else {
			autocompleteLogRaw(LOG_ERR, "Missing cache \"%s\" for user %s:%d", key, req->remote_host, req->remote_port);
			evbuffer_add_printf(buffer, "%s", key);
			evhttp_send_reply(req, HTTP_NOTFOUND, "MISSING CACHE", NULL);
		}

		free(request);
		free(proposals);
		free(key);
	} else {
		autocompleteLogRaw(LOG_CRIT, "User %s:%d have to set sitekey, word pattern and the JSONP callback for request: %s", req->remote_host, req->remote_port, req->uri);

		evhttp_send_reply(req, HTTP_BADREQUEST, "You have to set sitekey, word pattern and the JSONP callback", NULL);
	}

	evbuffer_free(buffer);
}

/**
 * This function parse the uri and create a valid response.
 * If not found NULL is returned
 */
char *upcloo_parse_key(const char *string, const char *search) {
	//TODO change this part using memory dinamically allocated and not 256 fixed
	char *result = (char *)malloc(256*sizeof(char));
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
	char *sitekey = upcloo_parse_key(uri, SITEKEY_URL);
	char *word = upcloo_parse_key(uri, WORD_URL);
	char *callback = upcloo_parse_key(uri, CALLBACK_URL);

	if (sitekey == NULL || word == NULL) {
		return NULL;
	} else {
		upcloo_request *request = (upcloo_request *)malloc(sizeof(upcloo_request));
		request->sitekey = sitekey;
		request->word = word;
		request->callback = callback;

		return request;
	}
}

void autocompleteLogRaw(int level, const char *fmt, ...) {
	char msg[LOG_MAX_ENTRY_LEN];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);

	syslog(level, "%s", msg);
}

int main(int argc, char **argv) {
	upcloo_conf *conf = parse_user_conf(argc, argv);

	openlog(SYSLOG_IDENTITY, LOG_PID | LOG_NDELAY | LOG_NOWAIT, LOG_INFO);

	signal(SIGHUP, SIG_IGN);
	if (conf->daemonize == 1) {
		autocompleteLogRaw(LOG_INFO, "Start as daemon.");
		daemonize();
	}

	memcached_server = memcached_create(NULL);

	struct evhttp *evhttp;

	//Start Memcached structure
	int index;
	//Add memcached servers
	for (index = 0; index<conf->upcloo_memcached_server_count; index++) {
		autocompleteLogRaw(LOG_INFO, "Added a memcached server %s:%d", conf->memcached_servers[index]->host, conf->memcached_servers[index]->port);
		memcached_server_add(
				memcached_server,
				conf->memcached_servers[index]->host,
				conf->memcached_servers[index]->port);
	}

	//Start event-driven Server
	autocompleteLogRaw(LOG_INFO, "Start listening for connections");
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

	/* Every output goes to /dev/null. */
	if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		if (fd > STDERR_FILENO) close(fd);
	}
}
