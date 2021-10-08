/*
 * ChgRegLkeyGet.cpp
 *
 *  Created on: 2020/11/26
 *      Author: A.Masago
 */
#include "SubWindows/ChgReg.h"
#include "SubWindows/ChgRegLkeyGet.h"
#include "SubWindows/ChgRegLkeyConf.h"
#include "MainWindow.h"

INT_PTR CALLBACK ChgRegLkeyGetDialog::handleEvent(HWND hwndDlg, // ダイアログボックスのハンドル
		UINT uMsg,     // メッセージ
		WPARAM wParam, // 最初のメッセージパラメータ
		LPARAM lParam  // 2 番目のメッセージパラメータ
		) {

	switch (uMsg) {
	case WM_INITDIALOG:
		Button_SetCheck(GetDlgItem(hwndDlg, IDC_CHGREG_LKEYGET_SUBSCRIPTION),
				BST_CHECKED);
		return (INT_PTR) TRUE;

	case WM_CLOSE:
		EndDialog(hwndDlg, IDCANCEL);
		break;

	case WM_COMMAND:
		LKeyRequestParam LKeyReqParam;
		ZeroMemory(&LKeyReqParam, sizeof(LKeyRequestParam));
		UINT wmId = LOWORD(wParam);
		switch (wmId) {
		case IDOK:
			GetDlgItemText(hwndDlg, IDC_CHGREG_LKEYGET_COMPANY,
					LKeyReqParam.company, Resource::MAX_LOADSTRING);
			GetDlgItemText(hwndDlg, IDC_CHGREG_LKEYGET_DEPARTMENT,
					LKeyReqParam.department, Resource::MAX_LOADSTRING);
			GetDlgItemText(hwndDlg, IDC_CHGREG_LKEYGET_FULLNAME,
					LKeyReqParam.fullname, Resource::MAX_LOADSTRING);
			GetDlgItemText(hwndDlg, IDC_CHGREG_LKEYGET_EMAIL,
					LKeyReqParam.email, Resource::MAX_LOADSTRING);
			if (Button_GetCheck(
					GetDlgItem(hwndDlg, IDC_CHGREG_LKEYGET_SUBSCRIPTION))) {
				LKeyReqParam.contract = IDC_CHGREG_LKEYGET_SUBSCRIPTION;
			} else if (Button_GetCheck(
					GetDlgItem(hwndDlg, IDC_CHGREG_LKEYGET_PACKAGE))) {
				LKeyReqParam.contract = IDC_CHGREG_LKEYGET_PACKAGE;
			} else {
				LKeyReqParam.contract = IDC_CHGREG_LKEYGET_NO_SUPPORT;
			}
			ChgRegLkeyConfDialog::create(hwndDlg, (LPARAM) &LKeyReqParam);
			break;

		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			ChgRegDialog::create(hwndDlg);
			break;
		}
		return (INT_PTR) TRUE;
	}
	return (INT_PTR) FALSE;
}

INT_PTR ChgRegLkeyGetDialog::create(HWND parent) {
	return Resource::showDialog(IDD_CHGREG_LKEYGET, parent, handleEvent);
}
