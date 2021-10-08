/*
 * PanelSetting.cpp
 *
 *  Created on: 2016/01/29
 *      Author: PC-EFFECT-002
 */

#include <stdio.h>

#include "app.h"
#include "MainWindow.h"
#include "resource.h"

#include "SubWindows/PanelSetting.h"

inline int showAlertMessage(HWND hwndDlg, LPCTSTR message) {
	// 入力を促すメッセージ表示
	TCHAR title[Resource::MAX_LOADSTRING];
	Resource::getString(IDS_SET_NO_TITLE, title);
	return MessageBox(hwndDlg, message, title, MB_OK | MB_ICONEXCLAMATION);
}

inline int showAlertMessage(HWND hwndDlg, int messageId) {
	TCHAR message[Resource::MAX_LOADSTRING];
	Resource::getString(messageId, message);
	return showAlertMessage(hwndDlg, message);
}

inline bool initDialog(HWND hwndDlg,  // ダイアログボックスのハンドル
		UINT uMsg,     // メッセージ
		WPARAM wParam, // 最初のメッセージパラメータ
		LPARAM lParam  // 2 番目のメッセージパラメータ
		) {
	// コントロール設定
	Edit_LimitText(GetDlgItem(hwndDlg, IDC_PANELSETTING_LINE1), 3);
	Edit_LimitText(GetDlgItem(hwndDlg, IDC_PANELSETTING_START1), 4);
	Edit_LimitText(GetDlgItem(hwndDlg, IDC_PANELSETTING_START2), 4);

	// 文字列設定
	TCHAR text[Resource::MAX_LOADSTRING];
	stprintf(text, TEXT("%s"),
			MainWindow::getInstance()->panelSettingData->getLine1());
	SetDlgItemText(hwndDlg, IDC_PANELSETTING_LINE1, text);
	stprintf(text, TEXT("%d"),
			MainWindow::getInstance()->panelSettingData->getStart1());
	SetDlgItemText(hwndDlg, IDC_PANELSETTING_START1, text);
	stprintf(text, TEXT("%s"),
			MainWindow::getInstance()->panelSettingData->getLine2());
	SetDlgItemText(hwndDlg, IDC_PANELSETTING_LINE2, text);
	stprintf(text, TEXT("%d"),
			MainWindow::getInstance()->panelSettingData->getStart2());
	SetDlgItemText(hwndDlg, IDC_PANELSETTING_START2, text);

	return true;
}

inline bool pressOKButton(HWND hwndDlg,  // ダイアログボックスのハンドル
		UINT uMsg,     // メッセージ
		WPARAM wParam, // 最初のメッセージパラメータ
		LPARAM lParam  // 2 番目のメッセージパラメータ
		) {
	TCHAR text[Resource::MAX_LOADSTRING];
	int len;
	int startNum, endNum;

	// 開始番号の読み込み
	len = GetDlgItemText(hwndDlg, IDC_PANELSETTING_START1, text,
			Resource::MAX_LOADSTRING);
	if (len != 0) {
		startNum = toInt(text);
	} else {
		// 入力を促すメッセージ表示
		showAlertMessage(hwndDlg, IDS_NO_INPUT_START);
		SetFocus(GetDlgItem(hwndDlg, IDC_PANELSETTING_START1));
		return false;
	}

	// 終了番号の読み込み
	len = GetDlgItemText(hwndDlg, IDC_PANELSETTING_START2, text,
			Resource::MAX_LOADSTRING);
	if (len != 0) {
		endNum = toInt(text);
	} else {
		// 入力を促すメッセージ表示
		showAlertMessage(hwndDlg, IDS_NO_INPUT_END);
		SetFocus(GetDlgItem(hwndDlg, IDC_PANELSETTING_START2));
		return false;
	}

	if (startNum > endNum) {
		// エラーメッセージ表示
		showAlertMessage(hwndDlg, IDS_NO_INPUT_START_END);
		SetFocus(GetDlgItem(hwndDlg, IDC_PANELSETTING_START2));
		return false;
	}

	len = GetDlgItemText(hwndDlg, IDC_PANELSETTING_LINE1, text,
			Resource::MAX_LOADSTRING);
	if (len == 0) {
		lstrcpy(text, TEXT("A"));
	}
	MainWindow::getInstance()->panelSettingData->setLine1(text);
	MainWindow::getInstance()->panelSettingData->setLine2(text);
	MainWindow::getInstance()->panelSettingData->setStart1(startNum);
	MainWindow::getInstance()->panelSettingData->setStart2(endNum);
	return true;
}

INT_PTR CALLBACK PanelSettingDialog::handleEvent(HWND hwndDlg, // ダイアログボックスのハンドル
		UINT uMsg,     // メッセージ
		WPARAM wParam, // 最初のメッセージパラメータ
		LPARAM lParam  // 2 番目のメッセージパラメータ
		) {
	switch (uMsg) {
	case WM_INITDIALOG:
		initDialog(hwndDlg, uMsg, wParam, lParam);
		return (INT_PTR) TRUE;

	case WM_CLOSE:
		EndDialog(hwndDlg, IDCANCEL);
		break;

	case WM_COMMAND:
		UINT wmId = LOWORD(wParam);
		UINT code = HIWORD(wParam);
		switch (wmId) {
		case IDC_PANELSETTING_LINE1:
			switch (code) {
			case EN_CHANGE:
				TCHAR text[Resource::MAX_LOADSTRING];
				int length = GetDlgItemText(hwndDlg, IDC_PANELSETTING_LINE1,
						text, Resource::MAX_LOADSTRING);
				TCHAR space = TEXT(' ');
				TCHAR zenSpace = TEXT('　');
				bool changed = false;
				for (int i = 0; i < length; i++) {
					if (text[i] == space) {
						text[i] = zenSpace;
						changed = true;
					}
				}

				if (changed == true) {
					SetDlgItemText(hwndDlg, IDC_PANELSETTING_LINE1, text);
				}
				SetDlgItemText(hwndDlg, IDC_PANELSETTING_LINE2, text);
			}
			break;

		case IDOK:
			if (pressOKButton(hwndDlg, uMsg, wParam, lParam)) {
				EndDialog(hwndDlg, IDOK);
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

/**
 * 検出設定ダイアログを表示する。
 * @param parent 親ウィンドウのハンドル
 *
 */
INT_PTR PanelSettingDialog::create(HWND parent) {
	MainWindow::getInstance()->panelSettingData->initDialogData();
	return Resource::showDialog(IDD_PANELSETTING, parent, handleEvent);
}
