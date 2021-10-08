/*
 * url_decoder.s
 *
 *  Created on: 2020/10/27
 *      Author: Kiyoshi TASAKI <k.tasaki@effect-effect.com>
 */

#include "url_decoder.h"
#include <ctype.h>

/*
 * Prototype declarations
 */
static __inline__ int hex_to_int(char chr);

/*
 * Check whether the character is permitted in scheme string
 */
static __inline__ int hex_to_int(char chr) {
	return chr - (chr <= '9' ? '0' : ((chr <= 'F' ? 'A' : 'a') - 10));
}

size_t decode_url(char *dst, const char *src, size_t srcLength) {
	size_t length;
	char *srcPtr = (char*) src;
	for (length = 0; srcPtr < src + srcLength && *srcPtr != '\0'; length++) {
		if (*srcPtr == '%' && srcPtr[1] != '\0' && srcPtr[2] != '\0'
				&& isxdigit(srcPtr[1]) && isxdigit(srcPtr[2])) {
			if (dst != NULL) {
				dst[length] = 16 * hex_to_int(srcPtr[1])
						+ hex_to_int(srcPtr[2]);
			}
			srcPtr += 3;
		} else {
			if (dst != NULL) {
				dst[length] = *srcPtr;
			}
			srcPtr++;
		}
	}
	if (dst != NULL) {
		dst[length] = '\0';
	}
	return length;
}
