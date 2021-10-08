/*
 * ChgRegLkeyConf.cpp
 *
 *  Created on: 2020/11/26
 *      Author: A.Masago
 */
#include "SubWindows/ChgReg.h"
#include "SubWindows/ChgRegLkeyGet.h"
#include "SubWindows/ChgRegLkeyConf.h"
#include "MainWindow.h"
#include "LicenseManager.h"
#include <SubWindows/ProgressDialog.h>
#include "Protection.h"

INT_PTR CALLBACK ChgRegLkeyConfDialog::handleEvent(HWND hwndDlg, // ダイアログボックスのハンドル
		UINT uMsg,     // メッセージ
		WPARAM wParam, // 最初のメッセージパラメータ
		LPARAM lParam  // 2 番目のメッセージパラメータ
		) {

	switch (uMsg) {
	case WM_INITDIALOG:
		ChgRegLkeyGetDialog::LKeyRequestParam *pLKeyReqParam;
		pLKeyReqParam = (ChgRegLkeyGetDialog::LKeyRequestParam*) lParam;

		SetDlgItemText(hwndDlg, IDC_CHGREG_LKEYCONF_COMPANY,
				pLKeyReqParam->company);
		SetDlgItemText(hwndDlg, IDC_CHGREG_LKEYCONF_DEPARTMENT,
				pLKeyReqParam->department);
		SetDlgItemText(hwndDlg, IDC_CHGREG_LKEYCONF_FULLNAME,
				pLKeyReqParam->fullname);
		SetDlgItemText(hwndDlg, IDC_CHGREG_LKEYCONF_EMAIL,
				pLKeyReqParam->email);
		Button_SetCheck(GetDlgItem(hwndDlg, pLKeyReqParam->contract),
				BST_CHECKED);
		return (INT_PTR) TRUE;

	case WM_CLOSE:
		EndDialog(hwndDlg, IDCANCEL);
		break;

	case WM_COMMAND:
		UINT wmId = LOWORD(wParam);
		switch (wmId) {
		case IDOK: {
			// ダイアログからユーザー登録データを取得
			ChgRegLkeyGetDialog::LKeyRequestParam registData;
			GetDlgItemText(hwndDlg, IDC_CHGREG_LKEYCONF_COMPANY,
					registData.company, Resource::MAX_LOADSTRING);
			GetDlgItemText(hwndDlg, IDC_CHGREG_LKEYCONF_DEPARTMENT,
					registData.department, Resource::MAX_LOADSTRING);
			GetDlgItemText(hwndDlg, IDC_CHGREG_LKEYCONF_FULLNAME,
					registData.fullname, Resource::MAX_LOADSTRING);
			GetDlgItemText(hwndDlg, IDC_CHGREG_LKEYCONF_EMAIL, registData.email,
					Resource::MAX_LOADSTRING);
			ContractType type;
			if (Button_GetCheck(GetDlgItem(hwndDlg, IDC_CHGREG_LKEYGET_SUBSCRIPTION))) {
				type = Subscription;
			} else if (Button_GetCheck(GetDlgItem(hwndDlg, IDC_CHGREG_LKEYGET_PACKAGE))) {
				type = Package;
			} else {
				type = NoSupport;
			}

			// ライセンスキー発行依頼(ユーザー登録)
			LPCTSTR company = registData.company;
			LPCTSTR unit = registData.department;
			LPCTSTR name = registData.fullname;
			LPCTSTR email = registData.email;
			LicenseManager licenseManager(LICENSE_SERVER_BASE_URL);
			Future<bool> registResult = licenseManager.regist(company, unit,
					name, email, getMachineGuid().c_str(), type, getVersion().c_str());

			// プログレスバー表示
			ProgressDialog::show(registResult);

			if (registResult.get()) {
				// ユーザー登録に成功
				Resource::showMessageBox(hwndDlg, IDS_CHGREG_LKEYGET_MESSAGE1,
				IDS_CHGREG_TITLE_INFO, MB_ICONINFORMATION);
			} else {
				// ユーザー登録に失敗
				Resource::showMessageBox(hwndDlg, IDS_CHGREG_LKEYGET_MESSAGE2,
				IDS_CHGREG_TITLE_ERROR, MB_ICONERROR);
			}

			EndDialog(hwndDlg, IDOK);
			PostMessage(GetParent(hwndDlg), WM_CLOSE, wmId, lParam);
		}
			break;

		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			break;
		}
		return (INT_PTR) TRUE;
	}
	return (INT_PTR) FALSE;
}

INT_PTR ChgRegLkeyConfDialog::create(HWND parent, LPARAM param) {
	return Resource::showDialog(IDD_CHGREG_LKEYCONF, parent, handleEvent, param);
}
