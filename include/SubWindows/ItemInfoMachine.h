/*
 * ItemInfoMachine.h
 *
 *  Created on: 2016/03/30
 *      Author: PC-EFFECT-002
 */

#ifndef ITEMINFOMACHINE_H_
#define ITEMINFOMACHINE_H_

#include "Dialog.h"

class ItemInfoMachineDialog: public Dialog {
public:
	ItemInfoMachineDialog(HWND handle);
	~ItemInfoMachineDialog();

	static INT_PTR CALLBACK handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);
	static int create(HWND parent);

private:

};

#endif /* ITEMINFOMACHINE_H_ */

