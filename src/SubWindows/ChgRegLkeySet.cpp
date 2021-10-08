/*
 * ChgRegLkeySetLkeySet.cpp
 *
 *  Created on: 2020/11/26
 *      Author: A.Masago
 */
#include <string.h>
#include "SubWindows/ChgReg.h"
#include "SubWindows/ChgRegLkeySet.h"
#include "MainWindow.h"
#include "Protection.h"
#include "LicenseManager.h"
#include <SubWindows/ProgressDialog.h>

// ライセンスキー情報長
#define LKEY_LENGTH 24

INT_PTR CALLBACK ChgRegLkeySetDialog::handleEvent(HWND hwndDlg, // ダイアログボックスのハンドル
		UINT uMsg,     // メッセージ
		WPARAM wParam, // 最初のメッセージパラメータ
		LPARAM lParam  // 2 番目のメッセージパラメータ
		) {

	switch (uMsg) {
	case WM_INITDIALOG:
		SendDlgItemMessage(hwndDlg, IDC_CHGREG_LKEYSET_LKEY, EM_SETLIMITTEXT,
		LKEY_LENGTH, 0);
		return (INT_PTR) TRUE;

	case WM_CLOSE:
		EndDialog(hwndDlg, IDCANCEL);
		break;

	case WM_COMMAND:
		UINT wmId = LOWORD(wParam);
		switch (wmId) {
		case IDOK: {
			// ライセンスキー情報のレジストリ登録
			TCHAR keyInfo[LKEY_LENGTH + 1];
			ZeroMemory(keyInfo, sizeof(keyInfo));
			GetDlgItemText(hwndDlg, IDC_CHGREG_LKEYSET_LKEY, keyInfo,
			LKEY_LENGTH + 1);
			int length = lstrlen(keyInfo);
			if (length != LKEY_LENGTH) {
				Resource::showMessageBox(hwndDlg, IDS_CHGREG_LKEYSET_MESSAGE2,
						IDS_CHGREG_TITLE_WARNING, MB_ICONWARNING);
			} else {
				LPTSTR strText = (LPTSTR) malloc(length * sizeof(TCHAR));
				wcscpy(strText, keyInfo);

				if (registInstallKey(strText)) {
					// アクティベーション実行
					if (checkLicense()) {
						// 登録成功＋アクティベーション成功
						Resource::showMessageBox(hwndDlg,
								IDS_CHGREG_LKEYSET_MESSAGE3,
								IDS_CHGREG_TITLE_INFO, MB_ICONINFORMATION);
						setLicenseKeyRegistered(true);
						EndDialog(hwndDlg, IDOK);
					} else {
						// 登録成功＋アクティベーション失敗
						deleteInstallKey();
						Resource::showMessageBox(hwndDlg,
								IDS_CHGREG_LKEYSET_MESSAGE4,
								IDS_CHGREG_TITLE_ERROR, MB_ICONERROR);
						EndDialog(hwndDlg, IDCANCEL);
						PostQuitMessage(0);
					}
				} else {
					// 登録失敗
					Resource::showMessageBox(hwndDlg,
							IDS_CHGREG_LKEYSET_MESSAGE5, IDS_CHGREG_TITLE_ERROR,
							MB_ICONERROR);
					EndDialog(hwndDlg, IDCANCEL);
					PostQuitMessage(0);
				}
				delete strText;
			}
		}
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

INT_PTR ChgRegLkeySetDialog::create(HWND parent) {
	return Resource::showDialog(IDD_CHGREG_LKEYSET, parent, handleEvent);
}
