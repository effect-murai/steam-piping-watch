/*
 * FileUtils.h
 *
 *  Created on: 2015/10/21
 *      Author: effectet
 */

#ifndef FILEUTILS_H_
#define FILEUTILS_H_

#include <time.h>
#include <windows.h>
#include <vector>

extern bool getFileTimeStd(const TCHAR *path, struct tm *creationTime,
		struct tm *lastAccessTime, struct tm *lastWriteTime);
extern bool getFileTimeWin(const TCHAR *path, LPFILETIME creationTime,
		LPFILETIME lastAccessTime, LPFILETIME lastWriteTime);
extern bool searchFiles(const TCHAR *path, const TCHAR *pattern,
		std::vector<TCHAR*> &files);
extern bool existFile(const TCHAR *fileName);
extern void searchFiles_cleanup(std::vector<TCHAR*> &files);
extern TCHAR* toBaseName(TCHAR *path);
extern TCHAR* getExt(TCHAR *fileName);

#endif /* FILEUTILS_H_ */
