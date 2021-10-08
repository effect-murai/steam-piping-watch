/*
 * Thread.cpp
 *
 *  Created on: 2020/12/08
 *      Author: k.tasaki
 */

#include <Common/Thread.h>

#include <stddef.h>

Thread::Thread(LPTHREAD_START_ROUTINE proc, void *parameters) {
	handle = CreateThread(NULL, 0, proc, parameters, 0, NULL);
}

Thread::~Thread() {
	CloseHandle(handle);
}

void Thread::join() {
	WaitForSingleObject(handle, INFINITE);
}

bool Thread::joinable() {
	return WaitForSingleObject(handle, 0) == WAIT_OBJECT_0;
}
