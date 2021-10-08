/*
 * CsvData.h
 *
 *  Created on: 2016/03/07
 *      Author: PC-EFFECT-011
 */
// (2017/6/2YM)追加データファイル処理を追加
#ifndef CSVDATA_H_
#define CSVDATA_H_

#include <windows.h>
#include "resource.h"

class CsvData {
public:
	// 基本情報：パラメータ
	typedef struct {
		TCHAR customerName[Resource::MAX_LOADSTRING];
		TCHAR address[Resource::MAX_LOADSTRING];
		TCHAR plantName[Resource::MAX_LOADSTRING];
		int plantOutput;
		int panelCount;
		TCHAR panelType[Resource::MAX_LOADSTRING];
		TCHAR panelMaker[Resource::MAX_LOADSTRING];
		TCHAR weather[Resource::MAX_LOADSTRING];
		TCHAR flightPerson[Resource::MAX_LOADSTRING];
		TCHAR patrolPerson[Resource::MAX_LOADSTRING];
		SYSTEMTIME settingDate;
	} CustomerInfoParameter;

	// 機器情報：パラメータ
	typedef struct {
		TCHAR inspector[Resource::MAX_LOADSTRING];
		TCHAR inspectorAdd[Resource::MAX_LOADSTRING];
		TCHAR droneType[Resource::MAX_LOADSTRING];
		TCHAR droneNumber[Resource::MAX_LOADSTRING];
		TCHAR droneDate[Resource::MAX_LOADSTRING];
		TCHAR cameraType[Resource::MAX_LOADSTRING];
		TCHAR cameraNumber[Resource::MAX_LOADSTRING];
		TCHAR cameraDate[Resource::MAX_LOADSTRING];
	} MachineInfoParameter;

	CsvData();
	~CsvData();
	/**
	 * 報告書ファイル名 設定
	 */
	void setReportPath(TCHAR *path);
	/**
	 * 報告書ファイル名 取得
	 */
	TCHAR* getReportPath();

	/**
	 * ホットスポット情報 ファイル名 設定
	 */
	void setHotspotInfoPath(TCHAR *path);
	/**
	 * ホットスポット情報 ファイル名 取得
	 */
	TCHAR* getHotspotInfoPath();

	/**
	 * 基本情報 ファイル名 設定
	 */
	void setCustomerInfoPath(TCHAR *path);
	/**
	 * 基本情報 ファイル名 取得
	 */
	TCHAR* getCustomerInfoPath();

	/**
	 * 基本情報 パラメータ 設定
	 */
	void setCustomerInfoParameter(CustomerInfoParameter *param);
	/**
	 * 基本情報 パラメータ 取得
	 */
	CustomerInfoParameter* getCustomerInfoParameter();

	/**
	 * 機器情報 ファイル名 設定
	 */
	void setMachineInfoPath(TCHAR *path);
	/**
	 * 機器情報 ファイル名 取得
	 */
	TCHAR* getMachineInfoPath();

	/**
	 * 機器情報 パラメータ 設定
	 */
	void setMachineInfoParameter(MachineInfoParameter *param);
	/**
	 * 機器情報 パラメータ 取得
	 */
	MachineInfoParameter* getMachineInfoParameter();

	/**
	 * 報告書2用ファイル名設定
	 */
	void setReport2DataPath(TCHAR *path);
	/**
	 * 報告書2用ファイル名取得
	 */
	TCHAR* getReport2DataPath(void);

	/**
	 * WatchOn_Data.txt ファイル名設定
	 */
	void setWatchOnDataInfoPath(TCHAR *path);
	/**
	 * WatchOn_Data.txt ファイル名取得
	 */
	TCHAR* getWatchOnDataInfoPath(void);

	/**
	 * WatchOn_Data.txt の元ファイルをコピー
	 */
	bool copyWatchOnDataFile(TCHAR *filePath);

	/**
	 *　ファイルパスが存在するか確認する
	 */
	bool checkExistFilePath(void);
	bool checkOpenfile(void);
	/**
	 * 存在をチェックするパスを設定
	 */
	void setCheckExistFilePath(TCHAR *path);

	// (2017/6/2YM)追加データファイルパスをセットする処理
	void setAddDataPath(TCHAR *path);

	// (2017/6/2YM)追加データファイルパスを取得する処理
	TCHAR* getAddDataPath();

private:
	// 報告書
	TCHAR reportPath[MAX_PATH];

	// ホットスポット情報
	TCHAR hotspotInfoPath[MAX_PATH];

	// 基本情報
	TCHAR customerInfoPath[MAX_PATH];
	// 基本情報：パラメータ
	CustomerInfoParameter customerInfoParam;

	// 機器情報
	TCHAR machineInfoPath[MAX_PATH];
	// 機器情報：パラメータ
	MachineInfoParameter machineInfoParam;

	// 報告書2用の情報
	TCHAR report2InfoPath[MAX_PATH];

	// WatchOn_Data.txt
	TCHAR watchOnDataInfoPath[MAX_PATH];

	// ファイルパス存在チェック用
	TCHAR checkPath[MAX_PATH];

	// (2017/6/2YM)追加データファイル格納変数を追加
	TCHAR AddDataInfoPath[MAX_PATH];

};

#endif /* CSVDATA_H_ */
