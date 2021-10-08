/*
 * CustomerInfo.cpp
 *
 *  Created on: 2016/02/15
 *      Author: PC-EFFECT-002
 */
#include<fstream>
#include<iostream>

#include "app.h"
#include "SubWindows/CustomerInfo.h"
#include "CsvData.h"
#include "MainWindow.h"

#include "StringUtils.h"

INT_PTR CALLBACK CustomerInfoDialog::handleEvent(HWND hwndDlg, // ダイアログボックスのハンドル
		UINT uMsg,     // メッセージ
		WPARAM wParam, // 最初のメッセージパラメータ
		LPARAM lParam  // 2 番目のメッセージパラメータ
		) {
	CsvData::CustomerInfoParameter *pCustomerInfoParam;
	CsvData::CustomerInfoParameter customerInfoParam;
	HWND comboBox;
	HWND dateTime;

	switch (uMsg) {
	case WM_INITDIALOG:
		pCustomerInfoParam =
				MainWindow::getInstance()->csvData->getCustomerInfoParameter();
		SetDlgItemText(hwndDlg, IDC_CUSTOMER_INFO_CUSTOMER_NAME,
				pCustomerInfoParam->customerName);
		SetDlgItemText(hwndDlg, IDC_CUSTOMER_INFO_ADDRESS,
				pCustomerInfoParam->address);
		SetDlgItemText(hwndDlg, IDC_CUSTOMER_INFO_PLANT_NAME,
				pCustomerInfoParam->plantName);
		setDialogItemTextAsInt(hwndDlg, IDC_CUSTOMER_INFO_PLANT_OUTPUT,
				pCustomerInfoParam->plantOutput);
		setDialogItemTextAsInt(hwndDlg, IDC_CUSTOMER_INFO_PANEL_COUNT,
				pCustomerInfoParam->panelCount);
		SetDlgItemText(hwndDlg, IDC_CUSTOMER_INFO_PANEL_TYPE,
				pCustomerInfoParam->panelType);
		SetDlgItemText(hwndDlg, IDC_CUSTOMER_INFO_PANEL_MAKER,
				pCustomerInfoParam->panelMaker);

		Edit_LimitText(GetDlgItem(hwndDlg,IDC_CUSTOMER_INFO_PLANT_OUTPUT), 9);
		Edit_LimitText(GetDlgItem(hwndDlg,IDC_CUSTOMER_INFO_PANEL_COUNT), 9);

		comboBox = GetDlgItem(hwndDlg, IDC_CUSTOMER_INFO_WEATHER);
		ComboBox_AddString(comboBox,
				Resource::getString(IDS_REPORT_WEATHER_SUNNY));
		ComboBox_AddString(comboBox,
				Resource::getString(IDS_REPORT_WEATHER_CLOUDY));
		ComboBox_SelectString(comboBox, -1, pCustomerInfoParam->weather);

		SetDlgItemText(hwndDlg, IDC_CUSTOMER_INFO_FLIGHT_PERSON,
				pCustomerInfoParam->flightPerson);
		SetDlgItemText(hwndDlg, IDC_CUSTOMER_INFO_PATROL_PERSON,
				pCustomerInfoParam->patrolPerson);

		dateTime = GetDlgItem(hwndDlg, IDC_CUSTOMER_INFO_SETTING_DATE);
		DateTime_SetSystemtime(dateTime, GDT_VALID,
				&pCustomerInfoParam->settingDate);
		return (INT_PTR) TRUE;

	case WM_CLOSE:
		EndDialog(hwndDlg, IDCANCEL);
		break;

	case WM_COMMAND:
		UINT wmId = LOWORD(wParam);
		switch (wmId) {
		case IDOK:
			GetDlgItemText(hwndDlg, IDC_CUSTOMER_INFO_CUSTOMER_NAME,
					customerInfoParam.customerName, Resource::MAX_LOADSTRING);
			GetDlgItemText(hwndDlg, IDC_CUSTOMER_INFO_ADDRESS,
					customerInfoParam.address, Resource::MAX_LOADSTRING);
			GetDlgItemText(hwndDlg, IDC_CUSTOMER_INFO_PLANT_NAME,
					customerInfoParam.plantName, Resource::MAX_LOADSTRING);
			customerInfoParam.plantOutput = getDialogItemTextAsInt(hwndDlg,
			IDC_CUSTOMER_INFO_PLANT_OUTPUT);
			customerInfoParam.panelCount = getDialogItemTextAsInt(hwndDlg,
			IDC_CUSTOMER_INFO_PANEL_COUNT);
			GetDlgItemText(hwndDlg, IDC_CUSTOMER_INFO_PANEL_TYPE,
					customerInfoParam.panelType, Resource::MAX_LOADSTRING);
			GetDlgItemText(hwndDlg, IDC_CUSTOMER_INFO_PANEL_MAKER,
					customerInfoParam.panelMaker, Resource::MAX_LOADSTRING);
			GetDlgItemText(hwndDlg, IDC_CUSTOMER_INFO_WEATHER,
					customerInfoParam.weather, Resource::MAX_LOADSTRING);
			GetDlgItemText(hwndDlg, IDC_CUSTOMER_INFO_FLIGHT_PERSON,
					customerInfoParam.flightPerson, Resource::MAX_LOADSTRING);
			GetDlgItemText(hwndDlg, IDC_CUSTOMER_INFO_PATROL_PERSON,
					customerInfoParam.patrolPerson, Resource::MAX_LOADSTRING);
			dateTime = GetDlgItem(hwndDlg, IDC_CUSTOMER_INFO_SETTING_DATE);
			DateTime_GetSystemtime(dateTime, &customerInfoParam.settingDate);

			MainWindow::getInstance()->csvData->setCustomerInfoParameter(
					&customerInfoParam);
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

INT_PTR CustomerInfoDialog::create(HWND parent) {
	return Resource::showDialog(IDD_CUSTOMER_INFO, parent, handleEvent);
}

bool CustomerInfoDialog::saveCustomerInfo(const TCHAR *filePath) {
	// データをファイルに保存する
	FILE *file = fileOpen(filePath, TEXT("w"));
	if (file == NULL) {
		return false;
	}
	CsvData::CustomerInfoParameter *customerInfo =
			MainWindow::getInstance()->csvData->getCustomerInfoParameter();

	char *utf8customerName = toUTF8String(customerInfo->customerName);
	char *utf8address = toUTF8String(customerInfo->address);
	char *utf8plantName = toUTF8String(customerInfo->plantName);
	char *utf8panelType = toUTF8String(customerInfo->panelType);
	char *utf8panelMaker = toUTF8String(customerInfo->panelMaker);
	char *utf8weather = toUTF8String(customerInfo->weather);
	char *utf8flightPerson = toUTF8String(customerInfo->flightPerson);
	char *utf8patrolPerson = toUTF8String(customerInfo->patrolPerson);

	// ファイルに書き込む
	fprintf(file, "%s\n%s\n%s\n%d\n%d\n%s\n%s\n%s\n%s\n%s\n%d/%d/%d\n",
			utf8customerName, utf8address, utf8plantName,
			customerInfo->plantOutput, customerInfo->panelCount, utf8panelType,
			utf8panelMaker, utf8weather, utf8flightPerson, utf8patrolPerson,
			customerInfo->settingDate.wYear, customerInfo->settingDate.wMonth,
			customerInfo->settingDate.wDay);

	delete utf8customerName;
	delete utf8address;
	delete utf8plantName;
	delete utf8panelType;
	delete utf8panelMaker;
	delete utf8weather;
	delete utf8flightPerson;
	delete utf8patrolPerson;

	fclose(file);

	return true;
}

bool CustomerInfoDialog::loadCustomerInfo(const TCHAR *filePath) {
	//　ファイルを開く
	FILE *file = fileOpen(filePath, TEXT("r"));
	if (file == NULL) {
		return false;
	}

	CsvData::CustomerInfoParameter customerInfo;
	ZeroMemory(&customerInfo, sizeof(CsvData::CustomerInfoParameter)); //　0クリア

	const int maxLen = Resource::MAX_LOADSTRING * 4;
	int nLine = CustomerInfoDialog::PARAM_CUSTOMER_NAME;
	char *pBuff = new char[maxLen];

	// ファイルから読み込む
	while (fgets(pBuff, maxLen, file) != NULL) {
		switch (nLine) {
		case CustomerInfoDialog::PARAM_CUSTOMER_NAME:	// 顧客名
			CustomerInfoDialog::getParameterText(pBuff,
					customerInfo.customerName);
			break;

		case CustomerInfoDialog::PARAM_ADDRESS:			// 住所
			CustomerInfoDialog::getParameterText(pBuff, customerInfo.address);
			break;

		case CustomerInfoDialog::PARAM_PLANT_NAME:		// 発電所名
			CustomerInfoDialog::getParameterText(pBuff, customerInfo.plantName);
			break;

		case CustomerInfoDialog::PARAM_PLANT_OUTPUT:	// 出力(kW)
			CustomerInfoDialog::getParameterInt(pBuff,
					&customerInfo.plantOutput);
			break;

		case CustomerInfoDialog::PARAM_PANEL_COUNT:		// パネル枚数(枚)
			CustomerInfoDialog::getParameterInt(pBuff,
					&customerInfo.panelCount);
			break;

		case CustomerInfoDialog::PARAM_PANEL_TYPE:		// パネル型式
			CustomerInfoDialog::getParameterText(pBuff, customerInfo.panelType);
			break;

		case CustomerInfoDialog::PARAM_PANEL_MAKER:		// パネルメーカー
			CustomerInfoDialog::getParameterText(pBuff,
					customerInfo.panelMaker);
			break;

		case CustomerInfoDialog::PARAM_WEATHER:			// 天気
			CustomerInfoDialog::getParameterText(pBuff, customerInfo.weather);
			break;

		case CustomerInfoDialog::PARAM_FLIGHT_PERSON:	// 飛行担当者
			CustomerInfoDialog::getParameterText(pBuff,
					customerInfo.flightPerson);
			break;

		case CustomerInfoDialog::PARAM_PATROL_PERSON:	// 監視人
			CustomerInfoDialog::getParameterText(pBuff,
					customerInfo.patrolPerson);
			break;

		case CustomerInfoDialog::PARAM_SETTING_DATE:	// 設置年月日
			CustomerInfoDialog::getParameterDate(pBuff,
					&customerInfo.settingDate);
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
	MainWindow::getInstance()->csvData->setCustomerInfoParameter(&customerInfo);

	return true;
}

bool CustomerInfoDialog::itemOutput_CustomerInfo(HWND hwnd) {
	int ret = 0;

	// 顧客情報入力ダイアログを表示
	TCHAR *pFilePath =
			MainWindow::getInstance()->csvData->getCustomerInfoPath();
	if (pFilePath != NULL) {
		CustomerInfoDialog::loadCustomerInfo(pFilePath);
		ret = CustomerInfoDialog::create(hwnd);
		if (ret == IDOK) {
			CustomerInfoDialog::saveCustomerInfo(pFilePath);
		} else {
			return false;
		}
	} else {
		return false;
	}
	return true;
}

bool CustomerInfoDialog::getParameterText(char *pBuff, TCHAR *pParam) {
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

bool CustomerInfoDialog::getParameterInt(char *pBuff, int *pParam) {
	bool ret = false;

	if (pBuff != NULL) {
		// パラメータ取得
		int param = atoi(pBuff);
		*pParam = param;
		// 戻り値設定
		ret = true;
	}

	return ret;
}

bool CustomerInfoDialog::getParameterDate(char *pBuff, SYSTEMTIME *pDate) {
	bool ret = false;
	const char separaterCode[] = "/";
	int count = CustomerInfoDialog::DATE_YEAR;
	int param;

	while ((pBuff != NULL) && (ret != true)) {
		// 区切り文字の場合
		if (*pBuff == separaterCode[0]) {
			// 次の文字へ
			pBuff++;
			// 次の項目へ
			count++;
			continue;
		}

		// パラメータ取得
		switch (count) {
		case CustomerInfoDialog::DATE_YEAR:		// 年
			param = atoi(pBuff);
			pDate->wYear = (WORD) param;
			break;

		case CustomerInfoDialog::DATE_MONTH:	// 月
			param = atoi(pBuff);
			pDate->wMonth = (WORD) param;
			break;

		case CustomerInfoDialog::DATE_DAY:		// 日
			param = atoi(pBuff);
			pDate->wDay = (WORD) param;
			// 戻り値設定
			ret = true;
			break;

		default:
			break;
		}
		// 次の区切り文字まで読み飛ばし
		pBuff = strstr(pBuff, separaterCode);
	}

	return ret;
}
