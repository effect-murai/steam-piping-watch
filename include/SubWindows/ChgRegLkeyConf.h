/*
 * ChgRegLkeyConf.h
 *
 *  Created on: 2020/11/26
 *      Author: A.Masago
 */

#ifndef INCLUDE_SUBWINDOWS_CHGREGLKEYCONF_H_
#define INCLUDE_SUBWINDOWS_CHGREGLKEYCONF_H_

#include <windows.h>
#include "resource.h"
#include "Dialog.h"

class ChgRegLkeyConfDialog: public Dialog {
public:
	ChgRegLkeyConfDialog(HWND handle);
	~ChgRegLkeyConfDialog(void);

	static INT_PTR CALLBACK handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);
	static INT_PTR create(HWND parent, LPARAM param);
};

#endif /* INCLUDE_SUBWINDOWS_CHGREGLKEYCONF_H_ */
