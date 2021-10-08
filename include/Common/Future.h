/*
 * Future.h
 *
 *  Created on: 2020/12/11
 *      Author: Kiyoshi TASAKI <k.tasaki@effect-effect.com>
 */

#ifndef COMMON_FUTURE_H_
#define COMMON_FUTURE_H_

#include <windows.h>

#include "Thread.h"

/**
 * Futureクラステンプレート.
 */
template<typename Type> class Future {
public:
	/**
	 * Futureクラスの処理内容.
	 */
	typedef Type (*Routine)(const void *args);

private:
	static __stdcall DWORD threadRoutine(Future<Type> *self) {
		self->set(self->proc((void*) self->args));
		return 0;
	}

public:
	/**
	 * 新しいFutureクラスのインスタンスを初期化します.
	 * @param proc 処理内容
	 * @param args 引数
	 */
	Future(Routine proc, const void *args) :
			proc(proc), args((void*) args), thread(
					(LPTHREAD_START_ROUTINE) threadRoutine, this) {
	}

	/**
	 * 処理結果を取得します.
	 */
	inline Type get() {
		thread.join();
		return value;
	}

	/**
	 * 処理結果を取得します.
	 */
	inline operator Type() {
		return value;
	}

	inline bool valid() {
		return thread.joinable();
	}

private:
	/**
	 * 処理結果を設定します.
	 */
	inline void set(Type value) {
		this->value = value;
	}

	Type value;
	/**
	 * 処理内容.
	 */
	Routine proc;
	/**
	 * 引数.
	 */
	void *args;
	Thread thread;
};

#define defineFutureRoutine(name, type, call)                                  \
	Future<type>::Routine name;                                                \
	{                                                                          \
		class {                                                                \
		  public:                                                              \
			static type proc(void *args) {                                     \
				call                                                           \
			}                                                                  \
		} name##Class;                                                         \
		name = (Future<type>::Routine)name##Class.proc;                        \
	}

#endif /* COMMON_FUTURE_H_ */
