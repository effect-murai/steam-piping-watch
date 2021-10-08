/*
 * TiffConverter.cpp
 *
 *  Created on: 2021/08/10
 *  Author: Kiyoshi TASAKI <k.tasaki@effect-effect.com>
 */

#include <Data/TiffConverter.h>
#include <Graphics.h>

#include <dirent.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <vector>
#include <windows.h>

#include "debug.h"

#include "tiff.h"

inline void writeFlightData(FILE *file, const TiffInfo *tiffInfo) {
	const struct tm *time = &tiffInfo->time;
	fprintf(file, "%04d/%02d/%02d "
			"%02d:%02d:%02d.%06d -- "
			"Acc: %lg %lg %lg, "
			"Gyr: %lg %lg %lg, "
			"Mag: %lg %lg %lg, "
			"iTOW: %d, "
			"Latitude: %.8lf, Longitude: %.8lf, Height: %lg, "
			"gpsFixOk: %d, %s\n", time->tm_year + 1900, time->tm_mon + 1,
			time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec,
			tiffInfo->tm_usec, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0,
			tiffInfo->gps.latitude, tiffInfo->gps.longitude,
			tiffInfo->gps.altitude, 1, "3D-fix");
}

inline void exportInfraredData(const wchar_t *fileName, TiffInfo *info) {
	size_t size = info->width * info->height;
	short buffer[size];
#	ifdef _OPENMP
#	pragma omp parallel for
#	endif
	for (size_t i = 0; i < size; i++) {
		buffer[i] = ((int) ((unsigned short*) info->imageData)[i] * 4) - 27315;
	}

	FILE *f = _wfopen(fileName, L"w+b");
	if (f != NULL) {
		int width = info->width, height = info->height;
		fwrite(&width, sizeof(int), 1, f);
		fwrite(&height, sizeof(int), 1, f);
		fwrite(buffer, size * sizeof(short), 1, f);
		fclose(f);
	}
}

inline bool createInfraredData(const wchar_t *path, size_t pathLen,
		std::wstring &ir, TiffInfo *tiffInfo, size_t index) {
	// 元の赤外線データファイルパスを取得する。
	wchar_t originalInfraredPath[pathLen + ir.size() + 2];
	swprintf(originalInfraredPath, L"%S\\%S", path, ir.c_str());
	if (!readTiffFile(originalInfraredPath, tiffInfo)) {
		error("readTiffFile: %S", originalInfraredPath);
		return false;
	}
	wchar_t newInfraredPath[pathLen + 16];
	swprintf(newInfraredPath, L"%S\\T%05dIR.dat", path, (int) index);
	exportInfraredData(newInfraredPath, tiffInfo);
	free((void*) tiffInfo->rawData);
	tiffInfo->rawData = NULL;
	return true;
}

inline bool createVisibleImage(const wchar_t *path, size_t pathLen,
		std::wstring &vb, size_t index) {
	// 元の可視光データファイルパスを取得する。
	wchar_t originalVisiblePath[pathLen + vb.size() + 2];
	swprintf(originalVisiblePath, L"%S\\%S", path, vb.c_str());
	// 新しい可視光データファイルパスを取得する。
	wchar_t newVisiblePath[pathLen + 16];
	swprintf(newVisiblePath, L"%S\\T%05dVB.jpg", path, (int) index);
	// 新しい名前でコピーする。
	return CopyFileW(originalVisiblePath, newVisiblePath, FALSE);
}

namespace TiffConverter {

int convert(const wchar_t *path, int *progress) {
	int progressMax = 1;
	int dummy = 0;
	if (progress == NULL) {
		progress = &dummy;
	}
	struct tm t;
	time_t timer = time(NULL);
	localtime_s(&t, &timer);

	// 対象のフォルダーを開き、ファイル一覧を作成する。
	_WDIR *dp = _wopendir(path);
	if (dp == NULL) {
		// フォルダーが開けなかった場合
		return TIFF_CONVERTER_OPEN_FOLDER_ERROR;
	}
	std::vector<std::wstring> infraredFileList;
	std::vector<std::wstring> visibleFileList;
	for (struct _wdirent *item = _wreaddir(dp); item != NULL;
			item = _wreaddir(dp)) {
		// ファイルの拡張子(.より後)を取得する。
		wchar_t *extension = NULL;
		for (int i = wcslen(item->d_name) - 1; i > 0; i--) {
			if (item->d_name[i] == L'.') {
				extension = &item->d_name[i + 1];
				break;
			}
		}
		// 拡張子がない場合は無視する。
		if (extension == NULL) {
			continue;
		}
		if (_wcsicmp(L"tiff", extension) == 0
				|| _wcsicmp(L"tif", extension) == 0) {
			// 拡張子が .tiff または .tif の場合は赤外線リストに追加する。
			infraredFileList.push_back(item->d_name);
		} else if (_wcsicmp(L"jpg", extension) == 0) {
			// 拡張子が .jpg の場合は可視光ファイルリストに追加する。
			visibleFileList.push_back(item->d_name);
		} else {
			// いずれでもでない場合は無視する。
			continue;
		}
	}
	_wclosedir(dp);

	if (infraredFileList.empty()) {
		// 赤外線データがない場合
		return TIFF_CONVERTER_NO_INFRARED_ERROR;
	}

	if (infraredFileList.size() >= 100000) {
		// データ数が多すぎる場合
		return TIFF_CONVERTER_TOO_MANY_INFRARED_ERROR;
	}

	const size_t flightDataCount = infraredFileList.size();

	unsigned int current = 1;
	progressMax = flightDataCount * 2 + 2;
	*progress = 100 / progressMax;

	TiffInfo flightData[flightDataCount];
	memset(flightData, 0, sizeof(TiffInfo) * flightDataCount);

	const size_t pathLen = wcslen(path);

	if (!visibleFileList.empty()) {
		// 可視光データがある場合

		if (flightDataCount != visibleFileList.size()) {
			// 赤外線データと可視光データの個数が異なる場合
			return TIFF_CONVERTER_VISIBLE_COUNT_ERROR;
		}

		const size_t pathLen = wcslen(path);
		int result = TIFF_CONVERTER_SUCCESS;
#		ifdef _OPENMP
#		pragma omp parallel for
#		endif
		for (size_t i = 0; i < flightDataCount; i++) {
			if (result != TIFF_CONVERTER_SUCCESS) {
				continue;
			}
			// 赤外線データを生成する。
			if (!createInfraredData(path, pathLen, infraredFileList[i],
					&flightData[i], i + 1)) {
				result = TIFF_CONVERTER_INVALID_INFRARED_ERROR;
				continue;
			}
			// 可視光データを生成する。
			if (!createVisibleImage(path, pathLen, visibleFileList[i], i + 1)) {
				result = TIFF_CONVERTER_INVALID_VISIBLE_ERROR;
				continue;
			}
			*progress = (++current) * 100 / progressMax;
		}
		if (result != TIFF_CONVERTER_SUCCESS) {
			debug("Result: %d", result);
			return result;
		}
	} else {
		// 可視光データがない場合
		int result = TIFF_CONVERTER_SUCCESS;
#		ifdef _OPENMP
#		pragma omp parallel for
#		endif
		for (size_t i = 0; i < flightDataCount; i++) {
			if (result != TIFF_CONVERTER_SUCCESS) {
				continue;
			}
			// 赤外線データを生成する。
			if (!createInfraredData(path, pathLen, infraredFileList[i],
					&flightData[i], i + 1)) {
				result = TIFF_CONVERTER_INVALID_INFRARED_ERROR;
				debug("result: %S", infraredFileList[i].c_str());
				continue;
			}
			// 空の可視光データを生成する。
			wchar_t visibleFile[pathLen + 16];
			swprintf(visibleFile, L"%S\\T%05dVB.jpg", path, (int) (i + 1));
			cv::Mat img(1200, 1600, CV_8UC1);
			Graphics::saveCVImage(visibleFile, img);
			*progress = (++current) * 100 / progressMax;
		}
		if (result != TIFF_CONVERTER_SUCCESS) {
			debug("Result: %d", result);
			return result;
		}
	}

	wchar_t filePath[wcslen(path) + 40];
	swprintf(filePath, L"%S\\WatchOn_Result%04d%02d%02d%02d%02d%02d.txt", path,
			t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min,
			t.tm_sec);

	FILE *watchOn = _wfopen(filePath, L"w+");
	if (watchOn == NULL) {
		return TIFF_CONVERTER_CREATE_FLIGHT_DATA_ERROR;
	}
	memcpy(&t, &flightData[0].time, sizeof(struct tm));
	fprintf(watchOn, "%04d/%02d/%02d %02d:%02d:%02d\n", t.tm_year + 1900,
			t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	for (size_t i = 0; i < flightDataCount; i++) {
		writeFlightData(watchOn, &flightData[i]);
		*progress = (++current) * 100 / progressMax;
	}
	fclose(watchOn);

	return TIFF_CONVERTER_SUCCESS;
}

}
