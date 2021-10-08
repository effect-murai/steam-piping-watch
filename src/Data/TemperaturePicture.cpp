/*
 * TemperaturePicture.cpp
 *
 *  Created on: 2020/06/11
 *      Author: k.tasaki
 */

#include "TemperaturePicture.h"

/**
 * 温度画像を作成する。
 * @param tempMap 温度マップ
 * @param width 温度マップの幅
 * @param height 温度マップの高さ
 * @param min 最低温度
 * @param max 最高温度
 * @param type 画像種別(0:グレースケール,1:カラー)
 * @returns 温度画像のBitmap
 */
Gdiplus::Bitmap* TemperaturePicture::create(float *tempMap, int width,
		int height, double min, double max, int type) {
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

	int COLOR_TONE;
	int TONE_MAX;
	COLORREF colorPattern[1024];

	if (type == GRAY_SCALE) {
		// 赤外線画像をグレースケールで作成する
		// 色のパターンをグレースケール(256階調)で作成する
		COLOR_TONE = 256;
		TONE_MAX = COLOR_TONE - 1;
		for (int tone = 0; tone < COLOR_TONE; tone++) {
			double percentage = (double) tone / TONE_MAX;
			int red = 0, green = 0, blue = 0;
			red = percentage * 255;
			green = blue = red;
			if (red > 255) {
				red = 255;
			} else if (red < 0) {
				red = 0;
			}
			if (green > 255) {
				green = 255;
			} else if (green < 0) {
				green = 0;
			}
			if (blue > 255) {
				blue = 255;
			} else if (blue < 0) {
				blue = 0;
			}
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
			if (red > 255) {
				red = 255;
			} else if (red < 0) {
				red = 0;
			}
			if (green > 255) {
				green = 255;
			} else if (green < 0) {
				green = 0;
			}
			if (blue > 255) {
				blue = 255;
			} else if (blue < 0) {
				blue = 0;
			}
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

	return image;
}
