/*
 * StringUtils.h
 *
 *  Created on: 2016/03/10
 *      Author: PC-EFFECT-012
 */

#ifndef STRINGUTILS_H_
#define STRINGUTILS_H_

#include <windows.h>

#ifdef _cplusplus
extern "C" {
#endif

extern int toUTF8String(const TCHAR *srcString, int srcLength, char *dstString,
		int dstLength);
extern char* toUTF8String(const TCHAR *srcString);
extern char* toOEMCodePageString(const TCHAR *srcString);
extern int fromUTF8String(const char *srcString, int srcLength,
		TCHAR *dstString, int dstLength);
extern TCHAR* fromUTF8String(const char *srcString);
extern TCHAR* fromOEMCodePageString(const char *srcString);

extern TCHAR* clone(TCHAR *src);

#ifdef _cplusplus
}
#endif

#endif /* STRINGUTILS_H_ */
