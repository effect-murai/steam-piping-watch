/*
 * ProgressDialog.cpp
 *
 *  Created on: 2020/12/15
 *      Author: k.tasaki
 */

#include <SubWindows/ProgressDialog.h>

ProgressDialog::ProgressDialog(HWND handle) :
		Dialog(handle) {
	result = NULL;
}

INT_PTR ProgressDialog::onInitialize(WPARAM wParam, LPARAM lParam) {
	result = (Future<void*>*) lParam;
	HWND hProgress = getDlgItem(IDC_PROGRESS);
	SetWindowLongPtr(hProgress, GWL_STYLE,
	PBS_MARQUEE | GetWindowLongPtr(hProgress, GWL_STYLE));
	SendMessage(getDlgItem(IDC_PROGRESS), PBM_SETMARQUEE, (WPARAM) TRUE,
			(LPARAM) 16);
	setText(TEXT("お待ちください"));
	SetTimer(this->getHandle(), ID_UPDATE_TIMER, UPDATE_TIMER_INTERVAL,
	NULL);
	return (INT_PTR) TRUE;
}

INT_PTR ProgressDialog::onClose(WPARAM wParam, LPARAM lParam) {
	KillTimer(this->getHandle(), ID_UPDATE_TIMER);
	return (INT_PTR) TRUE;
}

INT_PTR CALLBACK ProgressDialog::handleEvent(UINT uMsg, WPARAM wParam,
		LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		return onInitialize(wParam, lParam);
	case WM_DESTROY:
		break;
	case WM_CLOSE:
		return (INT_PTR) TRUE;
	case WM_COMMAND:
		break;
	case WM_TIMER:
		if (result == NULL || result->valid()) {
			close(0);
		}
		return (INT_PTR) TRUE;
	}
	return (INT_PTR) FALSE;
}

INT_PTR CALLBACK ProgressDialog::handleEvent(HWND hwndDlg, UINT uMsg,
		WPARAM wParam, LPARAM lParam) {
	ProgressDialog *target = (ProgressDialog*) fromHandle(hwndDlg);
	if (target == NULL) {
		if (uMsg == WM_INITDIALOG) {
			target = new ProgressDialog(hwndDlg);
		} else {
			return (INT_PTR) FALSE;
		}
	}
	return target->handleEvent(uMsg, wParam, lParam);
}
