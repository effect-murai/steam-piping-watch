/*
 * debug.h
 *
 *  Created on: 2020/05/07
 *      Author: Kiyoshi TASAKI <k.tasaki@effect-effect.com>
 */

#ifndef EFFECT_LIBC_DEBUG_H_
#define EFFECT_LIBC_DEBUG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#ifndef _WIN32
#include <sys/timeb.h>
#endif

#ifdef _DEBUG
#ifndef SHOW_DEBUG_MESSAGE
#define SHOW_DEBUG_MESSAGE
#endif
#endif

/**
 * デバッグメッセージを表示する.
 */
#ifdef _WIN32
#define printLog(stream, ...)                                                                      \
  {                                                                                                \
    char fileName[sizeof(__FILE__) + 1];                                                           \
    char ext[sizeof(__FILE__) + 1];                                                                \
    struct __timeb64 now;                                                                          \
    struct tm utc;                                                                                 \
    char dateTimeString[24];                                                                       \
    _splitpath_s(__FILE__, NULL, 0, NULL, 0, fileName, sizeof(__FILE__), ext, sizeof(__FILE__));   \
    _ftime64_s(&now);                                                                              \
    _gmtime64_s(&utc, &now.time);                                                                  \
    strftime(dateTimeString, 24, "%Y-%m-%dT%H:%M:%S", &utc);                                       \
    fprintf(stream, "%s.%03dZ %s%s:%s:%d ", dateTimeString, now.millitm, fileName, ext, __func__,  \
        __LINE__);                                                                                 \
    fprintf(stream, __VA_ARGS__);                                                                  \
    fprintf(stream, "\n");                                                                         \
    fflush(stream);                                                                                \
  }
#else
#define printLog(stream, ...)                                                                      \
  {                                                                                                \
    struct timeb now;                                                                              \
    struct tm utc;                                                                                 \
    char dateTimeString[24];                                                                       \
    ftime(&now);                                                                                   \
    gmtime_r(&now.time, &utc);                                                                     \
    strftime(dateTimeString, 24, "%Y-%m-%dT%H:%M:%S", &utc);                                       \
    fprintf(                                                                                       \
        stream, "%s.%03dZ %s:%s:%d ", dateTimeString, now.millitm, __FILE__, __func__, __LINE__);  \
    fprintf(stream, __VA_ARGS__);                                                                  \
    fprintf(stream, "\n");                                                                         \
    fflush(stream);                                                                                \
  }
#endif

#ifdef SHOW_DEBUG_MESSAGE
#define error(...) printLog(stderr, __VA_ARGS__)
#define debug(...) printLog(stdout, __VA_ARGS__)
#else
#define error(...) printLog(stderr, __VA_ARGS__)
#define debug(...)
#endif

#endif /* EFFECT_LIBC_DEBUG_H_ */
