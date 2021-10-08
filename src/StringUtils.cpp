/*
 * StringConv.cpp
 *
 *  Created on: 2016/01/27
 *      Author: PC-EFFECT-012
 */

#include "StringUtils.h"

/**
 * 内部コード文字列をUTF-8文字列へ変換する
 * @param srcString 変換前の内部コード文字列
 * @param srcLength 変換前の内部コード文字列の長さ
 * @param dstString 変換後のUTF-8文字列を格納する文字列バッファ
 * @param dstLength 変換後のUTF-8文字列を格納する文字列バッファの長さ
 * @param 変換後の文字列の長さ
 */
int toUTF8String(const TCHAR *srcString, int srcLength, char *dstString,
		int dstLength) {
	wchar_t *wcString;
#ifdef UNICODE
	wcString = (wchar_t*) srcString;
#else
	wcString= new TCHAR[srcLength];
	ZeroMemory(wcString, srcLength * sizeof(TCHAR));
	MultiByteToWideChar(CP_OEMCP, 0, srcString, srcLength, wcString, srcLength);
#endif
	WideCharToMultiByte(CP_UTF8, 0, wcString, srcLength, dstString, dstLength,
	NULL, NULL);
#ifndef UNICODE
	delete wcString;
#endif
	return dstLength;
}

/**
 * 内部コード文字列をUTF-8文字列へ変換する
 * @param srcString 変換前の内部コード文字列
 * @param 変換後のUTF-8文字列
 */
char* toUTF8String(const TCHAR *srcString) {
	const int srcLen = lstrlen(srcString);
	const int dstLen = WideCharToMultiByte(
	CP_UTF8, 0, srcString, srcLen, NULL, 0, NULL, NULL);
	char *dstString = new char[dstLen + 1];
	ZeroMemory(dstString, (dstLen + 1) * sizeof(char));
	toUTF8String(srcString, srcLen, dstString, dstLen + 1);
	return dstString;
}

/**
 * 内部コード文字列をUTF-8文字列へ変換する
 * @param srcString 変換前の内部コード文字列
 * @param 変換後のUTF-8文字列
 */
char* toOEMCodePageString(const TCHAR *srcString) {
	BOOL used = false;
	const int srcLength = lstrlen(srcString);
	const int dstLength = WideCharToMultiByte(CP_OEMCP, 0, srcString, srcLength,
	NULL, 0, NULL, &used);
	if (used) {
		return NULL;
	}
	char *dstString = new char[dstLength + 1];
	ZeroMemory(dstString, dstLength + 1);
	WideCharToMultiByte(CP_OEMCP, 0, srcString, srcLength, dstString, dstLength,
	NULL, NULL);
	return dstString;
}

/**
 * UTF-8文字列から内部コード文字列に変換する
 * @param srcString 変換前のUTF-8文字列
 * @param srcLength 変換前のUTF-8文字列の長さ
 * @param dstString 変換後の内部コード文字列を格納する文字列バッファ
 * @param dstLength 変換後の内部コード文字列を格納する文字列バッファの長さ
 * @param 変換後の文字列の長さ
 */
int fromUTF8String(const char *srcString, int srcLength, TCHAR *dstString,
		int dstLength) {
	wchar_t *wcString;
	wcString = new wchar_t[srcLength];
	ZeroMemory(wcString, srcLength * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, srcString, srcLength, wcString, dstLength);
#ifdef UNICODE
	memcpy(dstString, wcString, (dstLength - 1) * sizeof(TCHAR));
	dstString[dstLength - 1] = 0;
#else
	WideCharToMultiByte(CP_OEMCP, 0, wcString, srcLength, dstString, dstLength, NULL, NULL);
#endif
	delete wcString;
	return dstLength;
}

/**
 * UTF-8文字列を内部コード文字列へ変換する
 * @param srcString 変換前のUTF-8文字列
 * @param 変換後の内部コード文字列
 */
TCHAR* fromUTF8String(const char *srcString) {
	const int wcLength = MultiByteToWideChar(
	CP_UTF8, 0, srcString, -1, NULL, 0) + 1;
	wchar_t *wcString;
	wcString = new wchar_t[wcLength];
	ZeroMemory(wcString, wcLength * sizeof(wchar_t));
	MultiByteToWideChar(
	CP_UTF8, 0, srcString, -1, wcString, wcLength);
#ifdef UNICODE
	return wcString;
#else
	const int mbLength = WideCharToMultiByte(
			CP_UTF8, 0, wcString, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_OEMCP, 0,
			wcString, wcLength, mbString, mbLength, NULL, NULL);
	delete wcString;
	return mbString;
#endif
}

/**
 * UTF-8文字列を内部コード文字列へ変換する
 * @param srcString 変換前のUTF-8文字列
 * @param 変換後の内部コード文字列
 */
TCHAR* fromOEMCodePageString(const char *srcString) {
#ifdef UNICODE
	const int wcLength = MultiByteToWideChar(
	CP_OEMCP, 0, srcString, -1, NULL, 0) + 1;
	wchar_t *wcString;
	wcString = new wchar_t[wcLength];
	ZeroMemory(wcString, wcLength * sizeof(wchar_t));
	MultiByteToWideChar(
	CP_OEMCP, 0, srcString, -1, wcString, wcLength);
	return wcString;
#else
	return clone(srcString);
#endif
}

TCHAR* clone(TCHAR *src) {
	int len = lstrlen(src);
	TCHAR *dst = new TCHAR[len + 1];
	ZeroMemory(dst, (len + 1) * sizeof(TCHAR));
	lstrcpy(dst, src);
	return dst;
}
