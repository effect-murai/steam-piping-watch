/*
 * TrialInfomation.h
 *
 *  Created on: 2020/11/26
 *      Author: A.Masago
 */

#ifndef INCLUDE_SUBWINDOWS_TRIALINFOMATION_H_
#define INCLUDE_SUBWINDOWS_TRIALINFOMATION_H_

#include <windows.h>
#include "resource.h"
#include "Dialog.h"

extern void showTrialInfomationDialog();

class TrialInfoDialog: public Dialog {
public:
	TrialInfoDialog(HWND handle);
	~TrialInfoDialog(void);

	static INT_PTR CALLBACK handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);
	static INT_PTR create(HWND parent);

};

#endif /* INCLUDE_SUBWINDOWS_TRIALINFOMATION_H_ */
