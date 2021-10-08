/*
 * CalibrationData.cpp
 *
 *  Created on: 2016/01/25
 *      Author: PC-EFFECT-020
 */
#include <Core.h>
#include <windows.h>
#include <stdio.h>
#include <vector>
#include "app.h"
#include "CalibrationData.h"
#include "FileUtils.h"

//------------------------------------------------------------------------------
// CalibrationData class
//------------------------------------------------------------------------------

/**
 * キャリブレーションファイルを読み込む
 * @param[in] path 読み込み対象のフォルダのパス
 */
CalibrationData::CalibrationData(const TCHAR *path) {
	// 値の初期化
	available = false;
	sumCount = 0;
	ZeroMemory(&data, sizeof(GPSPictureInfo));

	// ファイルを開く
	FILE *f;
	std::vector<TCHAR*> files;
	searchFiles(path, TEXT("WatchOn_Calibration*.txt"), files);
	if (files.size() > 0) {
		// 最初に見つかったファイルを読み込む
		//(2020/01/29LEE) Memory Errorに対応するためにマクロを追加
		TCHAR *filePath = allocMemory(TCHAR, MAX_PATH * sizeof(TCHAR));
		stprintf(filePath, TEXT("%s\\%s"), path, files[0]);
		f = fileOpen(filePath, TEXT("r"));
		searchFiles_cleanup(files);
		free(filePath);
		if (f == NULL) {
			return;
		}
	} else {
		// 検索対象のファイルがひとつも見つからなかった場合
		return;
	}

	// 一時変数
	char *lineBuf = new char[1024];
	GPSPictureInfo sum;
	ZeroMemory(&sum, sizeof(GPSPictureInfo));

	while (!feof(f)) {
		memset(lineBuf, 0, 1024);
		fgets(lineBuf, 1024, f);
		GPSPictureInfo inf;

		if (parseSensorInfo(lineBuf, inf) == false) {
			continue;
		}

		if ((inf.gps.gpsFix == GPS_3D_FIX) || (inf.gps.gpsFix == GPS_2D_FIX)) {
			// 3D-Fixの場合のみ有効データとする。
			sum += inf;
			sumCount++;
		}
	}
	fclose(f);

	delete lineBuf;

	if (sumCount != 0) {
		data = sum / sumCount;
	}

	available = true;
	return;
}

/**
 * キャリブレーションデータの解放処理
 */
CalibrationData::~CalibrationData(void) {
	// 値の初期化
	available = false;
	sumCount = 0;
	ZeroMemory(&data, sizeof(GPSPictureInfo));
}

/**
 * キャリブレーションデータの平均値を取得する。
 * @return キャリブレーションデータの平均値
 */
GPSPictureInfo& CalibrationData::getAverage(void) {
	return data;
}

/**
 * キャリブレーションデータの平均GPS高度を取得する。
 * @return 高度
 */
double CalibrationData::getGPSHeight(void) {
	return data.gps.height;
}

/**
 * キャリブレーションデータが有効かどうかを取得する。
 * @return キャリブレーションデータの状態(true:有効/false:無効)
 */
bool CalibrationData::isAvailable(void) {
	return available;
}
