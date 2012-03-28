/*
 * upcloo-conf.c
 *
 *  Created on: Mar 28, 2012
 *      Author: walterdalmut
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "upcloo-conf.h"

/*
 * Remove white spaces
 */
char *trim (char *string)
{
    int i;

    while (isspace (*string)) string++;
    for (i = strlen (string) - 1; (isspace (string[i])); i--) ;
    string[i + 1] = '\0';

    return string;
}

/*
 * Read user configuration from file
 */
//TODO change completely... ugly written
void read_conf_from_file(char *path, upcloo_conf *conf)
{
	FILE *file = fopen(path, "r");

	if (file) {

		char *line = (char *)malloc(LINE_LEN * sizeof(char));
		while(fgets(line, LINE_LEN, file) != NULL) {

			if (line[0] == '#' || line[0] == '\0' || line[0] == '\n') continue;

			char *position;
			//Handle simple configuration as bind and port
			if ((position = strstr(line, BIND)) != NULL) {
				//Bind configuration
				char *v = parse_line(position);
				char *bind = (char *)malloc(strlen(v) * sizeof(char));
				strcpy(bind, v);
				conf->bind = bind;
			} else if ((position = strstr(line, PORT)) != NULL && position == line) {
				//TODO: fix port rule at beginning (memcached duplicate)
				//Port configuration
				char *port = parse_line(position);
				int iport = atoi(port);

				conf->port = iport;
			} else if ((position = strstr(line, DAEMONIZE)) != NULL) {
				char *daemonize = parse_line(position);
				if (strcmp(daemonize, "yes") == 0) {
					conf->daemonize = 1;
				} else {
					conf->daemonize = 0;
				}
			} else {
				//Handle multiple configuration as memcached.0.bind
				int i=0;
				upcloo_memcached_server **servers = (upcloo_memcached_server **)malloc(10 * sizeof(upcloo_memcached_server *));
				for(i=0; i<10; i++) {
					servers[i] = (upcloo_memcached_server *)malloc(sizeof(upcloo_memcached_server));
				}
				conf->memcached_servers = servers;

				for (i=0; ;i++) {
					char *memline = (char *)malloc(strlen(line) * sizeof(char));
					short int exit = 0;

					sprintf(memline, MEMCACHE_HOST, i);
					if ((position = strstr(line, memline)) != NULL) {
						//Bind configuration
						char *v = parse_line(position);
						char *bind = (char *)malloc(strlen(v) * sizeof(char));
						strcpy(bind, v);
						conf->memcached_servers[i]->host = bind;
						conf->upcloo_memcached_server_count = i+1;
					} else {
						++exit;
					}

					sprintf(memline, MEMCACHE_PORT, i);
					if ((position = strstr(line, memline)) != NULL) {
						char *port = parse_line(position);
						int iport = atoi(port);
						conf->memcached_servers[i]->port = iport;
						conf->upcloo_memcached_server_count = i+1;
					} else {
						++exit;
					}

					//TODO: create a better method
					if (exit == 2) {
						break;
					}
				}
			}
		}

		free(line);
		fclose(file);
	}
}

/*
 * Parse a single line a return the truncate
 * pointer
 */
char *parse_line(char *position)
{
	position = strchr(position, ' ');
	return trim(position);
}
