/*
 * upcloo-conf.h
 *
 *  Created on: Mar 28, 2012
 *      Author: walterdalmut
 */

#ifndef UPCLOO_CONF_H_
#define UPCLOO_CONF_H_

#include "upcloo-autocomplete.h"

#define LINE_LEN 1024
#define PORT "port"
#define BIND "bind"
#define MEMCACHE_HOST "memcached.%d.host"
#define MEMCACHE_PORT "memcached.%d.port"

/*
 * Read a configuration from file
 */
void read_conf_from_file(char *, upcloo_conf *);
char *trim (char *);

char *parse_line(char *);

#endif /* UPCLOO_CONF_H_ */
