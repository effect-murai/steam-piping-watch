/*
 * base64.cpp
 *
 *  Created on: 2020/11/06
 *      Author: Kiyoshi Tasaki <k.tasaki@effect-effect.com>
 */

#include "base64.h"
#include <stdlib.h>
#include <string.h>

static const char BASE64_TABLE[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char BASE64_TABLE_URL[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
static const int BASE64_TABLE_LENGTH = sizeof(BASE64_TABLE)
		/ sizeof(BASE64_TABLE[0]) - 1;

typedef struct {
	BASE64_TYPE type;
	const char *table;
	char pad;
	int maxLineLength;
	const char *lineSep;
	int lineSepLength;
} BASE64_SPEC;
static const BASE64_SPEC BASE64_SPECS[] = { { BASE64_TYPE_STANDARD,
		BASE64_TABLE, '=', 0, NULL, 0 }, { BASE64_TYPE_MIME, BASE64_TABLE, '=',
		76, "\r\n", 2 }, { BASE64_TYPE_URL, BASE64_TABLE_URL, 0, 0, NULL, 0 } };
static const size_t BASE64_SPECS_LENGTH = sizeof(BASE64_SPECS)
		/ sizeof(BASE64_SPECS[0]);

char* base64Encode(const void *data, const size_t size,
		const BASE64_TYPE type) {
	BASE64_SPEC spec;
	size_t length;
	char *base64;
	char *cursor;
	int lineLength;
	const char *dataBytes = (const char*) data;

	if (dataBytes == NULL) {
		return NULL;
	}

	spec = BASE64_SPECS[0];
	for (size_t i = 0; i < BASE64_SPECS_LENGTH; i++) {
		if (BASE64_SPECS[i].type == type) {
			spec = BASE64_SPECS[i];
			break;
		}
	}

	length = size * 4 / 3 + 3 + 1;
	if (spec.maxLineLength > 0) {
		length += size / spec.maxLineLength * spec.lineSepLength;
	}
	base64 = (char*) malloc(length);
	if (base64 == NULL) {
		return NULL;
	}

	cursor = base64;
	lineLength = 0;
	for (int i = 0, j = size; j > 0; i += 3, j -= 3) {
		if (spec.maxLineLength > 0) {
			if (lineLength >= spec.maxLineLength) {
				for (const char *sep = spec.lineSep; *sep != 0; sep++) {
					*(cursor++) = *sep;
				}
				lineLength = 0;
			}
			lineLength += 4;
		}

		if (j == 1) {
			*(cursor++) = spec.table[(dataBytes[i + 0] >> 2 & 0x3f)];
			*(cursor++) = spec.table[(dataBytes[i + 0] << 4 & 0x30)];
			*(cursor++) = spec.pad;
			*(cursor++) = spec.pad;
		} else if (j == 2) {
			*(cursor++) = spec.table[(dataBytes[i + 0] >> 2 & 0x3f)];
			*(cursor++) = spec.table[(dataBytes[i + 0] << 4 & 0x30)
					| (dataBytes[i + 1] >> 4 & 0x0f)];
			*(cursor++) = spec.table[(dataBytes[i + 1] << 2 & 0x3c)];
			*(cursor++) = spec.pad;
		} else {
			*(cursor++) = spec.table[(dataBytes[i + 0] >> 2 & 0x3f)];
			*(cursor++) = spec.table[(dataBytes[i + 0] << 4 & 0x30)
					| (dataBytes[i + 1] >> 4 & 0x0f)];
			*(cursor++) = spec.table[(dataBytes[i + 1] << 2 & 0x3c)
					| (dataBytes[i + 2] >> 6 & 0x03)];
			*(cursor++) = spec.table[(dataBytes[i + 2] << 0 & 0x3f)];
		}
	}
	*cursor = 0;

	return base64;
}

char* base64Decode(const void *base64, size_t *retSize,
		const BASE64_TYPE type) {
	BASE64_SPEC spec;
	char table[0x80];
	size_t length;
	char *data;
	char *cursor;
	const char *base64Bytes = (const char*) base64;

	if (base64Bytes == NULL) {
		return NULL;
	}

	spec = BASE64_SPECS[0];
	for (size_t i = 0; i < BASE64_SPECS_LENGTH; i++) {
		if (BASE64_SPECS[i].type == type) {
			spec = BASE64_SPECS[i];
			break;
		}
	}

	length = strlen(base64Bytes);
	data = (char*) malloc(length * 3 / 4 + 2 + 1);
	if (data == NULL) {
		return NULL;
	}

	memset(table, 0x80, sizeof(table));
	for (size_t i = 0; i < BASE64_TABLE_LENGTH; i++) {
		table[spec.table[i] & 0x7f] = i;
	}

	cursor = data;
	for (size_t i = 0, j = 0; i < length; i++, j = i % 4) {
		char ch;

		if (base64Bytes[i] == spec.pad) {
			break;
		}

		ch = table[base64Bytes[i] & 0x7f];
		if (ch & 0x80) {
			continue;
		}
		if (j == 0) {
			*cursor = ch << 2 & 0xfc;
		} else if (j == 1) {
			*(cursor++) |= ch >> 4 & 0x03;
			*cursor = ch << 4 & 0xf0;
		} else if (j == 2) {
			*(cursor++) |= ch >> 2 & 0x0f;
			*cursor = ch << 6 & 0xc0;
		} else {
			*(cursor++) |= ch & 0x3f;
		}
	}
	*cursor = 0;
	*retSize = cursor - data;

	return data;
}
