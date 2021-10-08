/*
 * sha1.cpp
 *
 *  Created on: 2020/10/23
 *      Author: Kiyoshi Tasaki <k.tasaki@effect-effect.com>
 */

#include <string.h>

#include "sha1.h"

inline unsigned long int getDword(const void *b, int i);
inline void putDword(unsigned long int n, void *b, int i);
inline unsigned long int rotateLeft(unsigned long int x, int n);

void sha1Init(Sha1Context *ctx) {
	ctx->total[0] = 0;
	ctx->total[1] = 0;

	ctx->state[0] = 0x67452301;
	ctx->state[1] = 0xEFCDAB89;
	ctx->state[2] = 0x98BADCFE;
	ctx->state[3] = 0x10325476;
	ctx->state[4] = 0xC3D2E1F0;
}

static void sha1Process(Sha1Context *ctx, const void *data) {
	unsigned long int temp, w[16], a, b, c, d, e;

	for (int i = 0; i < 16; i++) {
		w[i] = getDword(data, i * 4);
	}

#define R(t)                                                                   \
    (temp = w[(t - 3) & 0x0F] ^ w[(t - 8) & 0x0F] ^ w[(t - 14) & 0x0F] ^       \
            w[t & 0x0F],                                                       \
     (w[t & 0x0F] = rotateLeft(temp, 1)))

#define P(a, b, c, d, e, x)                                                    \
    {                                                                          \
        e += rotateLeft(a, 5) + F(b, c, d) + K + x;                            \
        b = rotateLeft(b, 30);                                                 \
    }

	a = ctx->state[0];
	b = ctx->state[1];
	c = ctx->state[2];
	d = ctx->state[3];
	e = ctx->state[4];

#define F(x, y, z) (z ^ (x & (y ^ z)))
#define K 0x5A827999

	P(a, b, c, d, e, w[0]);
	P(e, a, b, c, d, w[1]);
	P(d, e, a, b, c, w[2]);
	P(c, d, e, a, b, w[3]);
	P(b, c, d, e, a, w[4]);
	P(a, b, c, d, e, w[5]);
	P(e, a, b, c, d, w[6]);
	P(d, e, a, b, c, w[7]);
	P(c, d, e, a, b, w[8]);
	P(b, c, d, e, a, w[9]);
	P(a, b, c, d, e, w[10]);
	P(e, a, b, c, d, w[11]);
	P(d, e, a, b, c, w[12]);
	P(c, d, e, a, b, w[13]);
	P(b, c, d, e, a, w[14]);
	P(a, b, c, d, e, w[15]);
	P(e, a, b, c, d, R(16));
	P(d, e, a, b, c, R(17));
	P(c, d, e, a, b, R(18));
	P(b, c, d, e, a, R(19));

#undef K
#undef F

#define F(x, y, z) (x ^ y ^ z)
#define K 0x6ED9EBA1

	P(a, b, c, d, e, R(20));
	P(e, a, b, c, d, R(21));
	P(d, e, a, b, c, R(22));
	P(c, d, e, a, b, R(23));
	P(b, c, d, e, a, R(24));
	P(a, b, c, d, e, R(25));
	P(e, a, b, c, d, R(26));
	P(d, e, a, b, c, R(27));
	P(c, d, e, a, b, R(28));
	P(b, c, d, e, a, R(29));
	P(a, b, c, d, e, R(30));
	P(e, a, b, c, d, R(31));
	P(d, e, a, b, c, R(32));
	P(c, d, e, a, b, R(33));
	P(b, c, d, e, a, R(34));
	P(a, b, c, d, e, R(35));
	P(e, a, b, c, d, R(36));
	P(d, e, a, b, c, R(37));
	P(c, d, e, a, b, R(38));
	P(b, c, d, e, a, R(39));

#undef K
#undef F

#define F(x, y, z) ((x & y) | (z & (x | y)))
#define K 0x8F1BBCDC

	P(a, b, c, d, e, R(40));
	P(e, a, b, c, d, R(41));
	P(d, e, a, b, c, R(42));
	P(c, d, e, a, b, R(43));
	P(b, c, d, e, a, R(44));
	P(a, b, c, d, e, R(45));
	P(e, a, b, c, d, R(46));
	P(d, e, a, b, c, R(47));
	P(c, d, e, a, b, R(48));
	P(b, c, d, e, a, R(49));
	P(a, b, c, d, e, R(50));
	P(e, a, b, c, d, R(51));
	P(d, e, a, b, c, R(52));
	P(c, d, e, a, b, R(53));
	P(b, c, d, e, a, R(54));
	P(a, b, c, d, e, R(55));
	P(e, a, b, c, d, R(56));
	P(d, e, a, b, c, R(57));
	P(c, d, e, a, b, R(58));
	P(b, c, d, e, a, R(59));

#undef K
#undef F

#define F(x, y, z) (x ^ y ^ z)
#define K 0xCA62C1D6

	P(a, b, c, d, e, R(60));
	P(e, a, b, c, d, R(61));
	P(d, e, a, b, c, R(62));
	P(c, d, e, a, b, R(63));
	P(b, c, d, e, a, R(64));
	P(a, b, c, d, e, R(65));
	P(e, a, b, c, d, R(66));
	P(d, e, a, b, c, R(67));
	P(c, d, e, a, b, R(68));
	P(b, c, d, e, a, R(69));
	P(a, b, c, d, e, R(70));
	P(e, a, b, c, d, R(71));
	P(d, e, a, b, c, R(72));
	P(c, d, e, a, b, R(73));
	P(b, c, d, e, a, R(74));
	P(a, b, c, d, e, R(75));
	P(e, a, b, c, d, R(76));
	P(d, e, a, b, c, R(77));
	P(c, d, e, a, b, R(78));
	P(b, c, d, e, a, R(79));

#undef K
#undef F

	ctx->state[0] += a;
	ctx->state[1] += b;
	ctx->state[2] += c;
	ctx->state[3] += d;
	ctx->state[4] += e;
}

void sha1Update(Sha1Context *ctx, const void *input, unsigned long int length) {
	unsigned long int left, fill;
	const char *inputBytes = (const char*) input;

	if (!length) {
		return;
	}

	left = ctx->total[0] & 0x3F;
	fill = 64 - left;

	ctx->total[0] += length;
	ctx->total[0] &= 0xFFFFFFFF;

	if (ctx->total[0] < length)
		ctx->total[1]++;

	if (left && length >= fill) {
		memcpy((ctx->buffer + left), input, fill);
		sha1Process(ctx, ctx->buffer);
		length -= fill;
		inputBytes += fill;
		left = 0;
	}

	while (length >= 64) {
		sha1Process(ctx, input);
		length -= 64;
		inputBytes += 64;
	}

	if (length) {
		memcpy((void*) (ctx->buffer + left), input, length);
	}
}

static const unsigned char SHA1_PADDING[64] = { 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0 };

void sha1Final(Sha1Context *ctx, unsigned char *digest) {
	unsigned long int last, padding;
	unsigned long int high, low;
	unsigned char msglen[8];

	high = (ctx->total[0] >> 29) | (ctx->total[1] << 3);
	low = (ctx->total[0] << 3);

	putDword(high, msglen, 0);
	putDword(low, msglen, 4);

	last = ctx->total[0] & 0x3F;
	padding = (last < 56) ? (56 - last) : (120 - last);

	sha1Update(ctx, SHA1_PADDING, padding);
	sha1Update(ctx, msglen, 8);

	putDword(ctx->state[0], digest, 0);
	putDword(ctx->state[1], digest, 4);
	putDword(ctx->state[2], digest, 8);
	putDword(ctx->state[3], digest, 12);
	putDword(ctx->state[4], digest, 16);
}

unsigned long int getDword(const void *b, int i) {
	const unsigned char *p = (const unsigned char*) b + i;
	return ((unsigned long int) p[0] << 24) | ((unsigned long int) p[1] << 16)
			| ((unsigned long int) p[2] << 8) | ((unsigned long int) p[3]);
}

void putDword(unsigned long int n, void *b, int i) {
	unsigned char *p = (unsigned char*) b + i;
	p[0] = (unsigned char) (n >> 24);
	p[1] = (unsigned char) (n >> 16);
	p[2] = (unsigned char) (n >> 8);
	p[3] = (unsigned char) (n);
}

unsigned long int rotateLeft(unsigned long int x, int n) {
	return ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)));
}
