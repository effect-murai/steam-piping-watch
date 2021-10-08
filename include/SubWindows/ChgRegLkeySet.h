/*
 * ChgRegLkeySet.h
 *
 *  Created on: 2020/11/26
 *      Author: A.Masago
 */

#ifndef INCLUDE_SUBWINDOWS_CHGREGLKEYSET_H_
#define INCLUDE_SUBWINDOWS_CHGREGLKEYSET_H_

#include <windows.h>
#include "resource.h"
#include "Dialog.h"

class ChgRegLkeySetDialog: public Dialog {
public:
	ChgRegLkeySetDialog(HWND handle);
	~ChgRegLkeySetDialog(void);

	static INT_PTR CALLBACK handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);
	static INT_PTR create(HWND parent);

};

#endif /* INCLUDE_SUBWINDOWS_CHGREGLKEYSET_H_ */
