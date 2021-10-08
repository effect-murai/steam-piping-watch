/*
 * def.h
 *
 *  Created on: 2020/12/04
 *      Author: Kiyoshi TASAKI <k.tasaki@effect-effect.com>
 */

#ifndef DEF_H_
#define DEF_H_

#ifdef __cplusplus

#include <string.h>

#include <string>

#ifdef UNICODE
#define scanString(...) (swscanf(__VA_ARGS__))
typedef std::wstring String;
#else /* !defined(UNICODE) */
#define scanString(...) (sscanf(__VA_ARGS__))
typedef std::string String;
#endif /* UNICODE */

#endif /* __cplusplus */

#endif /* DEF_H_ */
