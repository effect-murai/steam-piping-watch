/*
 * url_parser.h
 *
 *  Created on: 2020/10/27
 *      Author: Kiyoshi TASAKI <k.tasaki@effect-effect.com>
 */

#ifndef _URL_PARSER_H
#define _URL_PARSER_H

/*
 * URL storage
 */
struct parsed_url {
	char *scheme; /* mandatory */
	char *host; /* mandatory */
	char *port; /* optional */
	char *path; /* optional */
	char *query; /* optional */
	char *fragment; /* optional */
	char *username; /* optional */
	char *password; /* optional */
};

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Declaration of function prototypes
 */
struct parsed_url* parse_url(const char*);
void parsed_url_free(struct parsed_url*);

#ifdef __cplusplus
}
#endif

#endif /* _URL_PARSER_H */
