/*
 * Mutex.cpp
 *
 *  Created on: 2016/03/31
 *      Author: PC-EFFECT-012
 */

#include "Mutex.h"
#include <unistd.h>

Mutex::Mutex(const TCHAR *name) :
		name(NameType(name)) {
	hMutex = NULL;
}

Mutex::Mutex(NameType name) :
		name(name) {
	hMutex = NULL;
}

bool Mutex::lock() {
	SECURITY_DESCRIPTOR sd;
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, 0, FALSE);

	SECURITY_ATTRIBUTES secAttribute;
	secAttribute.nLength = sizeof(secAttribute);
	secAttribute.lpSecurityDescriptor = &sd;
	secAttribute.bInheritHandle = TRUE;

	hMutex = CreateMutex(&secAttribute, FALSE, name.data());
	if (hMutex == NULL) {
		// ミューテックスの取得に失敗！
		return false;
	} else if (GetLastError() == ERROR_ALREADY_EXISTS) {
		// 既に起動しています！
		CloseHandle(hMutex);
		hMutex = NULL;
		return false;
	} else {
		// １つめの起動です！
		return true;
	}
}

void Mutex::unlock() {
	if (hMutex != NULL) {
		CloseHandle(hMutex);
		hMutex = NULL;
	}
}

void Mutex::wait() {
	int sleepTime = 100;
	while (!lock()) {
		usleep(sleepTime);
		sleepTime += 50;
		if (sleepTime > 1000) {
			sleepTime = 100;
		}
	}
}

Mutex::~Mutex(void) {
	unlock();
}
