/*
 * ResultData.cpp
 *
 *  Created on: 2016/01/27
 *      Author: PC-EFFECT-020
 */

#include <Core.h>
#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>
#include <vector>
#include <queue>
#include <math.h>
#include "app.h"
#include "FileUtils.h"
#include "Graphics.h"
#include "surveycalc.h"
#include "PanelDetector.h"
#include "resource.h"
#include "ResultData.h"
#include "HotspotLinker.h"
#include "MainWindow.h"

#include "picojson.h"
#include <iostream>
#include <fstream>
#include <exception>
#include <map>
#include "Mutex.h"
#include "PanelInfo.h"
#include <string>

#include "debug.h"
#include "tiff.h"

#ifdef UNICODE
typedef std::wstring String;
#else
typedef std::string String;
#endif

extern MainWindow *mainWindow;
#define ABS(X) ((X) < 0 ? -(X) : (X))
#define EPSILON 0.000001
#include "Functions.h"
#include "StringUtils.h"
#include "Matrix.h"
#include <opencv2/opencv.hpp>

extern HotspotLinker *hotspotLinker;
extern MainWindow *mainWindow;

extern PanelData *panelData;

class ParseError: public std::exception {
	const char* what() {
		return "Parse Error";
	}
};

inline void setDefaultCameraInfo(CameraInfo *cameraInfo) {
	cameraInfo->offset.x = 0;
	cameraInfo->offset.y = 0;
	cameraInfo->ratio.x = 1.76;	// 拡大率の初期値
	cameraInfo->ratio.y = 1.76;	// 拡大率の初期値
	cameraInfo->direction = 0;
	cameraInfo->datatype = 0;
}

inline bool checkCameraModel(const TiffInfo *info, const char *manufacturer,
		const char *model) {
	return strcmp(manufacturer, info->manufacturer) == 0
			&& strcmp(model, info->model) == 0;
}

//------------------------------------------------------------------------------
// Compiler Options
//------------------------------------------------------------------------------
//#define RESULT_LOGFILE

//------------------------------------------------------------------------------
// Global Functions
//------------------------------------------------------------------------------

#define checkOutOfRange(list, id) (((id) < 0) || ((size_t)(id) >= (list).size()))

// 各種センサー情報の加算
GPSPictureInfo& operator+=(GPSPictureInfo &x, const GPSPictureInfo &y) {
	x.accel.x += y.accel.x;
	x.accel.y += y.accel.y;
	x.accel.z += y.accel.z;
	x.gyro.x += y.gyro.x;
	x.gyro.y += y.gyro.y;
	x.gyro.z += y.gyro.z;
	x.mag.x += y.mag.x;
	x.mag.y += y.mag.y;
	x.mag.z += y.mag.z;
	x.gps.latitude += y.gps.latitude;
	x.gps.longitude += y.gps.longitude;
	x.gps.height += y.gps.height;
	return x;
}

GPSPictureInfo* operator+=(GPSPictureInfo *const x, const GPSPictureInfo &y) {
	*x += y;
	return x;
}

// 各種センサー情報の除算
GPSPictureInfo operator/(const GPSPictureInfo x, const int y) {
	GPSPictureInfo ret;
	memcpy(&ret, &x, sizeof(GPSPictureInfo));
	ret.accel.x = x.accel.x / y;
	ret.accel.y = x.accel.y / y;
	ret.accel.z = x.accel.z / y;
	ret.gyro.x = x.gyro.x / y;
	ret.gyro.y = x.gyro.y / y;
	ret.gyro.z = x.gyro.z / y;
	ret.mag.x = x.mag.x / y;
	ret.mag.y = x.mag.y / y;
	ret.mag.z = x.mag.z / y;
	ret.gps.latitude = x.gps.latitude / y;
	ret.gps.longitude = x.gps.longitude / y;
	ret.gps.height = x.gps.height / y;
	return ret;
}

/**
 * センサー情報ファイルの内容を解析する。
 * @param[in] lineBuf 解析対象の文字列バッファへのポインタ。文字列はNULL文字で終わる必要がある。
 * @param[out,ref] inf センサー情報を保存する変数
 * @return 解析の成否(true:成功/false:失敗)
 */
bool parseSensorInfo(const char *buffer, GPSPictureInfo &inf) {
	double iTOW, second;

	const int bufferLength = strlen(buffer);
	char *status = allocMemory(char, bufferLength + 1);
	// (2020/01/29LEE) 出来ない部分理由不明

	ZeroMemory(status, bufferLength + 1);

	ZeroMemory(&inf, sizeof(GPSPictureInfo));

	// ※注意点(ファイル内容はUTF-8)
	int count = sscanf(buffer, "%d/%d/%d"
			"%d:%d:%lg -- "
			"Acc: %lg %lg %lg, "
			"Gyr: %lg %lg %lg, "
			"Mag: %lg %lg %lg, "
			"iTOW: %lg, "
			"Latitude: %lf, Longitude: %lf, Height: %lf, "
			"gpsFixOk: %d, %s\n", &inf.time.tm_year, &inf.time.tm_mon,
			&inf.time.tm_mday, &inf.time.tm_hour, &inf.time.tm_min, &second,
			&inf.accel.x, &inf.accel.y, &inf.accel.z, &inf.gyro.x, &inf.gyro.y,
			&inf.gyro.z, &inf.mag.x, &inf.mag.y, &inf.mag.z, &iTOW,
			&inf.gps.latitude, &inf.gps.longitude, &inf.gps.height,
			&inf.gps.gpsOk, status);

	// Time of Week(iTOW)を整数に変換する
	inf.gps.iTOW = (int) iTOW;

	// マイクロ秒単位で取得する
	inf.time.tm_sec = floor(second);
	inf.tm_usec = (int) ((second - inf.time.tm_sec) * 1e6);

	// 行末の文字列
	if (count == 21) {
		if ((strcmp(status, "3D-fix") == 0)
				&& (strcmp(status, "3D-Fix") == 0)) {
			inf.gps.gpsFix = GPS_3D_FIX;
		} else if ((strcmp(status, "2D-fix") == 0)
				&& (strcmp(status, "2D-Fix") == 0)) {
			inf.gps.gpsFix = GPS_2D_FIX;
		}
	}

	free(status);

	// 指定したデータ数に達していない場合
	if (count < 20) {
		if (count != 6) {
			// 日付データ以外が入っていた/日付データがない場合
			ZeroMemory(&inf, sizeof(GPSPictureInfo));
		} else {
			// 日付データのみが入っていた場合
			// 年を1900年起算に変更
			inf.time.tm_year -= 1900;
			// 月を0起算に変更
			inf.time.tm_mon -= 1;
		}
		return false;
	}

	// 年を1900年起算に変更
	inf.time.tm_year -= 1900;
	// 月を0起算に変更
	inf.time.tm_mon -= 1;

	return true;
}

/**
 * ファイルから2バイト読み込む
 * @param file 読み込み対象のファイルポインタ
 * @return 読み込んだデータ
 */
inline int fgetw(FILE *file) {
	int d1 = fgetc(file);
	int d2 = fgetc(file);
	if (d1 == EOF || d2 == EOF) {
		throw std::exception();
		return EOF;
	}
	return (d1 | (d2 << 8));
}

/**
 * ファイルから2バイト(Big endian)読み込む
 * @param file 読み込み対象のファイルポインタ
 * @return 読み込んだデータ
 */
inline int fgetw_be(FILE *file) {
	int d1 = fgetc(file);
	int d2 = fgetc(file);
	if (d1 == EOF || d2 == EOF) {
		return EOF;
	}
	return ((d1 << 8) | d2);
}

/**
 * 指定したファイルに含まれる温度データを取得する@n
 * 先行リリース版用
 * @param fileName データを取得するファイル名
 * @param width 温度データの幅
 * @param height 温度データの高さ
 * @param time 温度データを取得した日時
 */
inline float* getTempDataForPreRelease(LPCTSTR fileName, int *width,
		int *height, struct tm *time) {
	// サイズを0にする
	*width = 0;
	*height = 0;

	// バイナリデータなのでbを指定する
	FILE *file = fileOpen(fileName, TEXT("rb"));
	if (file == NULL) {
		return NULL;
	}

	float *tempData = NULL;
	int pos = 0;

	// 温度データの検索
	int marker = 0;
	int length = 0;
	double dataLevel = 0;
	double dataSens = 0;
	bool loop = true;
	bool completed = false;
	int integerPart, decimalPart;
	while (loop) {
		// マーカー(ビッグエンディアン)
		marker = fgetw_be(file);
		if (marker == EOF) {
			loop = false;
			break;
		}
		switch (marker) {
		case Graphics::Jpeg::START_OF_IMAGE:
			// SOI
			break;

		case Graphics::Jpeg::END_OF_IMAGE:
			// EOI
			break;

		case Graphics::Jpeg::EXIF:
			// Exif
			length = fgetw_be(file);
			if (length == EOF) {
				loop = false;
				break;
			}
			// @todo カメラ情報の取得
			if (length != 2) {
				fseek(file, length - 2, SEEK_CUR);
			}
			break;

		case Graphics::Jpeg::EXTENDED_DATA_1:
			// Avio Extended Data 1
			length = fgetw_be(file);
			if (length == EOF) {
				loop = false;
				break;
			}

			// 54バイト進める
			// @todo APP10データの取得
			fseek(file, 54, SEEK_CUR);

			// 150バイト進める
			// @todo 設定条件の取得
			fseek(file, 128, SEEK_CUR);
			// 幅と高さを取得する
			*width = fgetw(file);
			if (*width == EOF) {
				loop = false;
				break;
			}
			*height = fgetw(file);
			if (*height == EOF) {
				loop = false;
				break;
			}
			// 温度格納用のデータ領域を確保する
			tempData = new float[*width * *height];
			// 150-128-4=18バイト進める
			fseek(file, 18, SEEK_CUR);

			// Data Levelの取得
			// リトルエンディアンなので注意
			decimalPart = fgetw(file);
			if (decimalPart == EOF) {
				loop = false;
				break;
			}
			integerPart = fgetw(file);
			if (integerPart == EOF) {
				loop = false;
				break;
			}
			dataLevel = integerPart + (double) decimalPart / 65536;

			// Data Sensの取得
			// リトルエンディアンなので注意
			decimalPart = fgetw(file);
			if (decimalPart == EOF) {
				loop = false;
				break;
			}
			integerPart = fgetw(file);
			if (integerPart == EOF) {
				loop = false;
				break;
			}
			dataSens = integerPart + (double) decimalPart / 65536;

			// 300バイト目までのデータは飛ばす
			fseek(file, 142, SEEK_CUR);

			// 日付データ(12バイト)を読み込む
			if (time != NULL) {
				ZeroMemory(time, sizeof(struct tm));
				char date[12 + 1];
				fread(date, 12, 1, file);
				date[12] = 0;
				int yearLen = 4;

				// 年(4桁)
				time->tm_year = 0;
				for (int i = 0; i < yearLen; i++) {
					int y = date[i];
					if (y == EOF) {
						loop = false;
						break;
					}
					y -= '0';
					time->tm_year = time->tm_year * 10 + y;
				}
				if (loop == false) {
					break;
				}
				// 1900年起算に変換
				time->tm_year -= 1900;

				// 月(2桁)
				time->tm_mon = 0;
				for (int i = 0; i < 2; i++) {
					int m = date[i + yearLen];
					if (m == EOF) {
						loop = false;
						break;
					}
					m -= '0';
					time->tm_mon = time->tm_mon * 10 + m;
				}
				if (loop == false) {
					break;
				}
				// 0起算に変換
				time->tm_mon -= 1;

				// 日(2桁)
				time->tm_mday = 0;
				for (int i = 0; i < 2; i++) {
					int d = date[i + yearLen + 2];
					if (d == EOF) {
						loop = false;
						break;
					}
					d -= '0';
					time->tm_mday = time->tm_mday * 10 + d;
				}
				if (loop == false) {
					break;
				}

				// 時刻データ(8バイト)を読み込む
				fread(date, 8, 1, file);
				date[8] = 0;

				// 時(2桁)
				time->tm_hour = 0;
				for (int i = 0; i < 2; i++) {
					int h = date[i];
					if (h == EOF) {
						loop = false;
						break;
					}
					h -= '0';
					time->tm_hour = time->tm_hour * 10 + h;
				}
				if (loop == false) {
					break;
				}

				// 分(2桁)
				time->tm_min = 0;
				for (int i = 0; i < 2; i++) {
					int m = date[i + 2];
					if (m == EOF) {
						loop = false;
						break;
					}
					m -= '0';
					time->tm_min = time->tm_min * 10 + m;
				}
				if (loop == false) {
					break;
				}

				// 秒(2桁)
				time->tm_sec = 0;
				for (int i = 0; i < 2; i++) {
					int s = date[i + 4];
					if (s == EOF) {
						loop = false;
						break;
					}
					s -= '0';
					time->tm_sec = time->tm_sec * 10 + s;
				}
				if (loop == false) {
					break;
				}
			} else {
				// 日付データを飛ばす
				fseek(file, 20, SEEK_CUR);
			}

			// 残りのデータは飛ばす
			fseek(file, length - (300 + 54 + 12 + 8) - 2, SEEK_CUR);
			break;

		case Graphics::Jpeg::EXTENDED_DATA_2:
			// Avio Extended Data 2
			length = fgetw_be(file);
			if (length == EOF) {
				loop = false;
				break;
			}

			// 54バイト進める
			// @todo 設定条件の取得
			fseek(file, 54, SEEK_CUR);

			for (int i = 0; i < 19200; i++) {
				int data = fgetw(file);
				if (data == EOF) {
					loop = false;
					break;
				}
				// 符号付整数に変換する
				if (data & 0x8000) {
					data = (int) (data & 0x7fff) - (int) 0x8000;
				}
				// 温度の計算
				tempData[pos] = (data * dataSens + dataLevel);
				pos++;
			}

			if (pos == *width * *height) {
				// 読み込みが完了した
				completed = true;
				loop = false;
			}
			break;

		default:
			loop = true;
			break;
		}
	}

	if (!completed) {
		*width = 0;
		*height = 0;
		delete tempData;
		return NULL;
	}
	fclose(file);

	return tempData;
}

/**
 * 指定したファイルに含まれる温度データを取得する@n
 * (正式リリース版用)
 * @param fileName データを取得するファイル名
 * @param width 温度データの幅
 * @param height 温度データの高さ
 * @param time 温度データを取得した日時
 */
inline float* getTempDataV1(LPCTSTR fileName, int *width, int *height,
		struct tm *time) {
	// サイズを0にする
	*width = 0;
	*height = 0;

	// バイナリデータなのでbを指定する
	FILE *file = fileOpen(fileName, TEXT("rb"));
	if (file == NULL) {
		return NULL;
	}
	fread(width, 1, sizeof(int), file);
	fread(height, 1, sizeof(int), file);

	float *data = new float[*width * *height];

	for (int y = 0; y < *height; y++) {
		float *scanLine = &data[y * *width];
		for (int x = 0; x < *width; x++) {
			short int _data = 0;
			fread(&_data, 1, sizeof(short), file);
			if ((_data) > 32768) {
				scanLine[x] = ((_data) - 65536) / 100.0;
			} else {
				scanLine[x] = ((_data) / 100.0);
			}
		}
	}

	fclose(file);

	return data;
}

/**
 * 指定したファイルの温度データを作成した日時を取得する
 * @param fileName ファイル名
 * @param time 温度データの作成日時
 * @return ファイル読み込みの成否(true:成功/false:失敗)
 */
inline bool getFileTime(LPCTSTR fileName, struct tm *time, int version) {
	int width, height;
	if (version < 1) {
		// ファイル時刻を取得
		float *tempData = ::getTempDataForPreRelease(fileName, &width, &height,
				time);
		if (tempData != NULL) {
			delete tempData;
			return true;
		}
	} else {
		// ファイル時刻を取得
		::getFileTimeStd(fileName, NULL, NULL, time);
	}
	return false;
}

/**
 * 実行ファイルのパスを取得
 * @return ファイル読み込みの成否(true:成功/false:失敗)
 */
String getModulePath(void) {

	// 実行ファイルパスを取得する
	String path;
	DWORD copied = 0;
	path.resize(MAX_PATH);
	do {
		path.resize(path.size() * 2);
		copied = GetModuleFileName(NULL, (LPTSTR) path.data(), path.size());
	} while (copied >= path.size());
	path.resize(copied);

	// 実行ファイルパス、ドライブ名、ディレクトリ名、ファイル名、拡張子
	String modulePath;
	TCHAR drive[copied + 1], dir[copied + 1], fname[copied + 1],
			ext[copied + 1];

	_wsplitpath_s(path.c_str(), drive, (copied + 1), dir, (copied + 1), fname,
			(copied + 1), ext, (copied + 1));
	// ドライブとディレクトリ名を結合して実行ファイルパスとする
	modulePath = String(drive) + String(dir);
	// 文末の\\を削除
	modulePath.erase(modulePath.length() - 1);

	return modulePath;
}

//------------------------------------------------------------------------------
// ResultData class
//------------------------------------------------------------------------------

/**
 * センサー情報と画像ファイルをリンクさせる
 * @param path センサー情報が記録されたフォルダのパス
 */
ResultData::ResultData(const TCHAR *path) {

	std::vector<TCHAR*> files_vb;
	std::vector<TCHAR*> files_ir;
	// データを初期化する
	Errornumcheck = true;
	available = false;
	data.clear();
	hotspots.clear();
	posData.clear();
	readpicturedata.clear();
	picturedata.clear(); // (2019/11/01LEE)追加。
	stringBuffer = NULL;
	dataPath = NULL;
	cachePath = NULL;
	exePath = NULL;
	calibrationData = new CalibrationData(path);
	flightDetail = new FlightData(path);
	disableCorrection();
	setStringBufferSize(MAX_PATH > 1024 ? MAX_PATH : 1024);
	baseDirection = 0;
	dataVersion = 1;

	// ファイルを開く
	FILE *f;
	std::vector<TCHAR*> files;
	searchFiles(path, TEXT("WatchOn_Result*.txt"), files);
	if (files.size() > 0) {
		// 最初に見つかったファイルを読み込む
		TCHAR *filePath = getStringBuffer();
		stprintf(filePath, TEXT("%s\\%s"), path, files[0]);
		f = fileOpen(filePath, TEXT("r"));
		searchFiles_cleanup(files);
		if (f == NULL) {
			// 開けなかった場合
			return;
		}
	} else {
		// 検索対象のファイルがひとつも見つからなかった場合
		return;
	}

	// 指定されたデータパスを保存する
	size_t pathLen = lstrlen(path);
	dataPath = allocMemory(TCHAR, (pathLen + 1) * sizeof(TCHAR));
	//(2020/01/29LEE) MemoryのErrorに対応するために変換

	ZeroMemory(dataPath, (pathLen + 1) * sizeof(TCHAR));
	memcpy(dataPath, path, pathLen * sizeof(TCHAR));

	// 作業フォルダパスの設定
	TCHAR *cachePathName = Resource::getString(IDS_CACHE_PATH);
	size_t cachePathLen = (pathLen + 2 + lstrlen(cachePathName))
			* sizeof(TCHAR);
	cachePath = (TCHAR*) malloc(cachePathLen);
	ZeroMemory(cachePath, cachePathLen);
	stprintf(cachePath, TEXT("%s\\%s"), dataPath, cachePathName);
	if (!(::PathFileExists(cachePath))) {
		_wmkdir(cachePath);
	}

	// 実行ファイルパスの設定
	String tmpPath = getModulePath();
	size_t exePathLen = (tmpPath.length() + 1) * sizeof(TCHAR);
	exePath = (TCHAR*) malloc(exePathLen);
	ZeroMemory(exePath, exePathLen);
	stprintf(exePath, TEXT("%s"), tmpPath.c_str());

	// 赤外データの一覧を取得する
	searchFiles(path, TEXT("t*ir.dat"), files_ir);
	if (files_ir.empty()) {
		// 赤外データがない場合は古いデータ形式(PhaseI)
		dataVersion = 0;

		// 赤外画像の一覧を取得する
		searchFiles(path, TEXT("t*ir.jpg"), files_ir);

		// 不具合対策
		if (files_ir.empty()) {
			// *ir.jpgがない場合は*mx.jpgを使用する
			searchFiles(path, TEXT("*mx.jpg"), files_ir);
		}
	}

	// 可視画像の一覧を取得する
	searchFiles(path, TEXT("t*vb.jpg"), files_vb);

	// パネル検出状態を更新する
	updatePanelDetected();

	// ファイルを読み込む
	char *lineBuf = (char*) stringBuffer;
	int lineNo = 0;
	time_t lastFileTime = 0, lastDataTime = 0;
	time_t creationTime = 0;
	int timeZone = 0;
	bool timeZoneBug = false;

	while (!feof(f)) {
		// ファイルから1行程度読み込む
		ZeroMemory(lineBuf, stringBufferSize);
		fgets(lineBuf, stringBufferSize, f);

		GPSPictureInfo inf;

		// センサー情報を解析する
		if (parseSensorInfo(lineBuf, inf) == false) {
			if (lineNo == 0) {
				// 最初の行は作成日時
				creationTime = mktime(&inf.time);
			}
			continue;
		}

		if (files_ir.size() > (unsigned int) lineNo) {
			if (dataVersion < 1) {
				// 対象ファイルとデータの時刻差を求める。
				struct tm modTime;
				TCHAR *fullPath = getStringBuffer();
				stprintf(fullPath, TEXT("%s\\%s"), path, files_ir[lineNo]);

				if (getFileTime(fullPath, &modTime, dataVersion) == false) {
					continue;
				}
				time_t fileTime = mktime(&modTime);
				time_t dataTime = mktime(&inf.time);

				double fileDiff, dataDiff;
				if (lineNo == 0) {
					fileDiff = 0;
					dataDiff = 0;
				} else {
					fileDiff = difftime(fileTime, lastFileTime);
					dataDiff = difftime(dataTime, lastDataTime);
				}
				// データ時刻の増分と撮影ファイル時刻の増分が3秒未満だった場合に画像とデータを関連付ける
				if (fabs(fileDiff - dataDiff) < 3) {
					lastFileTime = fileTime;
					lastDataTime = dataTime;

					// Jpegファイル名を設定
					memcpy(inf.fileName_8, files_vb[lineNo],
							sizeof(inf.fileName_8));
					memcpy(inf.fileName_ir_8, files_ir[lineNo],
							sizeof(inf.fileName_ir_8));

					data.push_back(inf);

					lineNo++;
				}
			} else {
				// 最初のデータの読み込み時にタイムゾーンを判定する
				if (lineNo == 0) {
					// 作成日時との時間差からタイムゾーンを判定する
					time_t dataTime = mktime(&inf.time);
					double diffTime = difftime(creationTime, dataTime);
					timeZone = (int) floor(diffTime / (60 * 30) + 0.5);
					if (timeZone <= -48 || timeZone >= 48) {
						// タイムゾーンが異常値の場合は0にする
						timeZone = 0;
						// タイムゾーン異常フラグをセットする
						timeZoneBug = true;
					}
				}

				// Jpegファイル名を設定
				memcpy(inf.fileName_8, files_vb[lineNo],
						sizeof(inf.fileName_8));
				memcpy(inf.fileName_ir_8, files_ir[lineNo],
						sizeof(inf.fileName_ir_8));
				// 輪郭画像ファイル名設定処理
				TCHAR filePN[20];
				_swprintf(filePN, TEXT("T%05dPN.jpg"), lineNo + 1);
				memcpy(inf.fileName_pn_8, filePN, sizeof(inf.fileName_pn_8));
				// タイムゾーンが設定されている場合は地方時に変換する
				if (timeZone != 0) {
					time_t localTime = mktime(&inf.time) + timeZone * 60 * 30;
					struct tm time;
					localtime_s(&time, &localTime);
					memcpy(&inf.time, &time, sizeof(struct tm));
				}

				data.push_back(inf);

				lineNo++;
			}
		}
	}
	fclose(f);

	// 後片付け
	searchFiles_cleanup(files_ir);
	searchFiles_cleanup(files_vb);

	// 端の座標を求める
	if (!data.empty()) {
		gpsRight = gpsLeft = data[0].gps.longitude;
		gpsBottom = gpsTop = data[0].gps.latitude;
		for (unsigned int i = 1; i < data.size(); i++) {
			// 左右端の経度を設定する
			if (gpsRight < data[i].gps.longitude) {
				gpsRight = data[i].gps.longitude;
			} else if (gpsLeft > data[i].gps.longitude) {
				gpsLeft = data[i].gps.longitude;
			}

			// 上下端の緯度を設定する
			if (gpsTop < data[i].gps.latitude) {
				gpsTop = data[i].gps.latitude;
			} else if (gpsBottom > data[i].gps.latitude) {
				gpsBottom = data[i].gps.latitude;
			}

		}

		// ホットスポット格納用領域を確保する
		hotspots.resize(data.size());
		// 内部位置情報用の領域を確保する
		posData.resize(data.size());
		// (2019/11/01LEE) 追加。
		picturedata.resize(data.size());
		readpicturedata.resize(data.size());

		// 可視光画像の全体設定を初期化する
		setDefaultCameraInfo(&resetdata);
		// デフォルト値を設定する
		setDefaultCameraInfo(&cameraInfo);

		if (!loadCameraInfo(getCameraInfoFileName())) {
			// 撮影方位を初期化
			baseDirection = 0;

			// 可視光画像の個別設定を初期化する
			for (int i = 0; i < getDataCount(); i++) {
				setDefaultCameraInfo(&picturedata[i]);
			}
		}

		// 可視画像サイズを読み込む
		Gdiplus::Bitmap *image;
		image = new Gdiplus::Bitmap(getVisibleFilePath(0));
		pictureSize.visible.width = image->GetWidth(); // (2019/10/24LEE) GetWidth() => GetWidth()*0.8 に変更。
		pictureSize.visible.height = image->GetHeight(); // (2019/10/24LEE) GetWidth() => GetWidth()*0.8 に変更。
		delete image;

		// 温度データのサイズを読み込む
		int width, height;
		float *tempData = getTempData(0, &width, &height, NULL);
		delete tempData;
		pictureSize.infrared.width = width;
		pictureSize.infrared.height = height;

		// 温度範囲を取得する
		//　(2016/12/20YM)*全赤外線画像中の最高温度(maxTemp)，最低温度(minTemp)，平均温度(aveTemp)を求める
		getTempRange();

		// PhaseIデータをPhaseII用に変換する
#ifdef CONVERT_PHASE1_DATA_TO_PHASE2_DATA
		if (dataVersion < 1) 
		{
			TCHAR backupPath[lstrlen(dataPath) + 9];
			stprintf(backupPath, TEXT("%s\\.backup"), dataPath);
			if (!(::PathFileExists(backupPath))) 
			{
				_wmkdir(backupPath);
				SetFileAttributes(backupPath, FILE_ATTRIBUTE_HIDDEN);
			}
			for (unsigned int id = 0; id < data.size(); id++) 
			{
				TCHAR* dataFileName;
				getInfraredDataPath(id, &dataFileName);
				FILE* file = fileOpen(dataFileName, TEXT("wb"));
				delete dataFileName;
				if (file == NULL) 
				{
					continue;
				}
				int tempMapWidth, tempMapHeight;
				float* tempMap = getTempData(id, &tempMapWidth, &tempMapHeight, NULL);
				fwrite(&tempMapWidth, 4, 1, file);
				fwrite(&tempMapHeight, 4, 1, file);
				for (int y = 0; y < tempMapHeight; y++) 
				{
					float* scanLine = &tempMap[y * tempMapWidth];
					for (int x = 0; x < tempMapWidth; x++) 
					{
						short temp = static_cast<short>(scanLine[x] * 100);
						fwrite(&temp, 2, 1, file);
					}
				}
				fclose(file);
				TCHAR* origFilePath;
				TCHAR backupFilePath[lstrlen(backupPath) + 13];
				getFilePath(id, this->INFRARED_IMAGE, &origFilePath);
				TCHAR* fileName = origFilePath + lstrlen(dataPath);
				stprintf(backupFilePath, TEXT("%s%s"), backupPath, fileName);
				MoveFile(origFilePath, backupFilePath);
				delete origFilePath;
				delete tempMap;
			}
			dataVersion = 1;
		}
#endif

		// 飛行記録を保存する
		if (timeZoneBug || !existFile(this->getFlightDataFileName())) {
			// ファイルが存在する場合は作成しない
			// (タイムゾーンバグがある場合は更新する)
			saveFlightData();
		}

		// データを有効にする
		available = true;
		piccount = 0;
		picnumbering = 0;
		memset(errorpicnum, 0, 1000 * sizeof(int));

		//(2020/01/15LEE) GPS DATA 間違う場合を個別にCheck
		for (std::vector<GPSPictureInfo>::iterator item = data.begin();
				item != data.end(); item++) {
			if (item->gps.latitude <= 0) {
				Errornumcheck = false;
				errorpicnum[piccount] = picnumbering + 1;
				piccount++;
			} else if (item->gps.longitude <= 0) {
				Errornumcheck = false;
				errorpicnum[piccount] = picnumbering + 1;
				piccount++;
			} else if (item->gps.height <= 0) {
				Errornumcheck = false;
				errorpicnum[piccount] = picnumbering + 1;
				piccount++;
			}
			picnumbering++;
		}
	} else {
		gpsRight = gpsLeft = gpsTop = gpsBottom = 0;
		pictureSize.infrared.width = 0;
		pictureSize.infrared.height = 0;
		pictureSize.visible.width = 0;
		pictureSize.visible.height = 0;

		// 画像のないデータは無効にする
		available = false;
		Errornumcheck = false;
		memset(errorpicnum, 0, 1000 * sizeof(int));
	}
}

/**
 * 結果データの解放処理
 */
ResultData::~ResultData(void) {
	for (int i = 0; i < getDataCount(); i++) {
		if (hotspots[i].size() != 0) {
			hotspots[i].clear();
		}
	}
	data.clear();
	hotspots.clear();
	if (stringBuffer != NULL) {
		delete stringBuffer;
		stringBuffer = NULL;
	}
	delete calibrationData;
	delete flightDetail;
	if (dataPath != NULL) {
		free(dataPath);
		dataPath = NULL;
	}
	if (cachePath != NULL) {
		free(cachePath);
		cachePath = NULL;
	}
	if (exePath != NULL) {
		free(exePath);
		exePath = NULL;
	}
}

/**
 * GPS高度を取得する。
 * @param[in] id GPSデータ番号
 */
double ResultData::getGPSHeight(unsigned int id) {
	double height = data[id].gps.height - calibrationData->getGPSHeight();

	if (height < 0.0) {
		return 0.0;
	}
	return height;
}

/**
 * 日付文字列を取得する。
 * @param[in] date 時刻
 * @return 日付文字列へのポインタ
 */
TCHAR* ResultData::getDateString(struct tm *date) {
	TCHAR *str = getStringBuffer();
	stprintf(str, TEXT("%d/%d/%d %d:%d:%d"), date->tm_year + 1900,
			date->tm_mon + 1, date->tm_mday, date->tm_hour, date->tm_min,
			date->tm_sec);
	return str;
}

/**
 * 現在時刻の日付文字列を取得する。@n
 * 内部バッファを使用するため、スレッドセーフではない。
 * @return 内部文字列バッファへのポインタ
 */
TCHAR* ResultData::getCurrentDateString(void) {
	struct tm local;
	time_t currentTime;
	currentTime = time(NULL);
	localtime_s(&local, &currentTime);
	return getDateString(&local);
}

/**
 * 最初の撮影時刻の日付文字列を取得する。@n
 * 内部バッファを使用するため、スレッドセーフではない。
 * @return 内部文字列バッファへのポインタ
 */
TCHAR* ResultData::getFirstDateString(void) {
	if (data.empty()) {
		return getCurrentDateString();
	}
	return getDateString(&data[0].time);
}

/**
 * 最後の撮影時刻の日付文字列を取得する。@n
 * 内部バッファを使用するため、スレッドセーフではない。
 * @return 内部文字列バッファへのポインタ
 */
TCHAR* ResultData::getLastDateString(void) {
	if (data.empty()) {
		return getCurrentDateString();
	}
	return getDateString(&data[data.size() - 1].time);
}

/**
 * 総データ数の文字列を取得する。@n
 * 内部バッファを使用するため、スレッドセーフではない。
 * @return 内部文字列バッファへのポインタ
 */
TCHAR* ResultData::getDataCountString(void) {
	TCHAR *str = getStringBuffer();
	stprintf(str, TEXT("%d%s"), data.size(),
			Resource::getString(IDS_PICTURE_COUNT_SUFFIX));
	return str;
}

/**
 * ホットスポット総数のメッセージボックスを表示する
 */
TCHAR* ResultData::getHotspotSumString(void) {
	int ptSum = getHotspotSum();
	TCHAR *str = getStringBuffer();
	stprintf(str, TEXT("%d"), ptSum);
	return str;
}

TCHAR* ResultData::getDataPath(void) {
	TCHAR *path = dataPath;
	return path;
}

TCHAR* ResultData::getCachePath(void) {
	TCHAR *path = cachePath;
	return path;
}

TCHAR* ResultData::getExePath(void) {
	TCHAR *path = exePath;
	return path;
}

/**
 * 撮影データのバージョンを取得する。
 * @return 撮影データバージョン
 */
int ResultData::getDataVersion(void) {
	return dataVersion;
}

/**
 * 撮影データ総数を取得する。
 * @return 撮影データ総数
 */
int ResultData::getDataCount(void) {
	return data.size();
}

/**
 * 指定した番号のデータの画像ファイルパスを取得する。@n
 * 内部バッファを使用するため、スレッドセーフではない。
 * @param id 取得するデータの番号
 * @param type 画像タイプ(VISIBLE_LIGHT_IMAGE:可視画像/INFRARED_IMAGE:熱画像)
 * @return 内部文字列バッファへのポインタ
 */
TCHAR* ResultData::getFilePath(int id, int type) {
	TCHAR *str = getStringBuffer();
	TCHAR fileName[9];
	ZeroMemory(fileName, sizeof(fileName));
	// (2017/5/30YM)輪郭画像用処理追加に伴い処理変更
	switch (type) {
	case INFRARED_IMAGE:
		// typeがIRの場合
		memcpy(fileName, data[id].fileName_ir_8,
				sizeof(data[id].fileName_ir_8));
		break;

	case PANEL_IMAGE:
		// typeがpnの場合
		memcpy(fileName, data[id].fileName_pn_8,
				sizeof(data[id].fileName_pn_8));
		break;

	default:
		// typeがVBの場合
		memcpy(fileName, data[id].fileName_8, sizeof(data[id].fileName_8));
		break;
	}
	stprintf(str, TEXT("%s\\%s.JPG"), dataPath, fileName);
	return str;
}

int ResultData::getFilePath(int id, int type, TCHAR **str) {
	TCHAR fileName[9];
	ZeroMemory(fileName, sizeof(fileName));
	// (2017/5/30YM)輪郭画像用処理追加に伴い処理変更
	switch (type) {
	case INFRARED_IMAGE:
		// typeがIRの場合
		memcpy(fileName, data[id].fileName_ir_8,
				sizeof(data[id].fileName_ir_8));
		break;

	case PANEL_IMAGE:
		// typeがpnの場合
		memcpy(fileName, data[id].fileName_pn_8,
				sizeof(data[id].fileName_pn_8));
		break;

	default:
		// typeがVBの場合
		memcpy(fileName, data[id].fileName_8, sizeof(data[id].fileName_8));
		break;
	}

	int length = lstrlen(dataPath) + lstrlen(fileName) + 5;
	*str = new TCHAR[length + 1];

	stprintf(*str, TEXT("%s\\%s.JPG"), dataPath, fileName);

	return length;
}

int ResultData::getInfraredDataPath(int id, TCHAR **str) {
	TCHAR fileName[9];
	ZeroMemory(fileName, sizeof(fileName));
	memcpy(fileName, data[id].fileName_ir_8, sizeof(data[id].fileName_ir_8));

	int length = lstrlen(dataPath) + lstrlen(fileName) + 5;
	*str = new TCHAR[length + 1];
	stprintf(*str, TEXT("%s\\%s.dat"), dataPath, fileName);
	return length;
}

/**
 * 指定した番号のデータの可視画像ファイルパスを取得する。
 * 内部バッファを使用するため、スレッドセーフではない。
 * @return 内部文字列バッファへのポインタ
 */
TCHAR* ResultData::getVisibleFilePath(int id) {
	return getFilePath(id, VISIBLE_LIGHT_IMAGE);
}

/**
 * 指定した番号のデータの熱画像ファイルパスを取得する。
 * 内部バッファを使用するため、スレッドセーフではない。
 * @return 内部文字列バッファへのポインタ
 */
TCHAR* ResultData::getInfraredFilePath(int id) {
	return getFilePath(id, INFRARED_IMAGE);
}

// (2017/5/30YM)輪郭画像ファイルパスを返す関数を追加
/**
 * 指定した番号のデータの輪郭画像ファイルパスを取得する。
 * 内部バッファを使用するため、スレッドセーフではない。
 * @return 内部文字列バッファへのポインタ
 */
TCHAR* ResultData::getPanelFilePath(int id) {
	return getFilePath(id, PANEL_IMAGE);
}

void ResultData::getDataAreaSize2(double *west, double *south, double *east,
		double *north) {
	double right = 0;
	double top = 0;
	double left = 1;
	double bottom = 1;
	if (getDataCount() != 0) {
		std::vector<InternalPositionInfo>::iterator item = posData.begin();
		const int w = getInfraredWidth();
		const int h = getInfraredHeight();
		double angle, width, height;
		double marginH, marginV;
		for (int i = 0; item != posData.end(); item++, i++) {
			angle = getDirection(i) + getBaseDirection();
			width = (int) floor(
					(fabs(w * cos(angle)) + fabs(h * sin(angle))) / 2 + 0.5);
			height = (int) floor(
					(fabs(w * sin(angle)) + fabs(h * cos(angle))) / 2 + 0.5);
			marginH = pixelToMeter(i, width);// (2017/3/30YM)回転した赤外線画像の横幅の長さ（ｍ）
			marginV = pixelToMeter(i, height);// (2017/3/30YM)回転した赤外線画像の縦幅の長さ（ｍ）
			if (i != 0) {
				if (left > item->x - marginH) {
					left = item->x - marginH;//　(2017/3/30YM)画像の横座標（ｍ）から横幅の長さ（ｍ）を引く
				}
				if (right < item->x + marginH) {
					right = item->x + marginH;
				}
				if (bottom > item->y - marginV) {
					bottom = item->y - marginV;
				}
				if (top < item->y + marginV) {
					top = item->y + marginV;
				}
			} else {
				right = item->x + marginH;
				top = item->y + marginV;
				left = item->x - marginH;
				bottom = item->y - marginV;
			}
		}
	}
	*west = left;
	*south = bottom;
	*east = right;
	*north = top;
}

/**
 * 経緯度から平面直角座標(撮影データの南西基準)を求める。
 */
void ResultData::getXY(double latitude, double longitude, double *x,
		double *y) {
	ll2xy(toRad(latitude), toRad(longitude), toRad(gpsBottom), toRad(gpsLeft),
			x, y);
}

/**
 * 指定したピクセル座標をGPS座標に変換する
 * @param[in] id 変換対象のデータを示すID
 * @param[out] x 変換後のX座標を保存する変数へのポインタ
 * @param[out] y 変換後のY座標を保存する変数へのポインタ
 */
void ResultData::pixelToGPSPos(int x, int y, double *gpsX, double *gpsY,
		double pixelPerMeter) {
	// ミニマップの縦横を計算
	double left, bottom, right, top;
	getDataAreaSize2(&left, &bottom, &right, &top);
	double height = top - bottom;

	// ratio=1 1px=1m

	// x座標を計算
	*gpsX = x / pixelPerMeter + left;

	// y座標を計算(pixel座標系ではY軸が反転しているため、y=height-y'で計算
	*gpsY = height - y / pixelPerMeter + bottom;
}

/**
 * 指定したIDのGPS座標をピクセル座標に変換する
 * @param[in] id 変換対象のデータを示すID
 * @param[out] x 変換後のX座標を保存する変数へのポインタ
 * @param[out] y 変換後のY座標を保存する変数へのポインタ
 */
void ResultData::gpsPosToPixel(double gpsX, double gpsY, int *x, int *y,
		double pixelPerMeter) {
	// ミニマップの縦横を計算
	double left, bottom, right, top;
	getDataAreaSize2(&left, &bottom, &right, &top);
	double height = top - bottom;

	// 位置情報を取得する
	const double dx = gpsX - left;
	const double dy = gpsY - bottom;

	// ratio=1 1px=1m

	// x座標を計算
	*x = floor(dx * pixelPerMeter);

	// y座標を計算(pixel座標系ではY軸が反転しているため、y=height-y'で計算
	*y = floor((height - dy) * pixelPerMeter);
}

/**
 * 指定したIDのGPS座標をピクセル座標に変換する
 * @param[in] id 変換対象のデータを示すID
 * @param[out] x 変換後のX座標を保存する変数へのポインタ
 * @param[out] y 変換後のY座標を保存する変数へのポインタ
 */
void ResultData::gpsPosToPixel(int id, int *x, int *y, double pixelPerMeter) {
	// 範囲チェック
	if ((getDataCount() <= 1) || (id >= getDataCount()) || (id < 0)) {
		*x = *y = 0;
		return;
	}

	// 範囲内の場合
	gpsPosToPixel(posData[id].x, posData[id].y, x, y, pixelPerMeter);
}

/**
 * 文字列バッファサイズを指定する。
 * @param[in] size 文字列バッファサイズ
 */
void ResultData::setStringBufferSize(unsigned int size) {
	if (size < MAX_PATH) {
		return;
	}

	stringBufferSize = size;
	if (stringBuffer != NULL) {
		delete stringBuffer;
	}

	stringBuffer = new TCHAR[size];
}

/**
 * GPS座標の補正を有効にする。
 * @param[in] speed 飛行速度
 * @param[in] points 補正に使用するデータ数
 * @return 補正機能の有無(true:補正可能/false:補正不可)
 */
bool ResultData::enableCorrection(double speed, int points) {
	if (flightDetail->isAvailable()) {
		flightSpeed = speed;
		correctPoints = points;
		return true;
	} else {
		flightSpeed = 0;
		correctPoints = 0;
		return false;
	}
}

/**
 * GPS座標補正を無効にする。
 */
void ResultData::disableCorrection(void) {
	enableCorrection(0, 0);
}

/**
 * 画像の表示倍率を取得する。
 * @param pictureSize 撮影画像の大きさ
 * @param height 撮影高度
 * @param viewAngle 撮影視野角
 */
double ResultData::getPictureRatio(int pictureSize, double height,
		double viewAngle, double pixelPerMeter) {
	// 表示データ数が0の場合は計算しない
	if (getDataCount() <= 0) {
		return 1.0;
	}

	// 画像の1mあたりのピクセル数を計算する //(2019/11/07LEE) viewAngle=> viewAngle*2 で変更。
	const double pxPerMeter = getPixelPerMeter(pictureSize, height, viewAngle);

	// 指定したpixelPerMeterにするには何倍にすればいいのか計算する
	double ratio;
	ratio = pixelPerMeter / pxPerMeter;

	return ratio;
}

/**
 * 磁力の強さから磁気方位を求める。
 * この関数は磁気センサーのX軸方向を北(0[rad])と過程
 * @param[in] mag 磁気センサー値を示す構造体へのポインタ
 * @todo 磁気方位であるため、真方位への補正が必要@n
 * (ただしこの補正には最新の地磁気情報が必要)
 * @todo Z軸補正
 */
double ResultData::getCardinalDirection(Vector3D *mag) {
	/**
	 * y軸方向が想定している向きと逆なので補正をする。
	 * Raspberry Pi2(Navio+)が水平の場合はz軸は無視してよい。
	 * @see MPU-9250 Product Specification Rev1.0 @n
	 * http://store.invensense.com/datasheets/invensense/MPU9250REV1.0.pdf
	 */
	const double x = mag->x;
	const double y = -mag->y;

	// 磁力の強さ(x,y)から角度を求める
	double angle;
	angle = atan2(y, x);
	if (angle < 0) {
		angle += M_PI * 2;
	}

	/**
	 * @return 求めた磁力方向を方位として返す。@n
	 * この場合で0を北としたとき方位は下記のようになる@n
	 * 北:  0[deg]@n
	 * 東: 90[deg]@n
	 * 南:180[deg]@n
	 * 西:270[deg]@n
	 * ※実際の値はrad@n
	 * この角度だけ時計回りに回転させればOK
	 */
	return angle;
}

/**
 * 磁力の強さから磁気方位を求める。
 * この関数は磁気センサーのX軸方向を北(0[rad])と過程
 * @param id 撮影点のID
 * @return 求めた磁力方向を方位として返す。@n
 * この場合で0を北としたとき方位は下記のようになる@n
 * 北:  0[deg]@n
 * 東: 90[deg]@n
 * 南:180[deg]@n
 * 西:270[deg]@n
 * ※実際の値はrad@n
 * この角度だけ時計回りに回転させればOK
 */
double ResultData::getCardinalDirection(int id) {
	const GPSPictureInfo gps = (*this)[id];
	return getCardinalDirection((Vector3D*) &gps.mag);
}

// 方位を求める
double ResultData::getGPSCardinalDirection(int id) {
	if (flightDetail->isAvailable()) {
		// GPS方位補正が可能な場合は、GPS方位補正を行う
		const double correctedDir = flightDetail->getGPSCardinalDirection(
				&data[id].time);
		if (correctedDir != 0) {
			return correctedDir;
		}
	}

	double dx = 0, dy = 0;
	int count = 0;

	const int iMin = (id > 0) ? id - 1 : 0;
	const int iMax = (id < getDataCount() - 1) ? id + 1 : getDataCount() - 1;

	for (int i = iMin; i < iMax; i++) {
		dx += (*this)[i + 1].gps.longitude - (*this)[i].gps.longitude;
		dy += (*this)[i + 1].gps.latitude - (*this)[i].gps.latitude;
		count++;
	}

	if (count > 0) {
		dx /= count;
		dy /= count;
	}

	const double angle = atan2(dx, dy);
	if (angle < 0) {
		// 角度が負の値の場合は2πを加算し正の値にする
		return M_PI * 2 + angle;
	}
	return angle;
}

/**
 * 内部バッファのポインタを取得する。
 */
TCHAR* ResultData::getStringBuffer(void) {
	memset(stringBuffer, 0, stringBufferSize * sizeof(TCHAR));
	return stringBuffer;
}

/**
 * 結果データが有効かどうかを取得する。
 * @return true:有効/false:無効
 */
bool ResultData::isAvailable(void) {
	return available;
}

/**
 * 指定した番号の撮影データを取得する。
 * @param id 撮影データの番号
 * @return 撮影データ@n
 * データが範囲外の場合はすべて0のデータを返す。
 */
GPSPictureInfo ResultData::operator [](unsigned int id) {
	GPSPictureInfo retData;
	if (id < (unsigned int) getDataCount()) {
		if (flightDetail->isAvailable() && (correctPoints > 0)
				&& (flightSpeed > 0)) {
			flightDetail->getCorrectedGPSData(&retData, &data[id].time,
					flightSpeed, correctPoints);
		} else {
			retData = data[id];
		}
	} else {
		ZeroMemory(&retData, sizeof(GPSPictureInfo));
	}
	return retData;
}

bool ResultData::saveTemperaturePicture(int id, double min, double max) {
	int width, height;
	float *tempMap = getTempData(id, &width, &height, NULL);
	if (tempMap == NULL) {
		return false;
	}
	Gdiplus::Bitmap *image = new Gdiplus::Bitmap(width, height,
	PixelFormat32bppARGB);
	Gdiplus::Rect rect;
	rect.X = 0;
	rect.Y = 0;
	rect.Width = width;
	rect.Height = height;
	Gdiplus::BitmapData imageData;
	image->LockBits(&rect, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB,
			&imageData);
	int *const pixels = (int*) (imageData.Scan0);

	// (2017/4/4YM)赤外線画像タイプを選択できるように変更
	int COLOR_TONE;
	int TONE_MAX;
	COLORREF colorPattern[1024];
	// 赤外線タイプ種別
	int InfraredSelectType = mainWindow->getInfraredType();

	// (2016/12/20YM)赤外線画像をグレースケールで作成する
	//　色のパターンをグレースケール(256階調)で作成する
	if (InfraredSelectType == 0) {
		COLOR_TONE = 256;
		TONE_MAX = COLOR_TONE - 1;
		for (int tone = 0; tone < COLOR_TONE; tone++) {
			double percentage = (double) tone / TONE_MAX;
			int red = 0, green = 0, blue = 0;
			red = percentage * 255;
			green = blue = red;
			if (red > 255)
				red = 255;
			else if (red < 0)
				red = 0;
			if (green > 255)
				green = 255;
			else if (green < 0)
				green = 0;
			if (blue > 255)
				blue = 255;
			else if (blue < 0)
				blue = 0;
			colorPattern[tone] = RGB(blue, green, red);
		}
	} else {
		// 色のパターン(1024階調)を作成する
		COLOR_TONE = 1024;
		TONE_MAX = COLOR_TONE - 1;
		for (int tone = 0; tone < COLOR_TONE; tone++) {
			double percentage = (double) tone / TONE_MAX;
			int red = 0, green = 0, blue = 0;
			if (percentage < 0.1) {
				blue = (percentage * 10) * 255;
			} else if (percentage < 0.3) {
				green = ((percentage - 0.1) * 5) * 255;
				blue = 255;
			} else if (percentage < 0.5) {
				green = 255;
				blue = (1 - (percentage - 0.3) * 5) * 255;
			} else if (percentage < 0.7) {
				red = ((percentage - 0.5) * 5) * 255;
				green = 255;
			} else if (percentage < 0.9) {
				red = 255;
				green = (1 - (percentage - 0.7) * 5) * 255;
			} else {
				red = 255;
				green = ((percentage - 0.9) * 10) * 255;
				blue = green;
			}
			if (red > 255)
				red = 255;
			else if (red < 0)
				red = 0;
			if (green > 255)
				green = 255;
			else if (green < 0)
				green = 0;
			if (blue > 255)
				blue = 255;
			else if (blue < 0)
				blue = 0;
			colorPattern[tone] = RGB(blue, green, red);
		}
	}

	const double range = ((double) TONE_MAX) / (max - min);
#	ifdef _OPENMP
#	pragma omp parallel for
#	endif
	for (int y = 0; y < height; y++) {
		const float *tempLine = &tempMap[y * width];
		int *scanLine = &pixels[y * width];
		for (int x = 0; x < width; x++) {
			// 温度を色に変換する
			int tone = (int) ((tempLine[x] - min) * range);
			if (tone > TONE_MAX) {
				tone = TONE_MAX;
			} else if (tone < 0) {
				tone = 0;
			}
			scanLine[x] = colorPattern[tone];
		}
	}

	// 取得したピクセルデータを解放する
	image->UnlockBits(&imageData);

	// 保存処理
	CLSID pngClsid;
	Graphics::GetEncoderClsid(L"image/jpeg", &pngClsid);
	wchar_t *fileName;
#ifndef UNICODE
	fileName = new wchar_t[MAX_PATH]
	MultiByteToWideChar(CP_OEMCP, MB_COMPOSITE,
			getFilePath(id, INFRARED_IMAGE), -1, fileName, MAX_PATH);
#else
	// UNICODE版の場合はそのまま使える
	getFilePath(id, INFRARED_IMAGE, &fileName);
#endif
	image->Save(fileName, &pngClsid, NULL);
	delete fileName;
	delete image;
	delete tempMap;

	return true;
}

bool ResultData::savePanelPicture(int id) {
	cv::Mat imagePanel = Graphics::loadCVImage(getInfraredFilePath(id));

	std::vector<std::vector<cv::Point> > panels;
	getPanels(id, panels);

	for (std::vector<std::vector<cv::Point> >::iterator panel = panels.begin();
			panel != panels.end(); panel++) {
		const cv::Point *p = panel->data();
		int n = (int) panel->size();
		fillPoly(imagePanel, &p, &n, 1, cv::Scalar(0, 0, 255));
	}

	// 保存処理
	Gdiplus::Bitmap *image = Graphics::toBitmap(&imagePanel);
	CLSID pngClsid;
	Graphics::GetEncoderClsid(L"image/jpeg", &pngClsid);
	wchar_t *fileName;
#ifndef UNICODE
	fileName = new wchar_t[MAX_PATH]
	MultiByteToWideChar(CP_OEMCP, MB_COMPOSITE,
			getFilePath(id, INFRARED_IMAGE), -1, fileName, MAX_PATH);
#else
	// UNICODE版の場合はそのまま使える
	getFilePath(id, PANEL_IMAGE, &fileName);
#endif
	image->Save(fileName, &pngClsid, NULL);
	delete fileName;
	delete image;

	return true;
}

float* ResultData::getTempData(int id, int *width, int *height,
		struct tm *time) {
	TCHAR *fileName;
	float *tempData;

	if (dataVersion < 1) {
		// 先行リリース版用
		getFilePath(id, INFRARED_IMAGE, &fileName);
		tempData = ::getTempDataForPreRelease(fileName, width, height, time);
	} else {
		// 正式リリース版用
		getInfraredDataPath(id, &fileName);
		tempData = ::getTempDataV1(fileName, width, height, time);
	}

	delete fileName;

	return tempData;
}
//試しに。
float ResultData::getHotspotTemperature(HotspotNumber num) {
	const int size = 3;
	float tempMax;
	float *tempMap;
	int mapWidth, mapHeight;
	tempMap = getTempData(num.pictureNo, &mapWidth, &mapHeight, NULL);
	if (tempMap == NULL) {
		return 0;
	}

	POINT hotspot;
	getHotspot(num.pictureNo, num.pointNo, &hotspot);
	tempMax = tempMap[hotspot.x + hotspot.y * mapWidth];
	for (int dy = -size; dy <= size; dy++) {
		int y = hotspot.y + dy;
		for (int dx = -size; dx <= size; dx++) {
			int x = hotspot.x + dx;
			// (2016/12/20YM)X,Y座標チェック処理追加
			if ((x >= 0) && (x <= pictureSize.infrared.width) && (y >= 0)
					&& (y <= pictureSize.infrared.height)) {
				if (tempMap[x + y * mapWidth] > tempMax) {
					tempMax = tempMap[x + y * mapWidth];
				}
			}
		}
	}
	delete tempMap;
	return tempMax;
}

/**
 * 画像内のホットスポットの温度リストを取得する
 * @param[in] picNo 画像番号
 * @return 温度リストへのポインタ。@n
 * ※不要になった場合はdeleteで解放してください。
 */
float* ResultData::getHotspotTemperatures(int picNo) {
	const int size = 3;
	int count;
	float tempMax;
	float *tempMap;
	float *tempList = NULL;
	int mapWidth, mapHeight;

	count = getHotspotCount(picNo);

	if (count > 0) {
		tempList = new float[count];
		tempMap = getTempData(picNo, &mapWidth, &mapHeight, NULL);
		for (int ptNo = 0; ptNo < count; ptNo++) {
			POINT hotspot;
			getHotspot(picNo, ptNo, &hotspot);
			tempMax = tempMap[hotspot.x + hotspot.y * mapWidth];
			for (int dy = -size; dy <= size; dy++) {
				int y = hotspot.y + dy;
				for (int dx = -size; dx <= size; dx++) {
					int x = hotspot.x + dx;
					// (2016/12/20YM)X,Y座標チェック処理追加
					if ((x >= 0) && (x <= pictureSize.infrared.width)
							&& (y >= 0) && (y <= pictureSize.infrared.height)) {
						if (tempMap[x + y * mapWidth] > tempMax) {
							tempMax = tempMap[x + y * mapWidth];
						}
					}
				}
			}
			tempList[ptNo] = tempMax;
		}
		delete tempMap;
	}
	return tempList;
}

void ResultData::getTempRange(void) {
	const int DATA_COUNT = getDataCount();

	if (DATA_COUNT == 0) {
		// データ数が0の場合、正しく処理できないため
		// すべてに0をセットして終了する
		maxTemp = 0;
		minTemp = 0;
		aveTemp = 0;
		return;
	}

	double averageSum = 0;
	bool first = true;
	for (int id = 0; id < DATA_COUNT; id++) {
		int width, height;
		float *_tempData = getTempData(id, &width, &height, NULL);
		if (_tempData == NULL) {
			continue;
		}
		float max, min;
		double sum = 0;
		max = min = _tempData[0];
		for (int y = 0; y < height; y++) {
			float *scanLine = &_tempData[y * width];
			for (int x = 0; x < width; x++) {
				float temp = scanLine[x];
				if (temp < min) {
					min = temp;
				} else if (temp > max) {
					max = temp;
				}
				sum += temp;
			}
		}
		averageSum += sum / (width * height);
		if (first == true) {
			maxTemp = max;
			minTemp = min;
			first = false;
		} else {
			if (maxTemp < max) {
				maxTemp = max;
			}
			if (minTemp > min) {
				minTemp = min;
			}
		}
		delete _tempData;
	}
	aveTemp = averageSum / DATA_COUNT;
}

float ResultData::getTempMax(void) {
	return maxTemp;
}
float ResultData::getTempMin(void) {
	return minTemp;
}
float ResultData::getTempAverage(void) {
	return aveTemp;
}
// (2017/5/30YM)パネル上の温度データを返す関数を追加
float ResultData::getPanelTempMax(void) {
	return PNmaxTemp;
}
float ResultData::getPanelTempMin(void) {
	return PNminTemp;
}
float ResultData::getPanelTempAverage(void) {
	return PNaveTemp;
}

TCHAR* ResultData::getHotspotFileName(void) {
	TCHAR *filePath = getStringBuffer();
	stprintf(filePath, TEXT("%s\\WatchOn_Hotspot.txt"), cachePath);
	return filePath;
}

TCHAR* ResultData::getHotspotLinkFileName(void) {
	TCHAR *filePath = getStringBuffer();
	stprintf(filePath, TEXT("%s\\Result_Hotspot_Link.txt"), cachePath);
	return filePath;
}

TCHAR* ResultData::getCameraInfoFileName(void) {
	TCHAR *filePath = getStringBuffer();
	stprintf(filePath, TEXT("%s\\Result_Camera_Info.txt"), cachePath);
	return filePath;
}

void ResultData::getXY(int id, double *x, double *y) {
	*x = posData[id].x;
	*y = posData[id].y;
}

// (2017/5/25YM)温度データファイル名を返す関数を追加
TCHAR* ResultData::getTempDataFileName(void) {
	TCHAR *filePath = getStringBuffer();
	stprintf(filePath, TEXT("%s\\Result_PanelTemp_Data.txt"), cachePath);
	return filePath;
}

//------------------------------------------------------------------------------
// Static Class Functions
//------------------------------------------------------------------------------

double ResultData::getPixelPerMeter(int pictureSize, double viewAngle,
		double height) {
	// 視野(水平走査範囲)
	const double viewWidth = tan(toRad(viewAngle) / 2) * 2;

	// 視野が0の場合も計算しない
	if (viewWidth == 0) {
		return 1.0;
	}

	// 1mあたりのピクセル数を計算する
	return (double) pictureSize / (viewWidth * height);
}

double ResultData::getMeterPerPixel(int pictureSize, double viewAngle,
		double height) {
	// 1ピクセルあたりの距離[m]を計算する
	return 1.0 / getPixelPerMeter(pictureSize, viewAngle, height);
}

double ResultData::meterToPixel(int pictureSize, double viewAngle,
		double height, double distance) {
	return distance * getPixelPerMeter(pictureSize, viewAngle, height);
}

double ResultData::pixelToMeter(int pictureSize, double viewAngle,
		double height, double distance) {
	return distance * getMeterPerPixel(pictureSize, viewAngle, height);
}

//------------------------------------------------------------------------------
// Camera Adjustment Functions
//------------------------------------------------------------------------------

int ResultData::getInfraredWidth(void) {
	return pictureSize.infrared.width;
}

int ResultData::getInfraredHeight(void) {
	return pictureSize.infrared.height;
}

int ResultData::getVisibleWidth(void) {
	return pictureSize.visible.width;
}

int ResultData::getVisibleHeight(void) {
	return pictureSize.visible.height;
}

int ResultData::getPictureWidth(int type) {
	switch (type) {
	case ResultData::INFRARED_IMAGE:

	case ResultData::PANEL_IMAGE:		// (2017/7/24YM)パネル輪郭追加
		return getInfraredWidth();

	case ResultData::VISIBLE_LIGHT_IMAGE:

	default:
		return getVisibleWidth();
	}
}

int ResultData::getPictureHeight(int type) {
	switch (type) {
	case ResultData::INFRARED_IMAGE:

	case ResultData::PANEL_IMAGE:		// (2017/7/24YM)パネル輪郭追加
		return getInfraredHeight();

	case ResultData::VISIBLE_LIGHT_IMAGE:

	default:
		return getVisibleHeight();
	}
}

//------------------------------------------------------------------------------
// Camera Adjustment Functions
//------------------------------------------------------------------------------
//-----------------------------------------------------------------------
//(2019/11/05LEE)新しい事を追加。

void ResultData::getCameraInfo(int id, CameraInfo *cameraInfo) {
	if (checkOutOfRange(picturedata, id)) {
		return;
	}
	cameraInfo->offset.x = picturedata[id].offset.x;
	cameraInfo->offset.y = picturedata[id].offset.y;
	cameraInfo->ratio.y = picturedata[id].ratio.y;
	cameraInfo->ratio.x = picturedata[id].ratio.x;
	cameraInfo->direction = picturedata[id].direction;
	cameraInfo->datatype = picturedata[id].datatype;
}

void ResultData::setCameraOffset2(int id, double x, double y) {
	picturedata[id].offset.x = x;
	picturedata[id].offset.y = y;
}

void ResultData::getCameraOffset2(int id, double *x, double *y) {
	*x = picturedata[id].offset.x;
	*y = picturedata[id].offset.y;
}

void ResultData::setCameraRatio2(int id, double vertical, double horizontal) {
	picturedata[id].ratio.x = horizontal;
	picturedata[id].ratio.y = vertical;
}

void ResultData::getCameraRatio2(int id, double *vertical, double *horizontal) {
	*horizontal = picturedata[id].ratio.x;
	*vertical = picturedata[id].ratio.y;
}

// (2016/12/20YM)可視赤外カメラ回転補正データセット処理追加
void ResultData::setCameraDirection2(int id, double direction) {
	picturedata[id].direction = direction;
}

void ResultData::getCameraDirection2(int id, double *direction) {
	*direction = picturedata[id].direction;
}

void ResultData::setCameraDatatype(int id, int datatype) {
	picturedata[id].datatype = datatype;
}

void ResultData::getCameraDatatype(int id, int *datatype) {
	*datatype = picturedata[id].datatype;
}

void ResultData::resetalldata(int id) {
	picturedata[id] = resetdata;	// (2019/11/29LEE) 追加
	picturedata[id].datatype = 0;
}
void ResultData::getalldata(double *x, double *y, double *vertical,
		double *horizontal, double *direction) {
	*x = resetdata.offset.x;
	*y = resetdata.offset.y;
	*vertical = resetdata.ratio.x;
	*horizontal = resetdata.ratio.y;
	*direction = resetdata.direction;
}
// (2019/12/03LEE) カメラ特性で全体モードと個別モードを詳しく分離。
void ResultData::setalldata(int id, double x, double y, double vertical,
		double horizontal, double direction) {
	resetdata.offset.x = x;
	resetdata.offset.y = y;
	resetdata.ratio.x = vertical;
	resetdata.ratio.y = horizontal;
	resetdata.direction = direction;
	resetdata.datatype = 0;

	for (int i = 0; i < getDataCount(); i++) // (2019/12/03LEE) 全体の値と個別の値を区分
			{
		if (picturedata[i].datatype == 0) {
			picturedata[i] = resetdata;
		}
	}

	picturedata[id].datatype = 0;
}

//-----------------------------------------------------------------------//(2019/11/05LEE)追加。

void ResultData::setCameraOffset(double x, double y) {
	cameraInfo.offset.x = x;
	cameraInfo.offset.y = y;
}

void ResultData::getCameraOffset(double *x, double *y) {
	*x = cameraInfo.offset.x;
	*y = cameraInfo.offset.y;
}

void ResultData::setCameraRatio(double vertical, double horizontal) {
	cameraInfo.ratio.x = horizontal;
	cameraInfo.ratio.y = vertical;
}

void ResultData::getCameraRatio(double *vertical, double *horizontal) {
	*horizontal = cameraInfo.ratio.x;
	*vertical = cameraInfo.ratio.y;
}

// (2016/12/20YM)可視赤外カメラ回転補正データセット処理追加
void ResultData::setCameraDirection(double direction) {
	cameraInfo.direction = direction;
}

void ResultData::getCameraDirection(double *direction) {
	*direction = cameraInfo.direction;
}
//　↑ここまで追加

double ResultData::getViewAngle(void) {
	return 45.0;
//	return 32.0;
}

bool ResultData::saveCameraInfo(const TCHAR *filePath) {
	// データをファイルに保存する
	FILE *file = fileOpen(filePath, TEXT("w"));
	if (file == NULL) {
		return false;
	}

	// ファイルに書き込む
	for (int i = 0; i < getDataCount(); i++) {
		fprintf(file, "%lf,%lf,%lf,%lf,%lf,%lf,%d\n", picturedata[i].offset.x,
				picturedata[i].offset.y, picturedata[i].ratio.y,
				picturedata[i].ratio.x, picturedata[i].direction, baseDirection,
				picturedata[i].datatype);
	}
	fclose(file);

	return true;
}

bool ResultData::loadCameraInfo(const TCHAR *filePath) {
	// ファイルを開く
	FILE *file = fileOpen(filePath, TEXT("r"));
	if (file == NULL) {
		return false;
	}
	int checkret = 0;
	for (int i = 0; i < getDataCount(); i++) {
		checkret = fscanf(file, "%lf,%lf,%lf,%lf,%lf,%lf,%d\n",
				&picturedata[i].offset.x, &picturedata[i].offset.y,
				&picturedata[i].ratio.y, &picturedata[i].ratio.x,
				&picturedata[i].direction, &baseDirection,
				&picturedata[i].datatype);
		if (picturedata[i].datatype == 0) {
			resetdata = picturedata[i];
		}
	}

	fclose(file);
	// (2020/01/16LEE)データ個数を6->7へ修正
	if (checkret != 7) {
		// ファイル書式が不正な場合
		ZeroMemory(&cameraInfo, sizeof(CameraInfo));
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
// Shooting Data Adjustment Functions
//------------------------------------------------------------------------------

void ResultData::setPosition(int id, InternalPositionInfo *pos) {
	posData[id].x = pos->x;
	posData[id].y = pos->y;
	posData[id].height = pos->height;
	posData[id].direction = pos->direction;
}

void ResultData::getPosition(int id, InternalPositionInfo *pos) {
	if (checkOutOfRange(posData, id)) {
		return;
	}

	pos->x = posData[id].x;
	pos->y = posData[id].y;
	pos->height = posData[id].height;
	pos->direction = posData[id].direction;
}

void ResultData::setDirection(int id, double direction) {
	posData[id].direction = direction;
}

double ResultData::getDirection(int id) {
	return posData[id].direction;
}

void ResultData::setBaseDirection(double direction) {
	baseDirection = direction;
}

double ResultData::getBaseDirection(void) {
	return baseDirection;
}

void ResultData::setHeight(int id, double height) {
	posData[id].height = height;
}

double ResultData::getHeight(int id) {
	double height = posData[id].height;
	if (height < 0.0) {
		return height;
	}
	return height;
}

void ResultData::setDefaultInternalPositionInfo(int id) {
	getXY(data[id].gps.latitude, data[id].gps.longitude, &posData[id].y,
			&posData[id].x);
	posData[id].height = getGPSHeight(id);
}

void ResultData::setDefaultDirection(int id) {
	TCHAR *fileName;
	getFilePath(id, VISIBLE_LIGHT_IMAGE, &fileName);
	posData[id].direction = -PanelDetector::getPanelAngle(fileName);
	delete fileName;
}

bool ResultData::savePositionInfo(const TCHAR *filePath) {
	// データをファイルに保存する
	FILE *file = fileOpen(filePath, TEXT("w"));
	if (file == NULL) {
		return false;
	}

	for (int i = 0; i < getDataCount(); i++) {
		// ファイルに書き込む
		fprintf(file, "x:%lf,y:%lf,h:%lf,dir:%lf\n", posData[i].x, posData[i].y,
				posData[i].height, posData[i].direction);
	}

	fclose(file);

	return true;
}

bool ResultData::loadPositionInfo(const TCHAR *filePath) {
	// ファイルを開く
	FILE *file = fileOpen(filePath, TEXT("r"));
	if (file == NULL) {
		return false;
	}

	for (int id = 0; !feof(file) && id < getDataCount(); id++) {
		InternalPositionInfo pos;
		// ファイルを読み込む
		int ret = fscanf(file, "x:%lf,y:%lf,h:%lf,dir:%lf\n", &pos.x, &pos.y,
				&pos.height, &pos.direction);
		if (ret == 4) {
			posData[id] = pos;
		}
	}
	fclose(file);

	return true;
}

bool ResultData::savePositionInfo(void) {
	TCHAR *str = getStringBuffer();
	wsprintf(str, TEXT("%s\\Result_Position_Data.txt"), cachePath);
	return savePositionInfo(str);
}

bool ResultData::loadPositionInfo(void) {
	TCHAR *str = getStringBuffer();
	wsprintf(str, TEXT("%s\\Result_Position_Data.txt"), cachePath);
	return loadPositionInfo(str);
}

bool ResultData::adjustPosition(int base, int target) {
	std::vector<int> linkList;
	for (int i = 0; i < getHotspotCount(target); i++) {
		int idTarget = this->getHotspotId(target, i);
		if (idTarget == -1) {
			continue;
		}
		for (int j = 0; j < getHotspotCount(base); j++) {
			int idBase = this->getHotspotId(base, j);
			if ((idTarget == -1) || (idTarget != idBase)) {
				continue;
			}
			linkList.push_back(i);
		}
	}

	int size = (int) (linkList.size());

	if (size == 0) {
		return false;
	}

	double minDist = 0;
	int minId = 0;
	for (int i = 0; i < size; i++) {
		// 中央から一番近い点を結合点として利用
		Vector2DPolar v;
		getPolarCoordinates(target, linkList[i], &v);
		if ((i == 0) || (v.r < minDist)) {
			minDist = v.r;
			minId = i;
		}
	}

	int targetPointNo = linkList[minId];
	int basePointNo = -1;

	int hotspotId = getHotspotId(target, linkList[minId]);
	HotspotLink link = (*hotspotLinker)[hotspotId];
	for (int i = 0; i < (int) link.size(); i++) {
		if (link[i].pictureNo == base) {
			basePointNo = link[i].pointNo;
			break;
		}
	}

	double x1 = posData[base].x;
	double y1 = posData[base].y;

	double dx1, dy1;
	Vector2DPolar pos1;
	getPolarCoordinates(base, basePointNo, &pos1);

	pos1.arg += getDirection(base) + getBaseDirection();
	dx1 = pos1.r * cos(pos1.arg);
	dy1 = pos1.r * sin(pos1.arg);
	dx1 = pixelToMeter(base, dx1);
	dy1 = -pixelToMeter(base, dy1);

	double dx2, dy2;
	Vector2DPolar pos2;
	getPolarCoordinates(target, targetPointNo, &pos2);

	pos2.arg += getDirection(target) + getBaseDirection();
	dx2 = pos2.r * cos(pos2.arg);
	dy2 = pos2.r * sin(pos2.arg);
	dx2 = pixelToMeter(target, dx2);
	dy2 = -pixelToMeter(target, dy2);

	double x2 = x1 + dx1 - dx2;
	double y2 = y1 + dy1 - dy2;

	posData[target].x = x2;
	posData[target].y = y2;

	return true;
}

bool ResultData::adjustPosition(int id) {
	std::queue<int> scan;
	bool completed[getDataCount()];
	getXY(data[id].gps.latitude, data[id].gps.longitude, &posData[id].y,
			&posData[id].x);

	scan.push(id);
	ZeroMemory(completed, sizeof(bool) * getDataCount());
	completed[id] = true;

	do {
		int i = scan.front();
		scan.pop();
		for (int j = 0; j < getDataCount(); j++) {
			if (completed[j] == true) {
				continue;
			}
			if (adjustPosition(i, j) == true) {
				scan.push(j);
				completed[j] = true;
			}
		}
	} while (!scan.empty());

	return true;
}

//(2019/12/26LEE) ここでホットスポット補正写真のデータ確認
int ResultData::getAroundPictureId(int id, double angle, double error) {
	double min = -1;
	int minId = -1;
	double x0, y0;
	getXY(id, &x0, &y0);
	for (int i = 0; i < getDataCount(); i++) {
		if (i == id) {
			continue;
		}
		double x1, y1;
		getXY(i, &x1, &y1);
		const double dx = x1 - x0;
		const double dy = y1 - y0;
		double _angle = atan2(dx, dy);
		while (_angle < -error) {
			_angle += 2 * M_PI;
		}
		if (fabs(_angle - angle) < error) {
			// 距離の2乗を求める
			const double distance = dx * dx + dy * dy;
			if ((min < 0) || (min > distance)) {
				min = distance;
				minId = i;
			}
		}
	}
	return minId;

}

int ResultData::getAroundPictureId(int id, int directionId) {
	int aroundId[3];
	double error[3] = {
	M_PI / 16,
	M_PI / 8,
	M_PI / 4 };
	const double direction = M_PI / 2 * directionId;
	double distance[3];

	double x0, y0;
	getXY(id, &x0, &y0);

	for (int i = 0; i < 3; i++) {
		aroundId[i] = getAroundPictureId(id, direction, error[i]);

		if (aroundId[i] != -1) {
			double x1, y1;
			getXY(aroundId[i], &x1, &y1);
			const double dx = x1 - x0;
			const double dy = y1 - y0;
			distance[i] = (dx * dx + dy * dy) * pow(1.5, i);
		} else {
			distance[i] = 1e300;
		}
	}

	int minId = 0;
	for (int i = 1; i < 3; i++) {
		if (distance[i] < distance[minId]) {
			minId = i;
		}
	}

	// (2020/01/07LEE) 画面を右、左に順番で対応するために追加

	if (directionId == 1) {
		if (id < getDataCount()) {
			aroundId[minId] = id + 1;
		}
		if (id == getDataCount() - 1) {
			return -1;
		}
	}

	if (directionId == 3) {
		if (id > 0) {
			aroundId[minId] = id - 1;
		}
		if (id == 0) {
			return -1;
		}
	}
	// (2020/01/07LEE) 画面を右、左に順番で対応するために追加

	return aroundId[minId];
}

bool ResultData::isAroundPicture(int id, int target) {
	if (id == -1 || target == -1) {
		return false;
	}
	for (int i = 0; i < 4; i++) {
		if (getAroundPictureId(id, i) == target) {
			return true;
		}
	}
	return false;
}

/**
 * メートル単位からピクセル単位に変換する
 * @param id 画像番号
 * @param distance 変換前の長さ[m]
 * @return 変換後のピクセル数
 */
double ResultData::meterToPixel(int id, double distance) {
	return meterToPixel(getInfraredWidth(), getViewAngle(), getHeight(id),
			distance);
}

/**
 * ピクセル単位からピクセル単位に変換する
 * @param id 画像番号
 * @param distance 変換前のピクセル数
 * @return 変換後の長さ[m]
 */
double ResultData::pixelToMeter(int id, double distance) {
	return pixelToMeter(getInfraredWidth(), getViewAngle(), getHeight(id),
			distance);
}

//------------------------------------------------------------------------------
// HotspotManager
//------------------------------------------------------------------------------

/**
 * ホットスポット検出
 * @param[in] id 検出を行う画像の番号
 * @param[in] threshold 閾値
 */
void ResultData::detectHotspot(int id, float threshold) { // (2019/09/25LEE) int threshold => float threshold に変更。
	POINT *points = NULL;
	int count = detectHotspotImpl(id, &points, threshold);

	hotspots[id].clear();
	HotspotList().swap(hotspots[id]);
	for (int j = 0; j < count; j++) {
		addHotspot(id, points[j].x, points[j].y, HOTSPOT_TYPE_CANDIDATE);
	}
	free(points);

}

/**
 * ホットスポットをファイルに保存する
 */
int ResultData::saveHotspot(void) {
	return saveHotspot(getHotspotFileName());
}

/**
 * ホットスポットをファイルから読み込む
 */
int ResultData::loadHotspot(void) {
	return loadHotspot(getHotspotFileName());
}
/*
 * 全体俯瞰画像が既にファイル内に存在しているか確認する
 */
bool ResultData::existWholePicture(int pictureType) {
	return loadWholePicture(getWholePictureFileName(pictureType));
}

TCHAR* ResultData::getWholePictureFileName(int pictureType) {
	TCHAR *filePath = getStringBuffer();
	if (pictureType == 0) {
		wsprintf(filePath, TEXT("%s\\visible.png"), cachePath);
	} else {
		wsprintf(filePath, TEXT("%s\\infrared.png"), cachePath);
	}
	return filePath;
}

TCHAR* ResultData::getFlightDataFileName(void) {
	TCHAR *filePath = getStringBuffer();
	wsprintf(filePath, TEXT("%s\\FlightData.txt"), cachePath);
	return filePath;
}

bool ResultData::loadWholePicture(TCHAR *filePath) {
	// ファイルを開く
	FILE *f = fileOpen(filePath, TEXT("r"));
	if (f == NULL) {
		return false;
	}
	fclose(f);
	return true;
}

int ResultData::disableHotspotCount(int id) {
	int count = 0;
	for (int i = 0; i < getHotspotCount(id); i++) {
		if (hotspots[id][i].type == HOTSPOT_TYPE_DISABLED) {
			count++;
		}
	}
	return count;
}

int ResultData::getHotspotCount(int id) {
	return hotspots[id].size();
}

int ResultData::getHotspotCountAll(void) {
	int count = 0;
	for (int i = 0; i < (int) hotspots.size(); i++) {
		count += hotspots[i].size() - disableHotspotCount(i);
	}
	return count;
}

bool ResultData::existHotspot(int id) {
	for (int i = 0; i < getHotspotCount(id); i++) {
		if (isHotspot(id, i)) {
			return true;
		}
	}
	return false;
}

void ResultData::getHotspot(int dataId, int spotId, POINT *point) {
	point->x = hotspots[dataId][spotId].x;
	point->y = hotspots[dataId][spotId].y;
}

int ResultData::getHotspotId(int dataId, int spotId) {
	return hotspots[dataId][spotId].id;
}

void ResultData::setHotspotId(int dataId, int spotId, int id) {
	hotspots[dataId][spotId].id = id;
}

int ResultData::getHotspotType(int dataId, int spotId) {
	return hotspots[dataId][spotId].type;
}

void ResultData::setHotspotType(int dataId, int spotId, int type) {
	hotspots[dataId][spotId].type = type;
}

void ResultData::enableHotspot(int dataId, int spotId) {
	setHotspotType(dataId, spotId, HOTSPOT_TYPE_ENABLED);
}

void ResultData::disableHotspot(int dataId, int spotId) {
	setHotspotType(dataId, spotId, HOTSPOT_TYPE_DISABLED);
}

/**
 * 指定したホットスポットがホットスポットかどうかを確認する。
 * @param[in] type ホットスポット種別
 * @return true:有効/false:無効(ホットスポット以外も含む)
 */
bool ResultData::isHotspot(int dataId, int spotId) {
	int type = getHotspotType(dataId, spotId);
	return (type == HOTSPOT_TYPE_CANDIDATE) || (type == HOTSPOT_TYPE_ENABLED);
}

/**
 * 指定したホットスポットが有効なホットスポットかどうかを確認する。
 * @param[in] type ホットスポット種別
 * @return true:有効/false:無効(ホットスポット以外も含む)
 */
bool ResultData::isEnabledHotspot(int dataId, int spotId) {
	int type = getHotspotType(dataId, spotId);
	return (type == HOTSPOT_TYPE_ENABLED);
}

/**
 * 指定したホットスポットがホットスポット候補かどうかを確認する。
 * @param[in] type ホットスポット種別
 * @return true:有効/false:無効(ホットスポット以外も含む)
 */
bool ResultData::isHotspotCandidate(int dataId, int spotId) {
	int type = getHotspotType(dataId, spotId);
	return (type == HOTSPOT_TYPE_CANDIDATE);
}

/**
 * 指定したホットスポットが無効化されているかどうかを確認する。
 * @param[in] type ホットスポット種別
 * @return true:有効/false:無効(ホットスポット以外も含む)
 */
bool ResultData::isDisabledHotspot(int dataId, int spotId) {
	int type = getHotspotType(dataId, spotId);
	return (type == HOTSPOT_TYPE_DISABLED);
}

/**
 * 指定したホットスポットが特徴点かどうかを確認する。
 * @param[in] type ホットスポット種別
 * @return true:有効/false:無効(ホットスポット以外も含む)
 */
bool ResultData::isKeypoint(int dataId, int spotId) {
	int type = getHotspotType(dataId, spotId);
	return (type == HOTSPOT_TYPE_KEYPOINT);
}

/**
 * ホットスポットデータを保存する
 */
int ResultData::saveHotspot(TCHAR *filePath) {
	// データをファイルに保存する
	FILE *file = fileOpen(filePath, TEXT("w"));
	if (file == NULL) {
		return false;
	}

	for (int i = 0; i < (int) hotspots.size(); i++) {
		HotspotList *hotspot = &this->hotspots[i];
		for (int j = 0; j < (int) hotspot->size(); j++) {
			int x = (*hotspot)[j].x;
			int y = (*hotspot)[j].y;
			int type = (*hotspot)[j].type;
			fprintf(file, "%d-%d,x:%d,y:%d,enable:%d\n", i, j, x, y, type);
		}
	}

	fclose(file);
	return true;
}

/**
 * ホットスポット情報をクリアする
 */
void ResultData::clearHotspot(void) {
	for (std::vector<HotspotList>::iterator item = hotspots.begin();
			item != hotspots.end(); item++) {
		item->clear();
		HotspotList().swap(*item);
	}
}

/**
 * ホットスポットを追加する
 * @param id ホットスポットを追加する画像番号
 * @param x ホットスポットのx座標
 * @param y ホットスポットのy座標
 */
void ResultData::addHotspot(int id, int x, int y, int type) {
	Hotspot hotspot;
	hotspot.id = NOT_ASSIGNED;
	hotspot.type = type;
	hotspot.x = x;
	hotspot.y = y;
	this->hotspots[id].push_back(hotspot);
}

/**
 * ホットスポットを削除する
 * @param id ホットスポットを削除する画像番号
 * @param hotspotId ホットスポット番号
 */
void ResultData::delHotspot(int id, int hotspotId) {
	if (checkOutOfRange(this->hotspots, id)) {
		return;
	}
	if (checkOutOfRange(this->hotspots[id], hotspotId)) {
		return;
	}
	this->hotspots[id].erase(this->hotspots[id].begin() + hotspotId);
}

/**
 * 指定した座標に最も近いホットスポットの番号を取得する
 */
int ResultData::getHotspot(int id, int x, int y) {
	double min = 0x7fffffff;
	int minId = NOT_ASSIGNED;
	int count = hotspots[id].size();
	for (int i = 0; i < count; i++) {
		int dx = hotspots[id][i].x - x;
		int dy = hotspots[id][i].y - y;
		int dist = sqrt(dx * dx + dy * dy);
		if (min > dist) {
			min = dist;
			minId = i;
		}
	}
	return minId;
}

/**
 * ホットスポットデータを読み込む
 */
int ResultData::loadHotspot(TCHAR *filePath) {
	// ファイルを開く
	FILE *f = fileOpen(filePath, TEXT("r"));
	if (f == NULL) {
		return false;
	}

	// ホットスポット情報をクリアする
	for (int i = 0; i < (int) hotspots.size(); i++) {
		this->hotspots[i].clear();
	}

	// ファイルを読み込む
	while (!feof(f)) {
		int picNo, ptNo, x, y, type;
		int ret = fscanf(f, "%d-%d,x:%d,y:%d,enable:%d\n", &picNo, &ptNo, &x,
				&y, &type);
		if (ret != 5) {
			// データ形式が異なる場合は無視
			continue;
		}
		if (checkOutOfRange(this->hotspots, picNo)) {
			// 不正な画像番号が指定されている場合も無視する
			continue;
		}

		HotspotList *hotspot = &this->hotspots[picNo];

		// データ数を増やす
		if ((int) hotspot->size() <= ptNo) {
			for (int k = hotspot->size(); k <= ptNo; k++) {
				Hotspot item;
				ZeroMemory(&item, sizeof(Hotspot));
				item.id = NOT_ASSIGNED;
				hotspot->push_back(item);
			}
		}

		(*hotspot)[ptNo].x = x;
		(*hotspot)[ptNo].y = y;
		(*hotspot)[ptNo].type = type;
		(*hotspot)[ptNo].id = NOT_ASSIGNED;
	}
	fclose(f);

	return true;
}

/**
 * 極座標形式でホットスポット位置を取得する
 */
void ResultData::getPolarCoordinates(int pictureNo, int pointNo,
		Vector2DPolar *point) {
	POINT pointRectangular;
	getHotspot(pictureNo, pointNo, &pointRectangular);
	int x = pointRectangular.x - getInfraredWidth() / 2;
	int y = pointRectangular.y - getInfraredHeight() / 2;

	point->r = sqrt(x * x + y * y);
	point->arg = atan2(y, x);
}

/**
 * 2つのホットスポットのずれ幅を取得する
 */
void ResultData::getHotspotRelativePos(int picNo1, int ptNo1, int picNo2,
		int ptNo2, double ratio, Vector2D *relativePos) {
	Vector2D pt1, pt2;

	Vector2DPolar polar[2];
	getPolarCoordinates(picNo1, ptNo1, &polar[0]);

	double arg1 = polar[0].arg + getDirection(picNo1) + getBaseDirection();
	pt1.x = polar[0].r * cos(arg1);
	pt1.y = polar[0].r * sin(arg1);

	getPolarCoordinates(picNo2, ptNo2, &polar[1]);

	double arg2 = polar[1].arg + getDirection(picNo2) + getBaseDirection();
	pt2.x = ratio * polar[1].r * cos(arg2);
	pt2.y = ratio * polar[1].r * sin(arg2);

	relativePos->x = pt2.x - pt1.x;
	relativePos->y = pt1.y - pt2.y;
}

/**
 * 指定した画像同士がリンクされているか確認する
 * @param picNo1 確認する画像1の番号
 * @param picNo2 確認する画像2の番号
 * @return true:リンクあり/false:リンクなし
 */
bool ResultData::existsLink(int picNo1, int picNo2) {
	// picNo1とpicNo2が等しい場合
	if (picNo1 == picNo2) {
		return false;
	}
	// picNo1が範囲外の場合
	if ((picNo1 < 0) || (picNo1 >= getDataCount())) {
		return false;
	}
	// picNo2が範囲外の場合
	if ((picNo2 < 0) || (picNo2 >= getDataCount())) {
		return false;
	}

	// 2つの画像のホットスポット数を取得する
	const int count1 = this->getHotspotCount(picNo1);
	const int count2 = this->getHotspotCount(picNo2);

	// 同じIDのホットスポットを探す
	for (int ptNo1 = 0; ptNo1 < count1; ptNo1++) {
		int id1 = getHotspotId(picNo1, ptNo1);
		if (id1 == NOT_ASSIGNED) {
			// 未割り当て
			continue;
		}
		for (int ptNo2 = 0; ptNo2 < count2; ptNo2++) {
			int id2 = getHotspotId(picNo2, ptNo2);
			if (id2 == NOT_ASSIGNED) {
				// 未割り当て
				continue;
			}
			if (id1 == id2) {
				// 同じIDのホットスポットがあればリンクあり
				return true;
			}
		}
	}

	return false;
}

/**
 * ホットスポット総数を取得する
 */
int ResultData::getHotspotSum(void) {
	int picNo, ptNo, ptSum;
	int noLink = 0;
	int yesLink = 0;

	for (picNo = 0; picNo < getDataCount(); picNo++) {
		for (ptNo = 0; ptNo < getHotspotCount(picNo); ptNo++) {
			if (isHotspot(picNo, ptNo)) {
				if (isUniqueHotspot(picNo, ptNo)) {
					noLink++;
				}
			}
		}
	}
	yesLink = hotspotLinker->hotspotCount();
	ptSum = noLink + yesLink;
	return ptSum;
}

/**
 * ホットスポットの位置を実世界座標で取得する
 * @param number ホットスポット番号
 * @return 指定ホットスポットの位置
 */
Vector2D ResultData::getHotspotInRealWorld(HotspotNumber number) {
	Vector2D pos;

	// ホットスポットの相対位置(画像の中心からの)を計算する
	Vector2DPolar polar;
	getPolarCoordinates(number.pictureNo, number.pointNo, &polar);

	double arg = polar.arg + getDirection(number.pictureNo)
			+ getBaseDirection();
	pos.x = pixelToMeter(number.pictureNo, polar.r * cos(arg));
	pos.y = -pixelToMeter(number.pictureNo, polar.r * sin(arg));

	// 相対位置に画像の中心位置を加算する
	pos.x += posData[number.pictureNo].x;
	pos.y += posData[number.pictureNo].y;
	return pos;
}

/**
 * ホットスポットIDから画像番号とホットスポットの画像内番号を取得する
 */
HotspotNumber ResultData::getHotspotNumber(int id) {
	HotspotNumber num = { -1, -1 };
	if (id < hotspotLinker->linkCount()) {
		// IDが割り振られているものはそのまま処理する
		return (*hotspotLinker)[id][0];
	} else if (id >= 0) {
		// IDが割り振られていないものは仮IDを割り当てて処理する
		int noLinkId = hotspotLinker->linkCount();
		for (int picNo = 0; picNo < getDataCount(); picNo++) {
			for (int ptNo = 0; ptNo < getHotspotCount(picNo); ptNo++) {
				if (isUniqueHotspot(picNo, ptNo)) {
					// リンクがないホットスポットの場合
					if (id == noLinkId) {
						num.pictureNo = picNo;
						num.pointNo = ptNo;
						return num;
					}
					noLinkId++;
				}
			}
		}
	}

	// 指定したIDがない場合
	return num;
}

/**
 * 同一ホットスポットを除くホットスポット総数を取得する
 * @return ホットスポット総数
 */
int ResultData::getUniqueHotspotCount(void) {
	int count = hotspotLinker->linkCount();
	for (int picNo = 0; picNo < getDataCount(); picNo++) {
		for (int ptNo = 0; ptNo < getHotspotCount(picNo); ptNo++) {
			if (isUniqueHotspot(picNo, ptNo)) {
				count++;
			}
		}
	}
	return count;
}

bool ResultData::isUniqueHotspot(int pictureNo, int pointNo) {
	if (getHotspotId(pictureNo, pointNo) == NOT_ASSIGNED) {
		return true;
	}
	return false;
}

TCHAR* ResultData::getPanelInfoPath(void) {
	TCHAR *str = getStringBuffer();
	wsprintf(str, TEXT("%s\\Result_PanelInfo_Data.txt"), cachePath);
	return str;
}

void ResultData::initHotspotOverallPosition(void) {
	allHotspot.clear();
	std::vector<Vector2D>().swap(allHotspot);
	allHotspotNo.clear();
	std::vector<HotspotNumber>().swap(allHotspotNo);
	allHotspotTemp.clear();
	std::vector<float>().swap(allHotspotTemp);
	allHotspotChecked.clear();
}

void ResultData::getHotspotOverallPosition(int pictureNo, int pointNo) {
	if (isHotspot(pictureNo, pointNo)) {
		int id = getHotspotId(pictureNo, pointNo);
		if (id != -1) {
			allHotspotCheckedData::iterator it = std::find(
					allHotspotChecked.begin(), allHotspotChecked.end(), id);
			if (it != allHotspotChecked.end()) {
				// リンクがある場合は最初のひとつだけにする
				return;
			}
			allHotspotChecked.push_back(id);
		}
		HotspotNumber hotspotNumber = hotspotNo(pictureNo, pointNo);
		Vector2D pos = getHotspotInRealWorld(hotspotNumber);
		float temp = getHotspotTemperature(hotspotNumber);
		//試しに。
		allHotspotTemp.push_back(temp);
		//緯度経度から写真番号、ホットスポット番号が参照できるようにする。
		allHotspotNo.push_back(hotspotNumber);
		allHotspot.push_back(pos);

		//デバッグ用↓↓
		debug("%4d:%d-%d", allHotspot.size(), pictureNo, pointNo);
	}
}

/**
 * ある範囲内の中のホットスポットの数を取得する
 */
int ResultData::getHotspotCountInArea(Vector2D *area, int count) {
	int ptCount = 0;
	for (int i = 0; i < (int) allHotspot.size(); i++) {
		if (isPointInPolygon(allHotspot[i], count, area)) {
			ptCount++;
		}
	}
	return ptCount;
}

int ResultData::getAllHotspotCount(void) {
	return (int) allHotspot.size();
}

Vector2D ResultData::getAllHotspotPos(int id) {
	return allHotspot[id];
}

// (2017/3/3YM)ホットスポットNoを返す変数追加
HotspotNumber ResultData::getAllHotspotNo(int id) {
	HotspotNumber hn;
	hn.pictureNo = allHotspotNo[id].pictureNo;
	hn.pointNo = allHotspotNo[id].pointNo;
	return hn;
}

HotspotDetail ResultData::getHotspotDetail(int id) {
	HotspotDetail detail;
	double latitude, longitude;
	xy2bl(allHotspot[id].y, allHotspot[id].x, toRad(gpsBottom), toRad(gpsLeft),
			toRad(gpsTop), toRad(gpsRight), &latitude, &longitude);
	detail.longitude = toDeg(longitude);
	detail.latitude = toDeg(latitude);
	detail.panelName = getPanelName(allHotspot[id].x, allHotspot[id].y);

	detail.temperature = allHotspotTemp[id];
	//　(2017/6/2YM)パネル平均温度に変更
	HotspotNumber hn = getAllHotspotNo(id);
	detail.temperatureAverage = PanelaveTemp[hn.pictureNo];
	//ポインタで取得した文字列をコピーし、コピーしたものを参照する。
	//（ポインタの中身が書き変わっている可能性があるため。）
	detail.fileNameIr = clone(getfileNameIr(id));
	detail.fileNameVb = clone(getfileNameVb(id));
	return detail;
}

HotspotDetail* ResultData::getHotspotListDetail(void) {
	int count = mainWindow->getHotspotIdListSize();
	HotspotDetail *ptDetail = new HotspotDetail[count];
	int i;
	for (i = 0; i < count; i++) {
		// リストの中のホットスポットIDを取得
		int ptId = mainWindow->getHotspotIdListId(i);
		double latitude, longitude;
		xy2bl(allHotspot[ptId].y, allHotspot[ptId].x, toRad(gpsBottom),
				toRad(gpsLeft), toRad(gpsTop), toRad(gpsRight), &latitude,
				&longitude);
		ptDetail[i].longitude = toDeg(longitude);
		ptDetail[i].latitude = toDeg(latitude);
		ptDetail[i].panelName = getPanelName(allHotspot[ptId].x,
				allHotspot[ptId].y);

		ptDetail[i].temperature = allHotspotTemp[ptId];
		//ポインタで取得した文字列をコピーし、コピーしたものを参照する。
		//（ポインタの中身が書き変わっている可能性があるため。）

		ptDetail[i].fileNameIr = clone(getfileNameIr(i));
		ptDetail[i].fileNameVb = clone(getfileNameVb(i));
		// (2017/5/31YM)パネル平均温度出力を追加
		// (2017/6/2YM)パネル最高温度出力を追加
		HotspotNumber hn;
		hn = getAllHotspotNo(ptId);
		int dataId = hn.pictureNo;
		ptDetail[i].panelmaxtemp = (double) PanelmaxTemp[dataId];
		ptDetail[i].panelavetemp = (double) PanelaveTemp[dataId];
		// (2017/5/31YM)閾値温度出力を追加
		ptDetail[i].thresholdtemp = (double) mainWindow->getThresholdTemp();// (2017/5/31YM)閾値温度を格納;
	}
	return ptDetail;
}

bool ResultData::saveHotspotDetail(const TCHAR *filePath) {
	// データをファイルに保存する
	FILE *file = fileOpen(filePath, TEXT("w"));
	if (file == NULL) {
		return false;
	}

	HotspotDetail *ptDetail = getHotspotListDetail();
	int count = mainWindow->getHotspotIdListSize();
	for (int i = 0; i < count; i++) {
		// 内部コード文字列をUTF-8文字列に変換する
		char *panelName = toUTF8String(ptDetail[i].panelName);
		char *fileNameIr = toUTF8String(ptDetail[i].fileNameIr);
		char *fileNameVb = toUTF8String(ptDetail[i].fileNameVb);

		// ファイルに書き込む
		// (2017/6/2YM)報告書データを追加
		//　閾値データを計算
		double Thre = (ptDetail[i].panelavetemp) + ptDetail[i].thresholdtemp;
		fprintf(file,
				"longitude:%lf,latitude:%lf,panelName:%s,temp:%lf,fileNameIr:%s,fileNameVb:%s,maxTemp:%f,aveTemp:%f,Threshold:%f\n",
				ptDetail[i].longitude, ptDetail[i].latitude, panelName,
				ptDetail[i].temperature, fileNameIr, fileNameVb,
				ptDetail[i].panelmaxtemp, ptDetail[i].panelavetemp, Thre);

		delete panelName;
		delete fileNameIr;
		delete fileNameVb;
	}
	fclose(file);

	// 後片付け
	for (int i = 0; i < count; i++) {
		delete ptDetail[i].fileNameIr;
		delete ptDetail[i].fileNameVb;
	}
	delete ptDetail;

	return true;
}

bool ResultData::saveHotspotDetail(void) {
	TCHAR *str = getHotspotDetail();
	return saveHotspotDetail(str);
}

TCHAR* ResultData::getHotspotDetail(void) {
	TCHAR *str = getStringBuffer();
	stprintf(str, TEXT("%s\\Result_Report2_Data.txt"), cachePath);
	return str;
}

TCHAR* ResultData::getfilePath(void) {
	TCHAR *str = getStringBuffer();
	stprintf(str, TEXT("%s\\filePath.txt"), cachePath);
	return str;
}

TCHAR* ResultData::getFileHotspotIdList(void) {
	TCHAR *str = getStringBuffer();
	stprintf(str, TEXT("%s\\Hotspot_Id_List.txt"), cachePath);
	return str;
}

TCHAR* ResultData::getfileNameIr(int id) {
	TCHAR *str = getStringBuffer();
	stprintf(str, TEXT("%s\\HD%04dIR.jpg"), cachePath, id);
	return str;
}
// (2020/01/20LEE) => HSPICTUREIR save
TCHAR* ResultData::selcetIR(void) {
	TCHAR *str = getStringBuffer();
	stprintf(str, TEXT("%s\\HSPICTUREIR.jpg"), cachePath);
	return str;
}

TCHAR* ResultData::getfileNameVb(int id) {
	TCHAR *str = getStringBuffer();
	stprintf(str, TEXT("%s\\HD%04dVB.jpg"), cachePath, id);
	return str;
}

//俯瞰合成画像
TCHAR* ResultData::getfileWholeIr(void) {
	TCHAR *str = getStringBuffer();
	stprintf(str, TEXT("%s\\infrared.png"), cachePath);
	return str;
}

TCHAR* ResultData::getfileWholeVb(void) {
	TCHAR *str = getStringBuffer();
	stprintf(str, TEXT("%s\\visible.png"), cachePath);
	return str;
}

void ResultData::saveFlightData(void) {
	// データをファイルに保存する
	FILE *file = fileOpen(this->getFlightDataFileName(), TEXT("w"));
	if (file == NULL) {
		return;
	}

	for (std::vector<GPSPictureInfo>::iterator item = data.begin();
			item != data.end(); item++) {
		fprintf(file, "%04d/%02d/%02d "
				"%02d:%02d:%02d.%06d -- "
				"Acc: %lg %lg %lg, "
				"Gyr: %lg %lg %lg, "
				"Mag: %lg %lg %lg, "
				"iTOW: %d, "
				"Latitude: %lf, Longitude: %lf, Height: %lg, "
				"gpsFixOk: %d, %s\n", item->time.tm_year + 1900,
				item->time.tm_mon + 1, item->time.tm_mday, item->time.tm_hour,
				item->time.tm_min, item->time.tm_sec, item->tm_usec,
				item->accel.x, item->accel.y, item->accel.z, item->gyro.x,
				item->gyro.y, item->gyro.z, item->mag.x, item->mag.y,
				item->mag.z, item->gps.iTOW, item->gps.latitude,
				item->gps.longitude, item->gps.height, item->gps.gpsOk,
				"3D-fix");
	}
	fclose(file);
}

// (2017/4/4YM)バッファストリングを取得するグローバル関数を追加
/**
 * 内部バッファのポインタを取得する。
 */
TCHAR* ResultData::getStringBufferPublic(void) {
	memset(stringBuffer, 0, stringBufferSize * sizeof(TCHAR));
	return stringBuffer;
}

//　(2017/4/4YM)画像位置を初期化する関数を追加
void ResultData::ResetDefaultInternalPositionInfo(int id) {
	getXY(data[id].gps.latitude, data[id].gps.longitude, &posData[id].y,
			&posData[id].x);
}

// (2017/5/25YM)温度データ読み込み関数を追加
int ResultData::loadTempdata(void) {
	return loadTempdata(getTempDataFileName());
}

// (2017/5/25YM)温度データ読み込み関数を追加
int ResultData::loadTempdata(TCHAR *filePath) {

	// (2017/6/1YM)閾値データ初期化,
	mainWindow->setThresholdTemp(-1);

	// 温度情報をクリアする
	for (int i = 0; i < (int) getDataCount(); i++) {
		PanelmaxTemp[i] = NAN;
		PanelminTemp[i] = NAN;
		PanelaveTemp[i] = NAN;
	}

	// ファイルを開く
	FILE *f = fileOpen(filePath, TEXT("r"));
	if (f == NULL) {
		return false;
	}

	//　戻り値セット
	bool retval = true;

	// (2017/6/1YM)閾値データを読み込む
	float thre;
	int ret = fscanf(f, "threshold:%f\n", &thre);
	if (ret == 1) {
		mainWindow->setThresholdTemp((double) thre);
	}

	// ファイルを読み込む
	while (!feof(f)) {
		int id;
		float maxTemp, minTemp, aveTemp;
		int ret = fscanf(f, "id:%d,max:%f,min:%f,ave:%f\n", &id, &maxTemp,
				&minTemp, &aveTemp);
		// 旧バージョンの互換性のため、
		// 最高温度が異常値の場合も温度データを無効にする。
		if (ret != 4 || maxTemp < -273.15) {
			// データ形式が異なる場合は無視して次の行に進む。
			while (!feof(f) && fgetc(f) != '\n') {
			}
			continue;
		}
		// データ格納
		PanelmaxTemp[id] = maxTemp;
		PanelminTemp[id] = minTemp;
		PanelaveTemp[id] = aveTemp;
	}
	fclose(f);

	return retval;
}

// (2017/5/25YM)最高・最低・平均温度をセットする関数を追加
void ResultData::setTemp(void) {
	int count = getDataCount();

	float maxTemp = -INFINITY;
	float minTemp = INFINITY;

	// 変数宣言
	double sum = 0;
	int sumCount = 0;

	for (int i = 0; i < count; i++) {
		if (!isnan(PanelmaxTemp[i]) && PanelmaxTemp[i] > maxTemp) {
			maxTemp = PanelmaxTemp[i];
		}
		if (!isnan(PanelminTemp[i]) && PanelminTemp[i] < minTemp) {
			minTemp = PanelminTemp[i];
		}
		if (!isnan(PanelaveTemp[i])) {
			sum += PanelaveTemp[i];
			sumCount++;
		}
	}
	// データがない場合はNANにする
	if (isinf(maxTemp)) {
		maxTemp = NAN;
	}
	if (isinf(minTemp)) {
		minTemp = NAN;
	}

	PNmaxTemp = maxTemp;
	PNminTemp = minTemp;
	PNaveTemp = (float) (sum / sumCount);
}

// (2017/5/29YM)パネル平均温度を保存する処理を追加
int ResultData::savePanelTemp(TCHAR *filePath) {
	// データをファイルに保存する
	FILE *file = fileOpen(filePath, TEXT("w"));
	if (file == NULL) {
		return false;
	}

	// (2017/6/1YM)閾値データを書き込み
	// ファイルに書き込む
	fprintf(file, "threshold:%f\n", mainWindow->getThresholdTemp());

	int count = getDataCount();
	for (int i = 0; i < count; i++) {
		// ファイルに書き込む
		double max = PanelmaxTemp[i];
		double min = PanelminTemp[i];
		double ave = PanelaveTemp[i];
		fprintf(file, "id:%d,", i);
		// 各値がNaNの場合、意図した値が出力されない場合があるので、
		// 強制的にNaNを出力する。
		if (!isnan(max)) {
			fprintf(file, "max:%lf,", max);
		} else {
			fprintf(file, "max:NaN,");
		}
		if (!isnan(min)) {
			fprintf(file, "min:%lf,", min);
		} else {
			fprintf(file, "min:NaN,");
		}
		if (!isnan(ave)) {
			fprintf(file, "ave:%lf\n", ave);
		} else {
			fprintf(file, "ave:NaN\n");
		}
	}
	fclose(file);

	return true;

}

bool ResultData::saveAddData(const TCHAR *filePath) {
	// データをファイルに保存する
	FILE *file = fileOpen(filePath, TEXT("w"));
	if (file == NULL) {
		return false;
	}

	// ファイルに書き込む
	double Thre = PNaveTemp + mainWindow->getThresholdTemp();
	fprintf(file, "%d\n%d\n%f\n%f\n", getDataCount(),			//　撮影枚数
			getAllHotspotCount(),	//　ホットスポット数
			PNaveTemp,				//　パネル全体平均温度
			Thre);					//　異常判定温度
	fclose(file);

	return true;
}

//　(2017/6/14YM)ホットスポットデータ変数クリアチェック関数追加
void ResultData::CheckHSClearData(void) {
	//　ホットスポットデータクリアチェック
	if (hotspots.size() == 0) {
		hotspots.clear();
		std::vector<HotspotList>().swap(hotspots);
	}

}

void ResultData::setPanelDetection(bool mode) {
	panelDetection = mode;
}

bool ResultData::getPanelDetection(void) {
	return panelDetection;
}

// (2017/7/31YM)ホットスポット温度を返す関数を追加
float ResultData::getHotspotTemp(int id) {
	return allHotspotTemp[id];
}

// (2017/7/31YM)パネル平均温度を返す関数を追加
float ResultData::getPanelAveTemp(int id) {
	HotspotNumber hn = getAllHotspotNo(id);
	return PanelaveTemp[hn.pictureNo];
}

// (2017/7/31YM)全ホットスポット№を返す関数を追加
int ResultData::getHotspotID(int dataID, int spotID) {
	int HSNO = -1;
	HotspotNumber hn;

	for (int i = 0; i < (int) allHotspotNo.size(); i++) {
		hn = getAllHotspotNo(i);
		if ((hn.pictureNo == dataID) && (hn.pointNo == spotID)) {
			HSNO = i;
			break;
		}
	}

	return HSNO;
}

bool ResultData::getErrorcheck() {
	return Errornumcheck;
}

int ResultData::getpiccount() {
	return piccount;
}

int ResultData::getpicnum(int i) {
	return errorpicnum[i];
}

int ResultData::getErrornumber(int picnum) {
	if (data[picnum - 1].gps.longitude <= 0) {
		if (data[picnum - 1].gps.latitude <= 0) {
			if (data[picnum - 1].gps.height <= 0) {
				return GPS_ERROR_ALL;
			}
			return GPS_ERROR_LONGITUDE_LATITUDE;
		}
		if (data[picnum - 1].gps.height <= 0) {
			return GPS_ERROR_LONGITUDE_HEIGHT;
		}
		return GPS_ERROR_LONGITUDE;
	} else if (data[picnum - 1].gps.latitude <= 0) {
		if (data[picnum - 1].gps.height <= 0) {
			return GPS_ERROR_LATITUDE_HEIGHT;
		}
		return GPS_ERROR_LATITUDE;
	} else if (data[picnum - 1].gps.height <= 0) {
		return GPS_ERROR_HEIGHT;
	} else {
		return 0;
	}
}

bool ResultData::isPanelDetected() {
	return panelIsDetected;
}

void ResultData::updatePanelDetected() {
	// パネル検出済みかを取得する
	std::vector<TCHAR*> files_pn;
	searchFiles(dataPath, TEXT("t*pn.jpg"), files_pn);
	panelIsDetected = !files_pn.empty();
	searchFiles_cleanup(files_pn);
}

bool ResultData::hasVisible() {
	std::vector<TCHAR*> files;
	bool result = true;

	// TIFFファイルがあるか確認
	searchFiles(dataPath, TEXT("*.tif"), files);
	for (std::vector<TCHAR*>::iterator it = files.begin(); it != files.end();
			it++) {
		TiffInfo info;
		LPCTSTR extension = PathFindExtension(*it);
		if (_wcsicmp(extension, TEXT(".tif")) == 0
				|| _wcsicmp(extension, TEXT(".tiff")) == 0) {
			TCHAR filePath[lstrlen(dataPath) + lstrlen(*it) + 2];
			stprintf(filePath, TEXT("%s\\%s"), dataPath, *it);
			if (readTiffFile(filePath, &info)) {
				if (checkCameraModel(&info, "DJI", "XT2")
						|| checkCameraModel(&info, "FLIR", "Duo Pro R")) {
					result = true;
				} else {
					result = false;
				}
				destroyTiffInfo(&info);
				break;
			}
		}
	}
	searchFiles_cleanup(files);
	return result;
}

TCHAR* ResultData::getPanelName(double x, double y) {
	return panelData->getPanelNameDetail(x, y);
}

inline cv::Point getValueAsCvPoint(picojson::value &value) {
	cv::Point point;
	double x = 0, y = 0;
	if (value.is<picojson::value::array>()) {
		picojson::value::array pointValue = value.get<picojson::value::array>();
		if (pointValue.size() != 2 || !pointValue[0].is<double>()
				|| !pointValue[1].is<double>()) {
			debug("Invalid point array format");
			throw ParseError();
		}
		x = pointValue[0].get<double>();
		y = pointValue[1].get<double>();
	} else if (value.is<picojson::value::object>()) {
		picojson::value xValue = value.get("x");
		picojson::value yValue = value.get("y");
		if (!xValue.is<double>() || !yValue.is<double>()) {
			debug("Invalid point object format");
			throw ParseError();
		}
		x = xValue.get<double>();
		y = yValue.get<double>();
	} else {
		debug("Invalid point format");
		throw ParseError();
	}
	point.x = x;
	point.y = y;
	return point;
}

inline std::vector<cv::Point> getValueAsCvPointQuadrangle(
		picojson::value &value) {
	if (!value.is<picojson::value::array>()) {
		debug("Not a array");
		throw ParseError();
	}
	picojson::value::array pointArray = value.get<picojson::value::array>();
	if (pointArray.size() != 4) {
		debug("Not a quadrangle");
		throw ParseError();
	}
	std::vector<cv::Point> result;
	for (picojson::value::array::iterator point = pointArray.begin();
			point != pointArray.end(); point++) {
		result.push_back(getValueAsCvPoint(*point));
	}
	return result;
}

inline std::vector<std::vector<cv::Point> > getValueAsCvPointQuadrangleList(
		picojson::value &value) {
	std::vector<std::vector<cv::Point> > result;
	if (!value.is<picojson::array>()) {
		debug("Not a array");
		throw ParseError();
	}
	picojson::array panelsValue = value.get<picojson::array>();
	for (picojson::array::iterator panel = panelsValue.begin();
			panel != panelsValue.end(); panel++) {
		result.push_back(getValueAsCvPointQuadrangle(*panel));
	}
	return result;
}

String joinPath(LPCTSTR dirPath, LPCTSTR fileName) {
	String filePath(dirPath);
	filePath += TEXT("\\");
	filePath += fileName;
	return filePath;
}

picojson::value readJsonFile(String filePath) {
	picojson::value value;
	std::stringstream input;
	FILE *file = _wfopen(filePath.c_str(), L"rb");
	if (file != NULL) {
		char buf[1024];
		while (!feof(file)) {
			size_t len = fread(buf, sizeof(char), 1024, file);
			if (len > 0) {
				input << std::string(buf, len);
			}
		}
		fclose(file);
	}
	picojson::parse(value, input);
	return value;
}

void writeJsonFile(String filePath, picojson::value &value) {
	FILE *file = _wfopen(filePath.c_str(), L"wb");
	if (file != NULL) {
		std::string serialized = value.serialize();
		fwrite(serialized.c_str(), sizeof(char), serialized.size(), file);
		fclose(file);
	}
}

inline std::vector<std::vector<cv::Point> > getPanels(int id,
		picojson::value &value) {
	// 画像情報をチェックする。
	if (!value.is<picojson::object>() || !value.contains("images")) {
		throw ParseError();
	}
	picojson::value images = value.get("images");
	if (!images.is<picojson::array>()) {
		throw ParseError();
	}
	picojson::array imageArray = images.get<picojson::array>();
	for (picojson::array::iterator image = imageArray.begin();
			image != imageArray.end(); image++) {
		if (!image->is<picojson::object>() || !image->contains("id")) {
			continue;
		}
		picojson::value idValue = image->get("id");
		if (!idValue.is<double>() || ((int) idValue.get<double>()) != id) {
			continue;
		}
		if (image->contains("panels")) {
			return getValueAsCvPointQuadrangleList(image->get("panels"));
		} else {
			throw ParseError();
		}
	}
	throw ParseError();
}

/**
 * 指定したIDの画像のパネルを取得する。
 *
 * @param id 指定したID
 * @param panels パネル一覧
 * @returns パネルの取得に成功した場合はtrue、失敗した場合はfalseを返す。
 */
bool ResultData::getPanels(int id,
		std::vector<std::vector<cv::Point> > &panels) {
	String dataFilePath = joinPath(dataPath, TEXT("ResultData.json"));
	Mutex mutex(TEXT("dronepvwatch::ResultData"));
	mutex.wait();

	try {
		// パネル一覧のクリアする。
		panels.clear();

		// ファイルを解析して取得する。
		picojson::value root = readJsonFile(dataFilePath.data());

		// 指定されたパネルデータを取得する。
		panels = ::getPanels(id, root);

		mutex.unlock();
		return true;
	} catch (ParseError &e) {
		// 読み込みに失敗した場合はデータを空にする。
		mutex.unlock();
		return false;
	}
}

static bool compareById(const picojson::value &e1, const picojson::value &e2) {
	double v1 = DBL_MAX, v2 = DBL_MAX;
	if (e1.is<picojson::object>() && e1.contains("id")
			&& e1.get("id").is<double>()) {
		v1 = e1.get("id").get<double>();
	}
	if (e2.is<picojson::object>() && e2.contains("id")
			&& e2.get("id").is<double>()) {
		v2 = e2.get("id").get<double>();
	}
	return v1 < v2;
}

/**
 * 指定したIDの画像のパネルを設定する。
 *
 * @param id 指定したID
 * @param panels パネル一覧
 */
void ResultData::setPanels(int id,
		const std::vector<std::vector<cv::Point> > &panels) {
	String dataFilePath = joinPath(dataPath, TEXT("ResultData.json"));
	Mutex mutex(TEXT("dronepvwatch::ResultData"));
	mutex.wait();

	// ファイルを解析して取得する。
	picojson::value root = readJsonFile(dataFilePath);

	// JSONファイルがオブジェクトでない場合、オブジェクトにする。
	if (!root.is<picojson::object>()) {
		root.set(picojson::object());
	}

	// imagesプロパティがない場合は追加する。
	picojson::object rootObj = root.get<picojson::object>();
	if (!root.contains("images")) {
		rootObj.insert(rootObj.end(),
				std::pair<std::string, picojson::value>("images",
						picojson::value()));
		root.set(rootObj);
	}

	// imageプロパティが配列でない場合、配列にする。
	picojson::value images = root.get("images");
	if (!images.is<picojson::array>()) {
		images.set(picojson::array());
	}

	// パネルのJSON配列データを作成する。
	picojson::array newPanelData;
	for (std::vector<std::vector<cv::Point> >::const_iterator panel =
			panels.begin(); panel != panels.end(); panel++) {
		picojson::array newPanel;
		for (std::vector<cv::Point>::const_iterator point = panel->begin();
				point != panel->end(); point++) {
			picojson::array newPoint;
			newPoint.push_back(picojson::value((double) point->x));
			newPoint.push_back(picojson::value((double) point->y));
			newPanel.push_back(picojson::value(newPoint));
		}
		newPanelData.push_back(picojson::value(newPanel));
	}

	// 保存されている画像情報を検索し、新しいパネルデータに置き換える。
	picojson::array::iterator image;
	picojson::array imageArray = images.get<picojson::array>();
	for (image = imageArray.begin(); image != imageArray.end(); image++) {
		// プロパティidを含まない場合は次の要素を調べる。
		if (!image->is<picojson::object>() || !image->contains("id")) {
			continue;
		}
		// プロパティidが指定したIDと一致しない場合は次の要素を調べる。
		picojson::value idValue = image->get("id");
		if (!idValue.is<double>() || ((int) idValue.get<double>()) != id) {
			continue;
		}
		picojson::object imageObject = image->get<picojson::object>();
		if (!image->contains("panels")) {
			// プロパティpanelsが含まれていない場合は追加する。
			imageObject.insert(imageObject.end(),
					std::pair<std::string, picojson::value>("panels",
							picojson::value(newPanelData)));
		} else {
			// プロパティpanelsを更新する。
			imageObject["panels"].set(newPanelData);
		}
		image->set(imageObject);
		break;
	}
	// 画像情報がない場合は新規作成する。
	if (image == imageArray.end()) {
		picojson::object imageObject;
		imageObject.insert(imageObject.end(),
				std::pair<std::string, picojson::value>("id",
						picojson::value((double) id)));
		imageObject.insert(imageObject.end(),
				std::pair<std::string, picojson::value>("panels",
						picojson::value(newPanelData)));
		imageArray.push_back(picojson::value(imageObject));
	}
	// ID順に並び替える。
	std::sort(imageArray.begin(), imageArray.end(), compareById);

	// 画像データを更新する。
	rootObj["images"].set(imageArray);
	root.set(rootObj);

	writeJsonFile(dataFilePath, root);
}
