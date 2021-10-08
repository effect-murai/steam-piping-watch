/*
 * AreaSettingData.cpp
 *
 *  Created on: 2016/03/14
 *      Author: PC-EFFECT-011
 */

#include <stdio.h>
#include "app.h"
#include "MainWindow.h"
#include "AreaSettingData.h"

AreaSettingData::AreaSettingData(TCHAR *path) {
	this->dataPath = path;
	this->fullFileName = new TCHAR[MAX_PATH];
	stprintf(this->fullFileName, TEXT("%s\\%s"), this->dataPath,
			TEXT("Result_AreaSetting.txt"));
}

AreaSettingData::~AreaSettingData() {
	delete this->fullFileName;
}

// 設定データ保存
bool AreaSettingData::saveAreaSetting(HotSpotArea *data) {
	// ファイルを開く
	FILE *pFile = fileOpen(this->fullFileName, TEXT("w"));
	if (pFile == NULL) {
		return false;
	}

	// ファイルに書き込む
	for (HotSpotArea::iterator it = data->begin(); it != data->end(); it++) {
		fprintf(pFile, "%lf,%lf\n", it->x, it->y);
	}

	// ファイルを閉じる
	fclose(pFile);

	return true;
}

// 設定データ取得
bool AreaSettingData::loadAreaSetting(HotSpotArea *data) {
	// ファイルを開く
	FILE *pFile = fileOpen(this->fullFileName, TEXT("r"));
	if (pFile == NULL) {
		return false;
	}

	// ファイルから読み込む
	Vector2D pos;
	while (!feof(pFile)) {
		int ret = fscanf(pFile, "%lf,%lf\n", &pos.x, &pos.y);
		if (ret == 2) {
			data->push_back(pos);
		}
	}

	// ファイルを閉じる
	fclose(pFile);

	// 空データの場合
	if (data->empty()) {
		return false;
	}

	return true;
}
