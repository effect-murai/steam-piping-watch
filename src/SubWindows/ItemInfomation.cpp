/*
 * ItemInfomation.cpp
 *
 *  Created on: 2016/03/30
 *      Author: PC-EFFECT-002
 */
#include "SubWindows/ItemInfomation.h"
#include "Data/CsvData.h"
#include "Data/ResultData.h"
#include "MainWindow.h"
#include <stdio.h>
#include <string.h>
#include "SubWindows/CustomerInfo.h"
#include "SubWindows/MachineInfo.h"
#include "app.h"
#include "CsvData.h"
#include "MainWindow.h"
#include "resource.h"
#include "StringUtils.h"

extern ResultData *result;

inline void loadCustomerInfo(void) {
	TCHAR *fileName = MainWindow::getInstance()->showLoadCSVFileDialog();
	if (fileName != NULL) {
		if (CustomerInfoDialog::loadCustomerInfo(fileName) == false) {
			MainWindow::getInstance()->showMessageBox(IDS_ERR_LOAD_PANEL_INFO,
			IDS_ERROR, MB_OK | MB_ICONEXCLAMATION);
		} else {
			// 成功時のメッセージ表示
		}
		delete fileName;
	}
}

inline void saveCustomerInfo(void) {
	TCHAR *fileName = MainWindow::getInstance()->showSaveCSVFileDialog();
	if (fileName != NULL) {
		if (CustomerInfoDialog::saveCustomerInfo(fileName) == false) {
			MainWindow::getInstance()->showMessageBox(IDS_ERR_SAVE_PANEL_INFO,
			IDS_ERROR, MB_OK | MB_ICONERROR);
		} else {
			// 成功時のメッセージ表示
		}
		delete fileName;
	}
}

inline void loadMachineInfo(void) {
	TCHAR *fileName = MainWindow::getInstance()->showLoadCSVFileDialog();
	if (fileName != NULL) {
		if (MachineInfoDialog::loadMachineInfo(fileName) == false) {
			MainWindow::getInstance()->showMessageBox(IDS_ERR_LOAD_PANEL_INFO,
			IDS_ERROR, MB_OK | MB_ICONEXCLAMATION);
		} else {
			// 成功時のメッセージ表示
		}
		delete fileName;
	}
}

inline void saveMachineInfo(void) {
	TCHAR *fileName = MainWindow::getInstance()->showSaveCSVFileDialog();
	if (fileName != NULL) {
		if (MachineInfoDialog::saveMachineInfo(fileName) == false) {
			MainWindow::getInstance()->showMessageBox(IDS_ERR_SAVE_PANEL_INFO,
			IDS_ERROR, MB_OK | MB_ICONERROR);
		} else {
			// 成功時のメッセージ表示
		}
		delete fileName;
	}
}

inline INT_PTR initDialog(void) {
	TCHAR path[MAX_PATH];

	ZeroMemory(path, sizeof(path));
	stprintf(path, TEXT("%s\\%s"), result->getCachePath(),
			Resource::getString(IDS_CSV_CUSTOMER_FILENAME));
	MainWindow::getInstance()->csvData->setCustomerInfoPath(path);
	CustomerInfoDialog::loadCustomerInfo(path);

	ZeroMemory(path, sizeof(path));
	stprintf(path, TEXT("%s\\%s"), result->getExePath(),
			Resource::getString(IDS_CSV_MACHINE_FILENAME));
	MainWindow::getInstance()->csvData->setMachineInfoPath(path);
	MachineInfoDialog::loadMachineInfo(path);

	return (INT_PTR) true;
}

/*
 * コンストラクタ
 */
ItemInfomationDialog::ItemInfomationDialog(HWND handle) :
		Dialog(handle) {
}

/*
 * デストラクタ
 */
ItemInfomationDialog::~ItemInfomationDialog() {
}

/*
 * イベントハンドラ
 */
INT_PTR CALLBACK ItemInfomationDialog::handleEvent(HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam) {
	UINT wmId;

	switch (uMsg) {
	case WM_INITDIALOG:
		return initDialog();

	case WM_CLOSE:
		EndDialog(hWnd, IDCANCEL);
		break;

	case WM_COMMAND:
		wmId = LOWORD(wParam);
		switch (wmId) {
		case IDOK:
			itemOutput(hWnd);
			EndDialog(hWnd, IDOK);
			break;

		case IDCANCEL:
			EndDialog(hWnd, IDCANCEL);
			break;

		case IDC_CUSTOMER_EDIT:
			CustomerInfoDialog::create(hWnd);
			break;

		case IDC_CUSTOMER_READ:
			loadCustomerInfo();
			break;

		case IDC_CUSTOMER_OUT:
			saveCustomerInfo();
			break;

		case IDC_MACHINE_EDIT:
			MachineInfoDialog::create(hWnd);
			break;

		case IDC_MACHINE_READ:
			loadMachineInfo();
			break;

		case IDC_MACHINE_OUT:
			saveMachineInfo();
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	return (INT_PTR) 0;
}

/*
 * ダイアログ作成
 */
INT_PTR ItemInfomationDialog::create(HWND parent) {
	return Resource::showDialog(IDD_ITEM, parent,
			ItemInfomationDialog::handleEvent);
}

bool ItemInfomationDialog::itemOutput(HWND hwnd) {
	TCHAR str[MAX_PATH];

	ZeroMemory(str, sizeof(str));
	stprintf(str, TEXT("%s\\%s"), result->getCachePath(),
			Resource::getString(IDS_CSV_CUSTOMER_FILENAME));
	MainWindow::getInstance()->csvData->setCustomerInfoPath(str);
	CustomerInfoDialog::saveCustomerInfo(str);

	ZeroMemory(str, sizeof(str));
	stprintf(str, TEXT("%s\\%s"), result->getExePath(),
			Resource::getString(IDS_CSV_MACHINE_FILENAME));
	MainWindow::getInstance()->csvData->setMachineInfoPath(str);
	MachineInfoDialog::saveMachineInfo(str);

	return true;
}
