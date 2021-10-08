/*
 * tiff.h
 *
 *  Created on: 2021/08/10
 *      Author: Kiyoshi TASAKI <k.tasaki@effect-effect.com>
 */

#ifndef SOLARCOPTER_TIFF_H_
#define SOLARCOPTER_TIFF_H_

#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * TIFFファイル情報.
 */
typedef struct {
	/**
	 * GPSセンサー情報.
	 */
	struct {
		/**
		 * 緯度(北緯).
		 */
		double latitude;
		/**
		 * 経度(東経).
		 */
		double longitude;
		/**
		 * 高度.
		 */
		double altitude;
	} gps;
	/**
	 * 撮影時刻.
	 */
	struct tm time;
	/**
	 * 撮影時刻(秒未満).
	 */
	int tm_usec;
	/**
	 * 画像データ.
	 */
	const unsigned char *imageData;
	/**
	 * 画像データサイズ.
	 */
	unsigned int imageDataSize;
	/**
	 * 幅.
	 */
	unsigned short width;
	/**
	 * 高さ.
	 */
	unsigned short height;
	/**
	 * 製造者.
	 */
	const char *manufacturer;
	/**
	 * 製造者.
	 */
	const char *model;
	/**
	 * XMP.
	 */
	const char *xmp;
	/**
	 * 元データ.
	 */
	const unsigned char *rawData;
	/**
	 * 元データのサイズ.
	 */
	size_t rawDataSize;
} TiffInfo;

/**
 * TIFFファイルを読み込む.
 */
int readTiffFile(const wchar_t *fileName, TiffInfo *tiffInfo);

/**
 * TIFF情報を破棄する.
 */
void destroyTiffInfo(TiffInfo *tiffInfo);

#ifdef __cplusplus
}
#endif

#endif /* SOLARCOPTER_TIFF_H_ */
