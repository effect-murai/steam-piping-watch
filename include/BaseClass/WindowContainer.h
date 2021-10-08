/*
 * WindowContainer.h
 *
 *  Created on: 2016/01/28
 *      Author: PC-EFFECT-011
 */

#ifndef WINDOWCONTAINER_H_
#define WINDOWCONTAINER_H_

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

/**
 * ウィンドウを格納するためのインターフェース
 */
class WindowContainer {
public:
	WindowContainer(void);
	virtual ~WindowContainer(void);
	virtual HWND getHandle(void);
};

#endif /* WINDOWCONTAINER_H_ */
