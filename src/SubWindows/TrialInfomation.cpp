/*
 * TrialInfomation
 *
 *  Created on: 2020/11/26
 *      Author: A.Masago
 */
#include "SubWindows/TrialInfomation.h"
#include "SubWindows/ChgReg.h"
#include "MainWindow.h"

INT_PTR CALLBACK TrialInfoDialog::handleEvent(HWND hwndDlg,  // ダイアログボックスのハンドル
		UINT uMsg,     // メッセージ
		WPARAM wParam, // 最初のメッセージパラメータ
		LPARAM lParam  // 2 番目のメッセージパラメータ
		) {

	switch (uMsg) {
	case WM_INITDIALOG:
		return (INT_PTR) TRUE;

	case WM_CLOSE:
		EndDialog(hwndDlg, IDCANCEL);
		break;

	case WM_COMMAND:
		UINT wmId = LOWORD(wParam);
		switch (wmId) {
		case IDOK:
			EndDialog(hwndDlg, IDOK);
			ChgRegDialog::create(hwndDlg);
			break;

		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			break;
		}
		return (INT_PTR) TRUE;
	}
	return (INT_PTR) FALSE;
}

INT_PTR TrialInfoDialog::create(HWND parent) {
	return Resource::showDialog(IDD_INFTRIAL, parent, handleEvent);
}

/**
 * 体験版情報表示ダイアログを表示する。
 */
void showTrialInfomationDialog() {
	(void) TrialInfoDialog::create(NULL);
	return;
}
