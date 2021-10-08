/*
 * Thread.h
 *
 *  Created on: 2020/12/08
 *      Author: k.tasaki
 */

#ifndef COMMON_THREAD_H_
#define COMMON_THREAD_H_

#include <windows.h>

#define defineThreadStartRoutine(name, type, call)                             \
	LPTHREAD_START_ROUTINE name;                                               \
	{                                                                          \
		class {                                                                \
		  public:                                                              \
			static DWORD __stdcall proc(void *args) {                          \
				return subproc((type *)args);                                  \
			}                                                                  \
			static DWORD subproc(type *args) {                                 \
				call                                                           \
			}                                                                  \
		} name##Class;                                                         \
		name = name##Class.proc;                                               \
	}

class Thread {
public:
	Thread(LPTHREAD_START_ROUTINE proc, void *parameters);
	virtual ~Thread();

	void join();

	bool joinable();

private:
	HANDLE handle;
};

#endif /* COMMON_THREAD_H_ */
