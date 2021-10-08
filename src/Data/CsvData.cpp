/*
 * CsvData.cpp
 *
 *  Created on: 2016/03/07
 *      Author: PC-EFFECT-011
 */
// (2017/6/2YM)追加データファイル関連の処理を追加
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <stdio.h>

#include "CsvData.h"
#include "app.h"

CsvData::CsvData() {
	TCHAR userDir[MAX_PATH];
	TCHAR passData[] = TEXT("\\");
	TCHAR *pFileName;

	// ユーザフォルダ取得
	SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, userDir);

	// 報告書
	pFileName = Resource::getString(IDS_XLSM_REPORT_FILENAME);
	lstrcpy(this->reportPath, userDir);
	lstrcat(this->reportPath, passData);
	lstrcat(this->reportPath, pFileName);

	// ホットスポット情報
	pFileName = Resource::getString(IDS_CSV_HOTSPOT_FILENAME);
	lstrcpy(this->hotspotInfoPath, userDir);
	lstrcat(this->hotspotInfoPath, passData);
	lstrcat(this->hotspotInfoPath, pFileName);

	// 基本情報
	pFileName = Resource::getString(IDS_CSV_CUSTOMER_FILENAME);
	lstrcpy(this->customerInfoPath, userDir);
	lstrcat(this->customerInfoPath, passData);
	lstrcat(this->customerInfoPath, pFileName);
	ZeroMemory(&this->customerInfoParam, sizeof(CustomerInfoParameter));

	// 機器情報
	pFileName = Resource::getString(IDS_CSV_MACHINE_FILENAME);
	lstrcpy(this->machineInfoPath, userDir);
	lstrcat(this->machineInfoPath, passData);
	lstrcat(this->machineInfoPath, pFileName);
	ZeroMemory(&this->machineInfoParam, sizeof(MachineInfoParameter));
}

CsvData::~CsvData() {
}

/**
 * 報告書ファイル名 設定
 */
void CsvData::setReportPath(TCHAR *path) {
	lstrcpy(this->reportPath, path);
}
/**
 *  報告書ファイル名 取得
 */
TCHAR* CsvData::getReportPath() {
	return this->reportPath;
}

/**
 * 存在をチェックするパスを設定
 */
void CsvData::setCheckExistFilePath(TCHAR *path) {
	lstrcpy(this->checkPath, path);
}
/**
 * ファイルパスが存在するか確認
 */
bool CsvData::checkExistFilePath(void) {
	int check1 = PathFileExists(checkPath);
	// ファイルかフォルダか判別
	int check2 = checkOpenfile();
	if ((check1 == true) && (check2 == false)) {
		return true;
	}
	return false;
}
bool CsvData::checkOpenfile(void) {
	FILE *file = fileOpen(checkPath, TEXT("r"));
	if (file == NULL) {
		// falseならフォルダ
		return false;
	}
	fclose(file);
	return true;
}

/**
 * ホットスポット情報 ファイル名 設定
 */
void CsvData::setHotspotInfoPath(TCHAR *path) {
	lstrcpy(this->hotspotInfoPath, path);
}
/**
 * ホットスポット情報 ファイル名 取得
 */
TCHAR* CsvData::getHotspotInfoPath() {
	return this->hotspotInfoPath;
}

/**
 * 基本情報 ファイル名 設定
 */
void CsvData::setCustomerInfoPath(TCHAR *path) {
	lstrcpy(this->customerInfoPath, path);
}
/**
 * 基本情報 ファイル名 取得
 */
TCHAR* CsvData::getCustomerInfoPath() {
	return this->customerInfoPath;
}

/**
 * 基本情報 パラメータ 設定
 */
void CsvData::setCustomerInfoParameter(CustomerInfoParameter *param) {
	memcpy(&this->customerInfoParam, param, sizeof(CustomerInfoParameter));
}
/**
 * 基本情報 パラメータ 取得
 */
CsvData::CustomerInfoParameter* CsvData::getCustomerInfoParameter() {
	return &this->customerInfoParam;
}

/**
 * 機器情報 ファイル名 設定
 */
void CsvData::setMachineInfoPath(TCHAR *path) {
	lstrcpy(this->machineInfoPath, path);
}
/**
 * 機器情報 ファイル名 取得
 */
TCHAR* CsvData::getMachineInfoPath() {
	return this->machineInfoPath;
}

/**
 * 機器情報 パラメータ 設定
 */
void CsvData::setMachineInfoParameter(MachineInfoParameter *param) {
	memcpy(&this->machineInfoParam, param, sizeof(MachineInfoParameter));
}
/**
 * 機器情報 パラメータ 取得
 */
CsvData::MachineInfoParameter* CsvData::getMachineInfoParameter() {
	return &this->machineInfoParam;
}

/**
 * 報告書2データファイル名設定
 */
void CsvData::setReport2DataPath(TCHAR *path) {
	lstrcpy(this->report2InfoPath, path);
}
/**
 * 報告書2データファイル名取得
 */
TCHAR* CsvData::getReport2DataPath() {
	return this->report2InfoPath;
}

/**
 * WatchOn_Data.txt ファイル名設定
 */
void CsvData::setWatchOnDataInfoPath(TCHAR *path) {
	lstrcpy(this->watchOnDataInfoPath, path);
}
/**
 * WatchOn_Data.txt ファイル名取得
 */
TCHAR* CsvData::getWatchOnDataInfoPath(void) {
	return this->watchOnDataInfoPath;
}

/**
 * WatchOn_Data.txtを元ファイルからコピー
 */
bool CsvData::copyWatchOnDataFile(TCHAR *filePath) {
	LPCTSTR path = filePath; // 元ファイル
	LPCTSTR path2 = watchOnDataInfoPath; // 出力ファイル
	return CopyFile(path, path2, false);
}

// (2017/6/2YM)追加データファイルをセットする処理
void CsvData::setAddDataPath(TCHAR *path) {
	lstrcpy(this->AddDataInfoPath, path);
}

// (2017/6/2YM)追加データファイルをセットする処理
TCHAR* CsvData::getAddDataPath() {
	return this->AddDataInfoPath;
}
