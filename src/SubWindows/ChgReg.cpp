/*
 * ChgReg.cpp
 *
 *  Created on: 2020/11/26
 *      Author: A.Masago
 */
#include "SubWindows/ChgReg.h"
#include "SubWindows/ChgRegLkeySet.h"
#include "SubWindows/ChgRegLkeyGet.h"
#include "MainWindow.h"
#include "Protection.h"

INT_PTR CALLBACK ChgRegDialog::handleEvent(HWND hwndDlg,  // ダイアログボックスのハンドル
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
			if (!isAdministrator()) {
				Resource::showMessageBox(hwndDlg, IDS_CHGREG_LKEYSET_MESSAGE1,
				IDS_CHGREG_TITLE_ERROR, MB_ICONERROR);
				EndDialog(hwndDlg, IDOK);
				PostQuitMessage(0);
			} else {
				EndDialog(hwndDlg, IDOK);
				(void) ChgRegLkeySetDialog::create(hwndDlg);
			}
			break;

		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			ChgRegLkeyGetDialog::create(hwndDlg);
			break;
		}
		return (INT_PTR) TRUE;
	}
	return (INT_PTR) FALSE;
}

INT_PTR ChgRegDialog::create(HWND parent) {
	return Resource::showDialog(IDD_CHGREG, parent, handleEvent);
}
