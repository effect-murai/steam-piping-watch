/*
 * sha1.h
 *
 *  Created on: 2020/10/23
 *      Author: Kiyoshi Tasaki <k.tasaki@effect-effect.com>
 */

#ifndef SHA1_H_
#define SHA1_H_

typedef struct {
	unsigned long int total[2];
	unsigned long int state[5];
	unsigned char buffer[64];
} Sha1Context;

#ifdef __cplusplus
extern "C" {
#endif

void sha1Init(Sha1Context *ctx);
void sha1Update(Sha1Context *ctx, const void *input, unsigned long int length);
void sha1Final(Sha1Context *ctx, unsigned char *digest);

#ifdef __cplusplus
}
#endif

#endif /* SHA1_H_ */
