/*
 * app.h
 *
 *  Created on: 2015/12/08
 *      Author: PC-EFFECT-012
 */

#ifndef APP_H_
#define APP_H_

//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------
#ifdef UNICODE

#define toInt _wtoi
#define toDouble _wtof
#define fileOpen _wfopen
#define pipeOpen _wpopen

#define ftprintf fwprintf
#define stprintf _swprintf
#define sntprintf snwprintf
#define tprintf  wprintf
#define splitpath  _wsplitpath_s
#else

#define toInt atoi
#define toDouble atof
#define fileOpen fopen
#define pipeOpen popen

#define ftprintf fprintf
#define stprintf sprintf
#define sntprintf snprintf
#define tprintf printf
#define splitpath  _splitpath_s
#endif // UNICODE

#endif /* APP_H_ */
