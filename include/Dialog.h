/*
 * Dialog.h
 *
 *  Created on: 2016/01/29
 *      Author: PC-EFFECT-012
 */
#include <windows.h>

#ifndef DIALOG_H_
#define DIALOG_H_

#include "Window.h"
#include "WindowContainer.h"

//------------------------------------------------------------------------------
// Class Definitions
//------------------------------------------------------------------------------
class Dialog: public Window, public WindowContainer {
public:
	HWND getHandle(void);
	void close(INT_PTR result);
protected:
	Dialog(HWND handle);
	virtual ~Dialog(void);
	HWND getDlgItem(int itemId);

	static HWND create(Window *parent, DLGPROC eventHandler, LPARAM param);

	static void setDialogItemTextAsInt(HWND dialog, int itemId, int value);
	static int getDialogItemTextAsInt(HWND dialog, int itemId);
};

#endif /* DIALOG_H_ */
