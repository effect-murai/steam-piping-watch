/*
 * ItemInfomation.h
 *
 *  Created on: 2016/03/30
 *      Author: PC-EFFECT-002
 */

#ifndef ITEMINFOMATION_H_
#define ITEMINFOMATION_H_

#include "Dialog.h"

class ItemInfomationDialog: public Dialog {
public:
	ItemInfomationDialog(HWND handle);
	~ItemInfomationDialog();

	static INT_PTR CALLBACK handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);
	static INT_PTR create(HWND parent);
	static bool itemOutput(HWND hwnd);
	static bool setItem(HWND hwnd);
private:

};

#endif /* ITEMINFOMATION_H_ */

