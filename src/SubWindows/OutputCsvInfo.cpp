/*
 * OutputCsvInfo.cpp
 *
 *  Created on: 2016/03/04
 *      Author: PC-EFFECT-011
 */

#include "SubWindows/OutputCsvInfo.h"
#include "CsvData.h"
#include "MainWindow.h"

#include <string.h>

// TODO 暫定
#include "FileUtils.h"
#include "Data/ResultData.h"
extern ResultData *result;

inline void setCheckExistFilePath(LPTSTR path) {
	TCHAR *path2 = new TCHAR[MAX_PATH];
	lstrcpy(path2, path);
	toBaseName(path2);
	MainWindow::getInstance()->csvData->setCheckExistFilePath(path2);
	delete path2;
}

inline void setHotspotInfoPath(LPTSTR path) {
	TCHAR *path2 = new TCHAR[MAX_PATH];
	lstrcpy(path2, path);
	toBaseName(path2);
	wcscat(path2, TEXT("\\"));
	wcscat(path2, Resource::getString(IDS_CSV_HOTSPOT_FILENAME));
	MainWindow::getInstance()->csvData->setHotspotInfoPath(path2);
	delete path2;
}

inline void setCustomerInfoPath(LPTSTR path) {
	TCHAR *path2 = new TCHAR[MAX_PATH];
	lstrcpy(path2, path);
	wcscat(path2, TEXT("\\"));
	wcscat(path2, Resource::getString(IDS_CSV_CUSTOMER_FILENAME));
	MainWindow::getInstance()->csvData->setCustomerInfoPath(path2);
	delete path2;
}

inline void setMachineInfoPath(LPTSTR path) {
	TCHAR *path2 = new TCHAR[MAX_PATH];
	lstrcpy(path2, path);
	wcscat(path2, TEXT("\\"));
	wcscat(path2, Resource::getString(IDS_CSV_MACHINE_FILENAME));
	MainWindow::getInstance()->csvData->setMachineInfoPath(path2);
	delete path2;
}

inline void setReport2DataPath(LPTSTR path) {
	TCHAR *path2 = new TCHAR[MAX_PATH];
	lstrcpy(path2, path);
	toBaseName(path2);
	wcscat(path2, TEXT("\\"));
	wcscat(path2, Resource::getString(IDS_TXT_REPORT2_FILENAME));
	MainWindow::getInstance()->csvData->setReport2DataPath(path2);
	delete path2;
}

inline void setWatchOnDataInfoPath(LPTSTR path) {
	TCHAR *path2 = new TCHAR[MAX_PATH];
	lstrcpy(path2, path);
	toBaseName(path2);
	wcscat(path2, TEXT("\\"));
	// 飛行データが保存されているファイル名を取得する
	std::vector<TCHAR*> files;
	// TODO 暫定でWatchOn_Result*.txtからデータを取得する
	searchFiles(result->getDataPath(), TEXT("WatchOn_Result*.txt"), files);
	if (!files.empty()) {
		// 最初に見つかったファイルを読み込む
		wcscat(path2, files[0]);
		// 後片付け
		searchFiles_cleanup(files);
	}

	MainWindow::getInstance()->csvData->setWatchOnDataInfoPath(path2);
	delete path2;
}

// (2017/6/2YM)追加データファイルパスセット処理を追加
inline void setAddDataPath(LPTSTR path) {
	TCHAR *path2 = new TCHAR[MAX_PATH];
	lstrcpy(path2, path);
	toBaseName(path2);
	wcscat(path2, TEXT("\\"));
	wcscat(path2, Resource::getString(IDS_CSV_ADDDATA_FILENAME));
	MainWindow::getInstance()->csvData->setAddDataPath(path2);
	delete path2;
}

/*
 * コンストラクタ
 */
OutputCsvInfoDialog::OutputCsvInfoDialog(HWND handle) :
		Dialog(handle) {
}

/*
 * デストラクタ
 */
OutputCsvInfoDialog::~OutputCsvInfoDialog() {
}

/*
 * イベントハンドラ
 */
INT_PTR CALLBACK OutputCsvInfoDialog::handleEvent(HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam) {
	UINT wmId;
	TCHAR *pPath;
	TCHAR *extName;
	TCHAR path[Resource::MAX_LOADSTRING];

	switch (uMsg) {
	case WM_INITDIALOG:
		SetDlgItemText(hWnd, IDC_OUTPUTREPORT_FILENAME,
				MainWindow::getInstance()->csvData->getReportPath());
#if 0
		SetDlgItemText(hWnd, IDC_OUTPUTCSV_HOTSPOT, MainWindow::getInstance()->csvData->getHotspotInfoPath());
		SetDlgItemText(hWnd, IDC_OUTPUTCSV_CUSTOMER, MainWindow::getInstance()->csvData->getCustomerInfoPath());
		SetDlgItemText(hWnd, IDC_OUTPUTCSV_MACHINE, MainWindow::getInstance()->csvData->getMachineInfoPath());
#endif // 0
		return (INT_PTR) true;

	case WM_CLOSE:
		EndDialog(hWnd, IDCANCEL);
		break;

	case WM_COMMAND:
		wmId = LOWORD(wParam);
		switch (wmId) {
		case IDOK:
			extName = getExt(
					MainWindow::getInstance()->csvData->getReportPath());
			if (lstrcmp(extName, TEXT("xlsm")) == 0) {
				GetDlgItemText(hWnd, IDC_OUTPUTREPORT_FILENAME, path,
						Resource::MAX_LOADSTRING);
				MainWindow::getInstance()->csvData->setReportPath(path);
				setHotspotInfoPath(path);
				setCustomerInfoPath(result->getCachePath());
				setMachineInfoPath(result->getExePath());
				setReport2DataPath(path);
				setWatchOnDataInfoPath(path);
				setCheckExistFilePath(path);
				// (2017/6/2YM)追加データファイルパスをセット
				setAddDataPath(path);
				EndDialog(hWnd, IDOK);
			} else {
				// 拡張子が.xlsm以外の場合
				TCHAR title[Resource::MAX_LOADSTRING];
				TCHAR message[Resource::MAX_LOADSTRING];
				Resource::getString(IDS_OUTPUT_TITLE, title);
				Resource::getString(IDS_EXT_IS_NOT_XLSM, message);
				MessageBox(hWnd, message, title, MB_OK | MB_ICONERROR);
			}
			break;

		case IDCANCEL:
			EndDialog(hWnd, IDCANCEL);
			break;

		case IDC_OUTREPORT_RENAME:
			pPath = MainWindow::getInstance()->showSaveXLSMFileDialog();
			if (pPath != NULL) {
				MainWindow::getInstance()->csvData->setReportPath(pPath);
				SetDlgItemText(hWnd, IDC_OUTPUTREPORT_FILENAME,
						MainWindow::getInstance()->csvData->getReportPath());
				delete pPath;
			}
			break;
#if 0
		case IDC_OUTPUTCSV_HOTSPOT_RENAME:
			pPath = MainWindow::getInstance()->showSaveCSVFileDialog();
			if (pPath != NULL) {
				MainWindow::getInstance()->csvData->setHotspotInfoPath(pPath);
				SetDlgItemText(hWnd, IDC_OUTPUTCSV_HOTSPOT, MainWindow::getInstance()->csvData->getHotspotInfoPath());
				delete pPath;
			}
			break;
		case IDC_OUTPUTCSV_CUSTOMER_RENAME:
			pPath = MainWindow::getInstance()->showSaveCSVFileDialog();
			if (pPath != NULL) {
				MainWindow::getInstance()->csvData->setCustomerInfoPath(pPath);
				SetDlgItemText(hWnd, IDC_OUTPUTCSV_CUSTOMER, MainWindow::getInstance()->csvData->getCustomerInfoPath());
				delete pPath;
			}
			break;
		case IDC_OUTPUTCSV_MACHINE_RENAME:
			pPath = MainWindow::getInstance()->showSaveCSVFileDialog();
			if (pPath != NULL) {
				MainWindow::getInstance()->csvData->setMachineInfoPath(pPath);
				SetDlgItemText(hWnd, IDC_OUTPUTCSV_MACHINE, MainWindow::getInstance()->csvData->getMachineInfoPath());
				delete pPath;
			}
			break;
#endif // 0
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
INT_PTR OutputCsvInfoDialog::create(HWND parent) {
	return Resource::showDialog(IDD_OUTPUT_REPORT, parent,
			OutputCsvInfoDialog::handleEvent);
}
