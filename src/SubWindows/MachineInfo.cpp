/*
 * MachineInfo.cpp
 *
 *  Created on: 2016/02/15
 *      Author: PC-EFFECT-002
 */
#include<fstream>
#include<iostream>

#include "app.h"
#include "SubWindows/MachineInfo.h"
#include "CsvData.h"
#include "MainWindow.h"

#include "StringUtils.h"

INT_PTR CALLBACK MachineInfoDialog::handleEvent(HWND hwndDlg,  // ダイアログボックスのハンドル
		UINT uMsg,     // メッセージ
		WPARAM wParam, // 最初のメッセージパラメータ
		LPARAM lParam  // 2 番目のメッセージパラメータ
		) {
	CsvData::MachineInfoParameter *pMachineInfoParam;
	CsvData::MachineInfoParameter machineInfoParam;

	switch (uMsg) {
	case WM_INITDIALOG:
		pMachineInfoParam =
				MainWindow::getInstance()->csvData->getMachineInfoParameter();

		SetDlgItemText(hwndDlg, IDC_MACHINE_INFO_INSPECTOR,
				pMachineInfoParam->inspector);
		SetDlgItemText(hwndDlg, IDC_MACHINE_INFO_INSPECTOR_ADDRESS,
				pMachineInfoParam->inspectorAdd);
		SetDlgItemText(hwndDlg, IDC_MACHINE_INFO_DRONE_TYPE,
				pMachineInfoParam->droneType);
		SetDlgItemText(hwndDlg, IDC_MACHINE_INFO_DRONE_NUMBER,
				pMachineInfoParam->droneNumber);
		SetDlgItemText(hwndDlg, IDC_MACHINE_INFO_DRONE_DATE,
				pMachineInfoParam->droneDate);
		SetDlgItemText(hwndDlg, IDC_MACHINE_INFO_CAMERA_TYPE,
				pMachineInfoParam->cameraType);
		SetDlgItemText(hwndDlg, IDC_MACHINE_INFO_CAMERA_NUMBER,
				pMachineInfoParam->cameraNumber);
		SetDlgItemText(hwndDlg, IDC_MACHINE_INFO_CAMERA_DATE,
				pMachineInfoParam->cameraDate);
		return (INT_PTR) TRUE;

	case WM_CLOSE:
		EndDialog(hwndDlg, IDCANCEL);
		break;

	case WM_COMMAND:
		UINT wmId = LOWORD(wParam);
		switch (wmId) {
		case IDOK:
			GetDlgItemText(hwndDlg, IDC_MACHINE_INFO_INSPECTOR,
					machineInfoParam.inspector, Resource::MAX_LOADSTRING);
			GetDlgItemText(hwndDlg, IDC_MACHINE_INFO_INSPECTOR_ADDRESS,
					machineInfoParam.inspectorAdd, Resource::MAX_LOADSTRING);
			GetDlgItemText(hwndDlg, IDC_MACHINE_INFO_DRONE_TYPE,
					machineInfoParam.droneType, Resource::MAX_LOADSTRING);
			GetDlgItemText(hwndDlg, IDC_MACHINE_INFO_DRONE_NUMBER,
					machineInfoParam.droneNumber, Resource::MAX_LOADSTRING);
			GetDlgItemText(hwndDlg, IDC_MACHINE_INFO_DRONE_DATE,
					machineInfoParam.droneDate, Resource::MAX_LOADSTRING);
			GetDlgItemText(hwndDlg, IDC_MACHINE_INFO_CAMERA_TYPE,
					machineInfoParam.cameraType, Resource::MAX_LOADSTRING);
			GetDlgItemText(hwndDlg, IDC_MACHINE_INFO_CAMERA_NUMBER,
					machineInfoParam.cameraNumber, Resource::MAX_LOADSTRING);
			GetDlgItemText(hwndDlg, IDC_MACHINE_INFO_CAMERA_DATE,
					machineInfoParam.cameraDate, Resource::MAX_LOADSTRING);
			MainWindow::getInstance()->csvData->setMachineInfoParameter(
					&machineInfoParam);
			EndDialog(hwndDlg, IDOK);
			break;

		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			break;
		}
		return (INT_PTR) TRUE;
	}
	return (INT_PTR) FALSE;
}

INT_PTR MachineInfoDialog::create(HWND parent) {
	return Resource::showDialog(IDD_MACHINE_INFO, parent, handleEvent);
}

bool MachineInfoDialog::saveMachineInfo(const TCHAR *filePath) {
	// データをファイルに保存する
	FILE *file = fileOpen(filePath, TEXT("w"));
	if (file == NULL) {
		return false;
	}

	CsvData::MachineInfoParameter *machineInfoParam =
			MainWindow::getInstance()->csvData->getMachineInfoParameter();

	char *utf8inspector = toUTF8String(machineInfoParam->inspector);
	char *utf8inspectorAdd = toUTF8String(machineInfoParam->inspectorAdd);
	char *utf8droneType = toUTF8String(machineInfoParam->droneType);
	char *utf8droneNumber = toUTF8String(machineInfoParam->droneNumber);
	char *utf8droneDate = toUTF8String(machineInfoParam->droneDate);
	char *utf8cameraType = toUTF8String(machineInfoParam->cameraType);
	char *utf8cameraNumber = toUTF8String(machineInfoParam->cameraNumber);
	char *utf8cameraDate = toUTF8String(machineInfoParam->cameraDate);

	// ファイルに書き込む
	fprintf(file, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n", utf8inspector,
			utf8inspectorAdd, utf8droneType, utf8droneNumber, utf8droneDate,
			utf8cameraType, utf8cameraNumber, utf8cameraDate);

	delete utf8inspector;
	delete utf8inspectorAdd;
	delete utf8droneType;
	delete utf8droneNumber;
	delete utf8droneDate;
	delete utf8cameraType;
	delete utf8cameraNumber;
	delete utf8cameraDate;

	fclose(file);

	return true;
}

bool MachineInfoDialog::loadMachineInfo(const TCHAR *filePath) {
	//　ファイルを開く
	FILE *file = fileOpen(filePath, TEXT("r"));
	if (file == NULL) {
		return false;
	}

	CsvData::MachineInfoParameter machineInfoParam;
	ZeroMemory(&machineInfoParam, sizeof(CsvData::MachineInfoParameter)); //　0クリア

	const int maxLen = Resource::MAX_LOADSTRING * 4;
	int nLine = MachineInfoDialog::PARAM_INSPECTOR;
	char *pBuff = new char[maxLen];

	// ファイルから読み込む
	while (fgets(pBuff, maxLen, file) != NULL) {
		switch (nLine) {
		case MachineInfoDialog::PARAM_INSPECTOR:		// 点検者情報:名称
			MachineInfoDialog::getParameterText(pBuff,
					machineInfoParam.inspector);
			break;

		case MachineInfoDialog::PARAM_INSPECTOR_ADD:	// 点検者情報:住所
			MachineInfoDialog::getParameterText(pBuff,
					machineInfoParam.inspectorAdd);
			break;

		case MachineInfoDialog::PARAM_DRONE_TYPE:		// ドローン情報:型式
			MachineInfoDialog::getParameterText(pBuff,
					machineInfoParam.droneType);
			break;

		case MachineInfoDialog::PARAM_DRONE_NUMBER:		// ドローン情報:製造番号
			MachineInfoDialog::getParameterText(pBuff,
					machineInfoParam.droneNumber);
			break;

		case MachineInfoDialog::PARAM_DRONE_DATE:		// ドローン情報:製造年
			MachineInfoDialog::getParameterText(pBuff,
					machineInfoParam.droneDate);
			break;

		case MachineInfoDialog::PARAM_CAMERA_TYPE:		// 赤外線カメラ情報:型式
			MachineInfoDialog::getParameterText(pBuff,
					machineInfoParam.cameraType);
			break;

		case MachineInfoDialog::PARAM_CAMERA_NUMBER:	// 赤外線カメラ情報:製造番号
			MachineInfoDialog::getParameterText(pBuff,
					machineInfoParam.cameraNumber);
			break;

		case MachineInfoDialog::PARAM_CAMERA_DATE:		// 赤外線カメラ情報:製造年
			MachineInfoDialog::getParameterText(pBuff,
					machineInfoParam.cameraDate);
			break;

		default:
			break;
		}
		// 次の行へ
		nLine++;
	}

	// ファイルを閉じる
	fclose(file);

	// 後片付け
	delete pBuff;

	// パラメータ設定
	MainWindow::getInstance()->csvData->setMachineInfoParameter(
			&machineInfoParam);

	return true;
}

bool MachineInfoDialog::itemOutput_MachineInfo(HWND hwnd) {
	int ret = 0;

	// 装置情報入力ダイアログを表示
	TCHAR *pFilePath = MainWindow::getInstance()->csvData->getMachineInfoPath();
	if (pFilePath != NULL) {
		MachineInfoDialog::loadMachineInfo(pFilePath);
		ret = MachineInfoDialog::create(hwnd);
		if (ret == IDOK) {
			MachineInfoDialog::saveMachineInfo(pFilePath);
		} else {
			return false;
		}
	} else {
		return false;
	}
	return true;
}

bool MachineInfoDialog::getParameterText(char *pBuff, TCHAR *pParam) {
	bool ret = false;

	if (pBuff != NULL) {
		// 改行コード削除
		int buffLen = strlen(pBuff) - 1;
		if (pBuff[buffLen] == '\n') {
			pBuff[buffLen] = '\0';
		}
		// パラメータ取得
		TCHAR *pWork = fromUTF8String(pBuff);
		lstrcpy(pParam, pWork);
		delete pWork;
		// 戻り値設定
		ret = true;
	}

	return ret;
}
