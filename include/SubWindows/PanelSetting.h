/*
 * PanelSetting.h
 *
 *  Created on: 2016/02/10
 *      Author: PC-EFFECT-002
 */

#ifndef PANELSETTING_H_
#define PANELSETTING_H_

#include "Dialog.h"

class PanelSettingDialog: public Dialog {
public:
	PanelSettingDialog(HWND handle);
	~PanelSettingDialog(void);

	static INT_PTR CALLBACK handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);
	static INT_PTR create(HWND parent);

private:
};

#endif /* PANELSETTING_H_ */
