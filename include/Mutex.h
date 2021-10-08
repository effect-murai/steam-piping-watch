/*
 * Mutex.h
 *
 *  Created on: 2019/09/12
 *      Author: PC-EFFECT-006
 */

#ifndef MUTEX_H_
#define MUTEX_H_

#include <windows.h>
#include <string>

/**
 * ミューテックス
 */
class Mutex {
private:
#	ifdef UNICODE
	typedef std::wstring NameType;
#	else
	typedef std::string NameType;
#	endif
public:
	Mutex(const TCHAR *name);
	Mutex(NameType name);

	~Mutex(void);

	bool lock();
	void unlock();

	void wait();

private:
	/**
	 * ミューテックスオブジェクト名.
	 */
	NameType name;
	/**
	 * ミューテックスオブジェクトへのハンドル.
	 */
	HANDLE hMutex;
};

#endif

