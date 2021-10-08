/*
 * DetectSetting.h
 *
 *  Created on: 2016/02/01
 *      Author: PC-EFFECT-012
 */

#ifndef DETECTSETTING_H_
#define DETECTSETTING_H_

#include "Dialog.h"

class DetectSettingDialog: public Dialog {
public:
	DetectSettingDialog(double);
	~DetectSettingDialog();
	static INT_PTR CALLBACK handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);
	static INT_PTR create(HWND parent);

	static DetectSettingDialog* getInstance(void);

private:
	DetectSettingDialog(HWND handle);

	INT_PTR initDialog();

};

#endif /* DETECTSETTING_H_ */
