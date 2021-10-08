/*
 * FlightData.cpp
 *
 *  Created on: 2016/01/29
 *      Author: PC-EFFECT-020
 */
#include <Core.h>
#include <windows.h>
#include <stdio.h>
#include <vector>
#include "app.h"
#include "FileUtils.h"
#include "FlightData.h"
#include "surveycalc.h"

//------------------------------------------------------------------------------
// FlightData class functions
//------------------------------------------------------------------------------

/**
 * 飛行データを初期化する。
 * @param path 飛行データの読み込み元ファイルパス
 */
FlightData::FlightData(const TCHAR *path) {
	// 値の初期化
	available = false;
	data.clear();

	// ファイルの読み込み
	FILE *f;
	std::vector<TCHAR*> files;
	searchFiles(path, TEXT("WatchOn_Data*.txt"), files);
	if (files.size() > 0) {
		// 最初に見つかったファイルを読み込む
		TCHAR *filePath = allocMemory(TCHAR, MAX_PATH * sizeof(TCHAR));
		//(2020/01/29LEE) MemoryのErrorに対応するために変換

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
	char *lineBuf = allocMemory(char, 1024);
	// (2020/01/29LEE) MemoryのErrorに対応するために変換

	GPSPictureInfo sum;
	ZeroMemory(&sum, sizeof(GPSPictureInfo));

	int prev_iTOW = 0;
	while (!feof(f)) {
		ZeroMemory(lineBuf, 1024);
		fgets(lineBuf, 1024, f);
		GPSPictureInfo inf;

		if (parseSensorInfo(lineBuf, inf) == false) {
			continue;
		}

		// GPS情報が更新されているかどうかをiTOWで確認する
		if (prev_iTOW == inf.gps.iTOW) {
			// 変わっていなければ取り込んだデータを捨てる
			continue;
		}
		prev_iTOW = inf.gps.iTOW;

		data.push_back(inf);
	}
	fclose(f);

	available = true;
	return;
}

/**
 * 飛行データの削除
 */
FlightData::~FlightData(void) {
	data.clear();
}

/**
 * 飛行データの取得
 * @param id 取得するデータの番号
 */
GPSPictureInfo FlightData::operator [](unsigned int id) {
	return data[id];
}

/**
 * 補正GPSデータの取得
 * @param[out] info GPSデータを格納する変数のポインタ
 * @param[in] time GPSデータを取得する日時
 * @param[in] flightSpeed GPSデータ取得時の平均速度
 * @param[in] points 補正に使用する前後の測定ポイント数
 * @return timeに指定した日時と一致するデータが存在する場合はtrueを返す。
 * @todo 速度による補正
 */
bool FlightData::getCorrectedGPSData(GPSPictureInfo *info, struct tm *time,
		double flightSpeed, int points) {
	ZeroMemory(info, sizeof(GPSPictureInfo));
	const time_t baseTime = mktime(time);
	int basePoint = -1;
	for (unsigned int i = 0; i < data.size(); i++) {
		const time_t tm = mktime(&data[i].time);
		if (baseTime == tm) {
			// 指定時刻と等しいデータを検索する。
			basePoint = i;
			break;
		}
	}

	if (basePoint == -1) {
		// 対象が見つからなかった
		return false;
	}

	// 平均化開始・終了点の番号を取得
	int startPoint = basePoint - points;
	unsigned int endPoint = basePoint + points;

	// 開始点が0を下回った場合は0にする
	if (startPoint < 0) {
		startPoint = 0;
	}

	// 終了点がsizeを上回った場合はsizeにする
	if (endPoint > data.size()) {
		endPoint = data.size();
	}

	const double baseLatitude = data[basePoint].gps.latitude;
	const double baseLongitude = data[basePoint].gps.longitude;

	// 合計値を求める
	int count = 0;
	for (unsigned int i = startPoint; i < endPoint; i++) {
		// 飛行速度が設定されている場合は
		// 50%以上の誤差があるデータを除外する
		if (flightSpeed > 0) {
			double dx = 0, dy = 0;
			if (i != (unsigned int) basePoint) {
				ll2xy(data[i].gps.latitude, data[i].gps.longitude, baseLatitude,
						baseLongitude, &dx, &dy);
			}
			const double distance = sqrt(pow(dx, 2) + pow(dy, 2));
			const double dt = fabs(
					difftime(mktime(&data[basePoint].time),
							mktime(&data[i].time)));
			const double velocity = distance / dt;
			if (fabs(velocity - flightSpeed) > flightSpeed * 0.5) {
				continue;
			}
		}

		// 対象のデータを加算する
		info += data[i];
		count++;
	}

	// 平均値を求める
	if (count > 1) {
		info->accel.x /= count;
		info->accel.y /= count;
		info->accel.z /= count;
		info->gyro.x /= count;
		info->gyro.y /= count;
		info->gyro.z /= count;
		info->mag.x /= count;
		info->mag.y /= count;
		info->mag.z /= count;
		info->gps.latitude /= count;
		info->gps.longitude /= count;
		info->gps.height /= count;
	}

	return true;
}

/**
 * 飛行データからGPS方位を求める。
 * @param time GPS方位を求める基準時刻
 */
double FlightData::getGPSCardinalDirection(struct tm *time) {
	int id = -1;
	const time_t baseTime = mktime(time);

	for (unsigned int i = 0; i < data.size(); i++) {
		const time_t tm = mktime(&data[i].time);
		if (baseTime == tm) {
			// 指定時刻と等しいデータを検索する。
			id = i;
		}
	}

	if (id == -1) {
		// 対象が見つからなかった
		return 0;
	}

	double dx = 0, dy = 0;
	int count = 0;

	const int iMin = (id - 2 >= 0) ? id - 2 : 0;
	const int iMax =
			(id + 2 < (int) data.size()) ? id + 2 : (int) data.size() - 1;

	for (int i = iMin; i < iMax; i++) {
		dx += data[i + 1].gps.longitude - data[i].gps.longitude;
		dy += data[i + 1].gps.latitude - data[i].gps.latitude;
		count++;
	}

	if (count > 0) {
		dx /= count;
		dy /= count;
	}

	const double angle = atan2(dx, dy);
	if (angle < 0) {
		// 角度が負の値の場合は2πを加算し正の値のする
		return M_PI * 2 + angle;
	}
	return angle;
}

/**
 * 指定したパスの飛行データが利用可能かどうかを取得する。
 * @return true:利用可能/false:利用不可
 */
bool FlightData::isAvailable(void) {
	return available;
}

/**
 * 取得した飛行データの総数を取得する。
 * @return 飛行データ数
 */
int FlightData::getCount(void) {
	return data.size();
}

