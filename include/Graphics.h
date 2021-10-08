/*
 * Graphics.h
 *
 *  Created on: 2015/10/21
 *      Author: effectet
 */

#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <windows.h>
#include <gdiplus.h>

#include <opencv2/opencv.hpp>

namespace Graphics {

Gdiplus::Bitmap* stretchBitmap(Gdiplus::Bitmap *srcImage, double ratio);
Gdiplus::Bitmap* rotateBitmap(Gdiplus::Bitmap *srcImage, double angle);
void setAlphaChannel(Gdiplus::Bitmap *srcImage, double alpha);
int* getBrightnessMap(int *srcImagePixels, int width, int height);
int* getHueMap(int *srcImagePixels, int width, int height);
Gdiplus::Bitmap* loadBitmap(const TCHAR *fileName, int picturetype,
		double ratio, double direction);

cv::Mat toCVMatrix(Gdiplus::Bitmap *srcImage);
cv::Mat loadCVImage(LPCTSTR path);
void saveCVImage(LPCTSTR path, cv::Mat &image);
int GetEncoderClsid(const wchar_t *format, CLSID *pClsid);

Gdiplus::Bitmap* trimBitmap(Gdiplus::Bitmap *srcImage, int x, int y, int width,
		int height);

Gdiplus::Bitmap* toBitmap(const cv::Mat *srcImage);

namespace Jpeg {
enum {
	START_OF_IMAGE = 0xFFD8,
	END_OF_IMAGE = 0xFFD9,
	EXIF = 0xFFE1,
	EXTENDED_DATA_1 = 0xFFEA,
	EXTENDED_DATA_2 = 0xFFEB
};
}

}

#include "CommonData.h"
bool isPointInPolygon(const POINT &pointTarget, const unsigned int uiCountPoint,
		const POINT *aPoint);
bool isPointInPolygon(const Vector2D &pointTarget,
		const unsigned int uiCountPoint, const Vector2D *aPoint);

#endif /* GRAPHICS_H_ */
