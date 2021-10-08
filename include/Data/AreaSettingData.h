/*
 * AreaSettingData.h
 *
 *  Created on: 2016/03/14
 *      Author: PC-EFFECT-011
 */

#ifndef AREASETTINGDATA_H_
#define AREASETTINGDATA_H_

#include <windows.h>
#include "CommonData.h"
#include "resource.h"

class AreaSettingData {
public:
	AreaSettingData(TCHAR *path);
	~AreaSettingData();

	// 設定データ保存
	bool saveAreaSetting(HotSpotArea *data);

	// 設定データ取得
	bool loadAreaSetting(HotSpotArea *data);

private:
	// 格納フォルダ
	TCHAR *dataPath;
	// パス + ファイル名
	TCHAR *fullFileName;

};

#endif /* AREASETTINGDATA_H_ */
