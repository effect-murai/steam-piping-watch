/*
 * base64.h
 *
 *  Created on: 2020/11/06
 *      Author: Kiyoshi Tasaki <k.tasaki@effect-effect.com>
 */

#ifndef BASE64_H_
#define BASE64_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum tagBASE64_TYPE {
	BASE64_TYPE_STANDARD, BASE64_TYPE_MIME, BASE64_TYPE_URL
} BASE64_TYPE;

char* base64Encode(const void *data, const size_t size, const BASE64_TYPE type);
char* base64Decode(const void *base64, size_t *retSize, const BASE64_TYPE type);

#ifdef __cplusplus
}
#endif

#endif /* BASE64_H_ */
