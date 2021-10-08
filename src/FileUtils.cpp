/*
 * FileUtils.cpp
 *
 *  Created on: 2015/10/21
 *      Author: effectet
 */

#include "FileUtils.h"

void SystemTimeToStdCTimeStruct(LPSYSTEMTIME systemTime, struct tm *stdTime) {
	if ((systemTime == NULL) || (stdTime == NULL)) {
		return;
	}
	ZeroMemory(stdTime, sizeof(struct tm));
	stdTime->tm_year = systemTime->wYear - 1900;
	stdTime->tm_mon = systemTime->wMonth - 1;
	stdTime->tm_mday = systemTime->wDay;
	stdTime->tm_hour = systemTime->wHour;
	stdTime->tm_min = systemTime->wMinute;
	stdTime->tm_sec = systemTime->wSecond;
}

bool getFileTimeStd(const TCHAR *path, struct tm *creationTime,
		struct tm *lastAccessTime, struct tm *lastWriteTime) {
	FILETIME ftCreation, ftAccess, ftModified;
	if (!getFileTimeWin(path, &ftCreation, &ftAccess, &ftModified)) {
		return false;
	}

	SYSTEMTIME systemTime;
	if (creationTime != NULL) {
		FileTimeToSystemTime(&ftCreation, &systemTime);
		SystemTimeToStdCTimeStruct(&systemTime, creationTime);
	}
	if (lastAccessTime != NULL) {
		FileTimeToSystemTime(&ftAccess, &systemTime);
		SystemTimeToStdCTimeStruct(&systemTime, lastAccessTime);
	}
	if (lastWriteTime != NULL) {
		FileTimeToSystemTime(&ftModified, &systemTime);
		SystemTimeToStdCTimeStruct(&systemTime, lastWriteTime);
	}

	return true;
}

/**
 * ファイル時刻を取得する。
 * @param[in] path 時刻を取得するファイルのパス
 * @param[out] creationTime 作成日時
 * @param[out] lastAccessTime 最終アクセス日時
 * @param[out] lastWriteTime 最終更新日時
 * @return 関数の成否(true:成功/false:失敗)
 */
bool getFileTimeWin(const TCHAR *path, LPFILETIME creationTime,
		LPFILETIME lastAccessTime, LPFILETIME lastWriteTime) {
	bool ret;
	HANDLE hFile;

	hFile = CreateFile(path, 0,
	FILE_SHARE_READ | FILE_SHARE_WRITE,
	NULL,
	OPEN_EXISTING, 0,
	NULL);

	ret = GetFileTime(hFile, creationTime, lastAccessTime, lastWriteTime);

	CloseHandle(hFile);
	return ret;
}

/**
 * ファイルの一覧を取得する。
 * @param[in] path 検索対象のフォルダ
 * @param[in] pattern 検索パターン
 * @param[out] files パターンに一致したファイル名(ファイル名のみ)
 * @return 関数の成否(true:成功/false:失敗)
 */
bool searchFiles(const TCHAR *path, const TCHAR *pattern,
		std::vector<TCHAR*> &files) {
	int len;
	HANDLE hFind;
	WIN32_FIND_DATA findFileData;
	bool ret = false;
	TCHAR *fileName;

	files.clear();

	TCHAR *searchFileName = (TCHAR*) malloc(MAX_PATH * sizeof(TCHAR));
	if (searchFileName == NULL) {
		return false;
	}
	memset(searchFileName, 0, MAX_PATH * sizeof(TCHAR));
	wsprintf(searchFileName, TEXT("%s\\%s"), path, pattern);

	hFind = FindFirstFile(searchFileName, &findFileData);
	if (hFind != INVALID_HANDLE_VALUE) {
		ret = true;
		do {
			len = lstrlen(findFileData.cFileName);
			fileName = new TCHAR[len + 1];
			memset(fileName, 0, (len + 1) * sizeof(TCHAR));
			memcpy(fileName, findFileData.cFileName, len * sizeof(TCHAR));
			files.push_back(fileName);
		} while (FindNextFile(hFind, &findFileData));
		FindClose(hFind);
	}

	free(searchFileName);
	searchFileName = NULL;
	return ret;
}

/**
 * ファイルの存在を確認する。
 * @param[in] fileName 対象のファイル名/パターン
 * @return 関数の成否(true:成功/false:失敗)
 */
bool existFile(const TCHAR *fileName) {
	HANDLE hFind;
	WIN32_FIND_DATA findFileData;

	hFind = FindFirstFile(fileName, &findFileData);

	if (hFind != INVALID_HANDLE_VALUE) {
		FindClose(hFind);
		return true;
	}

	return false;
}

/**
 * searchFiles関数で作成したファイル一覧の内容を空にする。
 * @param[in] files ファイル一覧
 */
void searchFiles_cleanup(std::vector<TCHAR*> &files) {
	for (std::vector<TCHAR*>::iterator file = files.begin();
			file != files.end(); file++) {
		delete *file;
	}
	files.clear();
	std::vector<TCHAR*>().swap(files);
}

/**
 * ファイルパスをフォルダパスに変換する。
 * @param[in] path ファイルパスへのポインタ
 * @return pathと同じ
 */
TCHAR* toBaseName(TCHAR *path) {
	for (int i = lstrlen(path); i >= 0; i--) {
		if (path[i] == TEXT('\\')) {
			path[i] = TEXT('\0');
			break;
		}
	}
	return path;
}

/**
 * ファイル拡張子を取得する。
 * @param[in] path ファイルパスへのポインタ
 * @return path+拡張子の位置
 */
TCHAR* getExt(TCHAR *fileName) {
	int length = lstrlen(fileName);
	for (int i = length - 1; i >= 0; i--) {
		// 最後の.を探す
		if (fileName[i] == TEXT('.')) {
			return &fileName[i + 1];
		}
	}
	return &fileName[length];
}
