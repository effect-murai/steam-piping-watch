/*
 * url_decoder.h
 *
 *  Created on: 2020/10/27
 *      Author: Kiyoshi TASAKI <k.tasaki@effect-effect.com>
 */

#ifndef _URL_DECODER_H
#define _URL_DECODER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Declaration of function prototypes
 */
size_t decode_url(char*, const char*, size_t);

#ifdef __cplusplus
}
#endif

#endif /* _URL_DECODER_H */
