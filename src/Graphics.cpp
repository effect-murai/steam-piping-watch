/*
 * Graphics.cpp
 *
 *  Created on: 2015/10/21
 *      Author: effect
 */

#include "Graphics.h"
#include <math.h>
#include <stdio.h>
#include <algorithm>
#ifdef _OPENMP
#include <omp.h>
#endif
#include <opencv2/opencv.hpp>
#include "StringUtils.h"
#include "FileUtils.h"

template<typename _Tp>
bool isPointInPolygon(const _Tp &pointTarget, const unsigned int uiCountPoint,
		const _Tp *aPoint) {
	int countCrossing = 0;

	_Tp point0 = aPoint[0];
	bool flag0x = (pointTarget.x <= point0.x);
	bool flag0y = (pointTarget.y <= point0.y);

	// レイの方向は、Ｘプラス方向
	for (unsigned int ui = 1; ui < uiCountPoint + 1; ui++) {
		_Tp point1 = aPoint[ui % uiCountPoint];	// 最後は始点が入る（多角形データの始点と終点が一致していないデータ対応）
		bool bFlag1x = (pointTarget.x <= point1.x);
		bool bFlag1y = (pointTarget.y <= point1.y);
		if (flag0y != bFlag1y) {
			// 線分はレイを横切る可能性あり。
			if (flag0x == bFlag1x) {
				// 線分の２端点は対象点に対して両方右か両方左にある
				if (flag0x) {
					// 完全に右。⇒線分はレイを横切る
					countCrossing += (flag0y ? -1 : 1);	// 上から下にレイを横切るときには、交差回数を１引く、下から上は１足す。
				}
			} else {
				// レイと交差するかどうか、対象点と同じ高さで、対象点の右で交差するか、左で交差するかを求める。
				if (pointTarget.x
						<= (point0.x
								+ (point1.x - point0.x)
										* (pointTarget.y - point0.y)
										/ (point1.y - point0.y))) {
					// 線分は、対象点と同じ高さで、対象点の右で交差する。⇒線分はレイを横切る
					countCrossing += (flag0y ? -1 : 1);	// 上から下にレイを横切るときには、交差回数を１引く、下から上は１足す。
				}
			}
		}
		// 次の判定のために、
		point0 = point1;
		flag0x = bFlag1x;
		flag0y = bFlag1y;
	}

	// クロスカウントがゼロのとき外、ゼロ以外のとき内。
	return 0 != countCrossing;
}

/**
 * 指定した点が多角形内にあるか調べる。
 * @param pointTarget 調査する点
 * @param uiCountPoint 多角形の頂点数
 * @param aPoint 点が格納された配列
 * @return true:多角形内/false:多角形外
 */
bool isPointInPolygon(const POINT &pointTarget, const unsigned int uiCountPoint,
		const POINT *aPoint) {
	return isPointInPolygon<POINT>(pointTarget, uiCountPoint, aPoint);
}

/**
 * 指定した点が多角形内にあるか調べる。
 * @param pointTarget 調査する点
 * @param uiCountPoint 多角形の頂点数
 * @param aPoint 点が格納された配列
 * @return true:多角形内/false:多角形外
 */
bool isPointInPolygon(const Vector2D &pointTarget,
		const unsigned int uiCountPoint, const Vector2D *aPoint) {
	return isPointInPolygon<Vector2D>(pointTarget, uiCountPoint, aPoint);
}

namespace Graphics {
// helper function
int GetEncoderClsid(const wchar_t *format, CLSID *pClsid) {

	UINT num = 0;          // number of image encoders
	UINT size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo *pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0) {
		return -1;  // Failure
	}
	pImageCodecInfo = (Gdiplus::ImageCodecInfo*) malloc(size);
	if (pImageCodecInfo == NULL) {
		return -1;
	}

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j) {
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure

}

/**
 * OpenCV Matrix形式で画像を読み込む
 */
cv::Mat loadCVImage(LPCTSTR path) {
#	ifdef UNICODE
	wchar_t *wcPath = (wchar_t *)path;
#	else
	wchar_t *wcPath = fromOEMCodePageString(path);
#	endif // UNICODE
	Gdiplus::Bitmap *bitmap = new Gdiplus::Bitmap(wcPath);
	cv::Mat image = toCVMatrix(bitmap);
	delete bitmap;
#	ifndef UNICODE
	delete wcPath;
#	endif // UNICODE
	return image;
}

/**
 * OpenCV Matrix形式の画像を出力する。
 */
void saveCVImage(LPCTSTR path, cv::Mat &image) {
#	ifdef UNICODE
	wchar_t *wcPath = (wchar_t *)path;
#	else
	wchar_t *wcPath = fromOEMCodePageString(path);
#	endif // UNICODE
	LPTSTR ext = getExt((LPTSTR)wcPath);
	CLSID imgClsid;
	if (wcsicmp(L"jpg", ext) == 0 || wcsicmp(L"jpeg", ext) == 0) {
		Graphics::GetEncoderClsid(L"image/jpeg", &imgClsid);
	} else {
		Graphics::GetEncoderClsid(L"image/png", &imgClsid);
	}
	Gdiplus::Bitmap *out = toBitmap(&image);
	out->Save(wcPath, &imgClsid, NULL);
	delete out;
#	ifndef UNICODE
	delete wcPath;
#	endif // UNICODE
}

/**
 * GDI+ビットマップオブジェクトからOpenCV Matrix形式に変換する
 * @todo 未実装
 */
cv::Mat toCVMatrix(Gdiplus::Bitmap *srcImage) {
	int width = srcImage->GetWidth();
	int height = srcImage->GetHeight();

	// 出力画像のピクセルデータを取得する
	Gdiplus::Rect rect;
	Gdiplus::BitmapData srcImageData;
	rect.X = 0;
	rect.Y = 0;
	rect.Width = width;
	rect.Height = height;
	srcImage->LockBits(&rect, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB,
			&srcImageData);
	int *const srcImagePixels = (int*) (srcImageData.Scan0);

	cv::Mat dstImage(cv::Size(width, height), CV_8UC3);
	const int elemSize = dstImage.elemSize();

	// 3チャンネル(RGB)の場合
#	ifdef _OPENMP
#	pragma omp parallel for
#	endif
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			size_t pos = dstImage.step * y + x * elemSize;
			unsigned int color = srcImagePixels[x + y * width];
			unsigned char blue = color & 0xff;
			unsigned char green = (color >> 8) & 0xff;
			unsigned char red = (color >> 16) & 0xff;
			dstImage.data[pos] = blue;
			dstImage.data[pos + 1] = green;
			dstImage.data[pos + 2] = red;
		}
	}

	// 取得したピクセルデータを解放する
	srcImage->UnlockBits(&srcImageData);

	return dstImage;
}

/**
 * ビットマップを読み込み、リサイズおよび回転を加える
 * @param fileName ファイル名(Unicode形式)
 * @param ratio 倍率(最大10倍まで)
 * @param direction 回転角度[rad]
 * @return Bitmapオブジェクト
 */
Gdiplus::Bitmap* loadBitmap(const TCHAR *fileName, int picturetype,
		double ratio, double direction) {
	if ((ratio < 0) || (ratio > 10)) {
		ratio = 1;
	}

	wchar_t *wcFileName;
#	ifndef UNICODE
	// パス長の最大まで確保する
	wcFileName = new wchar_t[MAX_PATH];
	MultiByteToWideChar(CP_OEMCP, MB_COMPOSITE, fileName, -1, wcFileName, MAX_PATH);
#	else // UNICODE
	// UNICODE版の場合はそのまま使える
	wcFileName = (wchar_t*) fileName;
#	endif // UNICODE

#	ifdef NO_OPENCV
	// OpenCVを利用しない
	Gdiplus::Bitmap* img = new Gdiplus::Bitmap(wcFileName);
#	else // NO_OPENCV
	// OpenCVを利用する
	cv::Mat img = loadCVImage(wcFileName);

	// (2019/11/21LEE)追加。
	if (picturetype == 0) {
		cv::Mat dst;			// (2019/11/15LEE) 追加。
		cv::resize(img, dst, cv::Size(1600, 1200), 0, 0, CV_INTER_NN);// (2019/11/15LEE) 追加。
		img = dst;			// (2019/11/15LEE) 追加。
	}

#	endif // NO_OPENCV

#	ifndef UNICODE
	delete wcFileName;
#	endif // UNICODE

#	ifdef NO_OPENCV
	// OpenCVを利用しない
	Gdiplus::Bitmap* img2 = img;
	if (ratio != 1) 
	{
		img2 = Graphics::stretchBitmap(img, ratio);
		delete img;
	}
	
	Gdiplus::Bitmap* imgRet = img2;
	if (direction != 0) 
	{
		imgRet = Graphics::rotateBitmap(img2, direction);
		delete img2;
	}
#	else // NO_OPENCV
	// OpenCVを利用する
	cv::Mat img2;
	if (ratio != 1) {
		int width = img.size().width * ratio;
		int height = img.size().height * ratio;
		if (width <= 0) {
			width = 1;
		}
		if (height <= 0) {
			height = 1;
		}
		cv::resize(img, img2, cv::Size(width, height));
	} else {
		img2 = img;
	}

#	ifndef NO_OPENCV_ROTATE
	Gdiplus::Bitmap *img3 = toBitmap(&img2);
	Gdiplus::Bitmap *imgRet = img3;
	if (direction != 0) {
		imgRet = Graphics::rotateBitmap(img3, direction);
		delete img3;
	}
#	else // NO_OPENCV_ROTATE
	float angle = -(direction * 180.0 / M_PI), scale = 1.0;
	// 中心：画像中心
	cv::Point2f center(img2.cols * 0.5, img2.rows * 0.5);
	// 以上の条件から2次元の回転行列を計算
	const cv::Mat affine_matrix = cv::getRotationMatrix2D(center, angle, scale);
	
	cv::Mat img3;
	cv::warpAffine(img2, img3, affine_matrix, img2.size());
	Gdiplus::Bitmap* imgRet = toBitmap(&img3);
#	endif // NO_OPENCV_ROTATE

#	endif // NO_OPENCV
	return imgRet;
}

/**
 * OpenCV Matrix から Bitmap へ変換する@n
 * 入力は1/3チャンネルカラー画像という前提でそれ以外は読めない
 * @param[in] srcImage 変換元画像(OpenCV Matrix形式)
 * @return Bitmapオブジェクト(32bitARGB)
 */
Gdiplus::Bitmap* toBitmap(const cv::Mat *srcImage) {
	Gdiplus::Bitmap *destImage;	// 出力画像
	Gdiplus::BitmapData dstImageData;
	Gdiplus::Rect rect;

	const int width = srcImage->size().width;
	const int height = srcImage->size().height;

	// 出力画像用のメモリ領域を確保する
	destImage = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);

	// 出力画像のピクセルデータを取得する
	rect.X = 0;
	rect.Y = 0;
	rect.Width = width;
	rect.Height = height;
	destImage->LockBits(&rect, Gdiplus::ImageLockModeWrite,
	PixelFormat32bppARGB, &dstImageData);
	int *const dstImagePixels = (int*) dstImageData.Scan0;

	const int elemSize = srcImage->elemSize();

	// チャンネル数判定
	// ループ内で判定すると遅くなるのでループ外で判定
	if (srcImage->channels() == 3) {
		// 3チャンネル(RGB)の場合
#		ifdef _OPENMP
#		pragma omp parallel for
#		endif
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				size_t pos = srcImage->step * y + x * elemSize;
				unsigned char b = srcImage->data[pos];
				unsigned char g = srcImage->data[pos + 1];
				unsigned char r = srcImage->data[pos + 2];
				unsigned int pixel = ((r << 16) + (g << 8) + b) | 0xff000000;
				dstImagePixels[x + y * width] = pixel;
			}
		}
	} else if (srcImage->channels() == 1) {
		// 1チャンネル(GrayScale)の場合
#		ifdef _OPENMP
#		pragma omp parallel for
#		endif
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				size_t pos = srcImage->step * y + x * elemSize;
				unsigned char v = srcImage->data[pos];
				unsigned int pixel = (v * 0x10101) | 0xff000000;
				dstImagePixels[x + y * width] = pixel;
			}
		}
	}

	// 取得したピクセルデータを解放する
	destImage->UnlockBits(&dstImageData);

	return destImage;
}

/**
 * 指定した画素の色を取得する(32bit RGB/ARGB)
 * @param[in] pixels 画像データが格納された配列
 * @param[in] width 画像の幅
 * @param[in] height 画像の高さ
 * @param[in] x 値を取得する画素のx座標
 * @param[in] y 値を取得する画素のy座標
 * @return 指定した画素の色
 */
inline int getPixel(int *pixels, int width, int height, int x, int y) {
	return pixels[x + y * width];
}

inline int getPixelBrightness(int *pixels, int width, int height, int x,
		int y) {
	union {
		int value;
		unsigned char channel[4];
	} color;
	color.value = pixels[x + y * width];
	return ((int) (29 * color.channel[0]) + (int) (150 * color.channel[1])
			+ (int) (77 * color.channel[2])) >> 8;
}

inline int getPixelHue(int *pixels, int width, int height, int x, int y) {
	union {
		int value;
		unsigned char channel[4];
	} color;
	color.value = pixels[x + y * width];
	const int blue = (int) color.channel[0];
	const int green = (int) color.channel[1];
	const int red = (int) color.channel[2];
	int max, min, type = 0;
	if (red > green) {
		max = red;
		type = 0;
	} else {
		max = green;
		type = 1;
	}
	if (max < blue) {
		max = blue;
		type = 2;
	}
	if (red < green) {
		min = red;
	} else {
		min = green;
	}
	if (min > blue) {
		min = blue;
	}
	double h;
	if (type == 0) {
		h = 60 * ((double) (green - blue) / (max - min));
	} else if (type == 1) {
		h = 60 * ((double) (blue - red) / (max - min)) + 120;
	} else {
		h = 60 * ((double) (red - green) / (max - min)) + 240;
	}
	return (int) (h / 360.0 * 255);
}

/**
 * 画像から輝度マップを作成する。
 * @param[in] srcImagePixels 元画像のピクセルデータ
 * @param[in] width 画像の幅
 * @param[in] height 画像の高さ
 * @return 輝度マップを格納した配列へのポインタ
 */
int* getBrightnessMap(int *srcImagePixels, int width, int height) {
	int *brightnessMap = new int[width * height];
#	ifdef _OPENMP
#	pragma omp parallel for
#	endif
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			brightnessMap[x + y * width] = getPixelBrightness(srcImagePixels,
					width, height, x, y);
		}
	}
	return brightnessMap;
}

/**
 * 画像から色相マップを作成する。
 * @param[in] srcImagePixels 元画像のピクセルデータ
 * @param[in] width 画像の幅
 * @param[in] height 画像の高さ
 * @return 色相マップを格納した配列へのポインタ
 */
int* getHueMap(int *srcImagePixels, int width, int height) {
	int *hueMap = new int[width * height];
#	ifdef _OPENMP
#	pragma omp parallel for
#	endif
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			hueMap[x + y * width] = getPixelHue(srcImagePixels, width, height,
					x, y);
		}
	}
	return hueMap;
}

/**
 * Bilinear補間で補正した色を取得する
 * @param[in] pixels 画像データが格納された配列
 * @param[in] width 画像の幅
 * @param[in] height 画像の高さ
 * @param[in] x 値を取得する画素のx座標
 * @param[in] y 値を取得する画素のy座標
 * @param[in] dx 値を取得する画素のx座標の小数部(1024倍した整数値)
 * @param[in] dy 値を取得する画素のy座標の小数部(1024倍した整数値)
 * @return 指定した画素の色
 */
inline int bilinearInterpolation(int *pixels, int width, int height, int x,
		int y, int dx, int dy) {
	// 端の色を取得する
	const int c1 = getPixel(pixels, width, height, x, y);
	const int c2 =
			(x < width - 1) ? getPixel(pixels, width, height, x + 1, y) : 0;
	const int c3 =
			(y < height - 1) ? getPixel(pixels, width, height, x, y + 1) : 0;
	const int c4 =
			((x < width - 1) && (y < height - 1)) ?
					getPixel(pixels, width, height, x + 1, y + 1) : 0;

	union {
		int value;
		unsigned char channel[4];
	} color;

	// 1 - dx、1 - dyを求める(※dx, dyは1024倍した整数値)
	const int ldx = 1024 - dx;
	const int ldy = 1024 - dy;

	// ARGBの各チャネルごとに計算する
	// 下位から順に(B→G→R→A)計算する
	for (int i = 0; i < 4; i++) {
		const int shiftNum = i * 8;
		// 各点のチャネル値を取得する
		const int _c1 = (c1 >> shiftNum) & 0xff;
		const int _c2 = (c2 >> shiftNum) & 0xff;
		const int _c3 = (c3 >> shiftNum) & 0xff;
		const int _c4 = (c4 >> shiftNum) & 0xff;

		// Bilinearでチャネル値を計算する
		const int _ch = ((ldy * (ldx * _c1 + dx * _c2)
				+ dy * (ldx * _c3 + dx * _c4)) >> 20) & 0xff;

		// 計算したチャネル値を結合する
		color.channel[i] = _ch;
	}

	// 計算後の色を返す
	return color.value;
}

/**
 * ビットマップの伸縮処理@n
 * CPUが処理するため極めて低速である。
 * 高速化のため補間は行わない。
 * @param[in] srcImage ビットマップの入力画像
 * @param[in] ratio 倍率
 * @return 処理後のビットマップ画像
 */
Gdiplus::Bitmap* stretchBitmap(Gdiplus::Bitmap *srcImage, double ratio) {
	Gdiplus::Bitmap *destImage;	//出力画像
	Gdiplus::BitmapData srcImageData;
	Gdiplus::BitmapData dstImageData;
	Gdiplus::Rect rect;

	// サイズを取得する
	const int SrcWidth = srcImage->GetWidth();
	const int SrcHeight = srcImage->GetHeight();

	// 出力画像のサイズを計算
	const int DstWidth = (int) (SrcWidth * ratio);
	const int DstHeight = (int) (SrcHeight * ratio);

	// 出力画像用のメモリ領域を確保する
	destImage = new Gdiplus::Bitmap(DstWidth, DstHeight, PixelFormat32bppARGB);

	// 入力画像のピクセルデータを取得する
	rect.X = 0;
	rect.Y = 0;
	rect.Width = SrcWidth;
	rect.Height = SrcHeight;
	srcImage->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB,
			&srcImageData);
	int *const srcImagePixels = (int*) (srcImageData.Scan0);

	// 出力画像のピクセルデータを取得する
	rect.X = 0;
	rect.Y = 0;
	rect.Width = DstWidth;
	rect.Height = DstHeight;
	destImage->LockBits(&rect, Gdiplus::ImageLockModeWrite,
	PixelFormat32bppARGB, &dstImageData);
	int *const dstImagePixels = (int*) (dstImageData.Scan0);

#	ifdef _OPENMP
#	pragma omp parallel for
#	endif
	for (int y2 = 0; y2 < DstHeight; y2++) {
		for (int x2 = 0; x2 < DstWidth; x2++) {
			// 出力画像座標(x2,y2)から入力画像の座標(x1,y1)を逆計算する
			int x1 = (x2 << 10) / ratio;
			int y1 = (y2 << 10) / ratio;

			const int dx = x1 & 0x3ff;
			const int dy = y1 & 0x3ff;

			x1 >>= 10;
			y1 >>= 10;

			// x1, y1がともに入力画像の有効範囲にあれば出力へコピーを行う
			dstImagePixels[x2 + y2 * DstWidth] = bilinearInterpolation(
					srcImagePixels, SrcWidth, SrcHeight, x1, y1, dx, dy);
		}
	}

	// 取得したピクセルデータを解放する
	srcImage->UnlockBits(&srcImageData);
	destImage->UnlockBits(&dstImageData);

	return destImage;
}

/**
 * ビットマップの回転処理@n
 * CPUが処理するため極めて低速である。
 * 高速化のため線形補間は行わない。
 * @param[in] srcImage ビットマップの入力画像
 * @param[in] angle 回転角度[rad]
 * @return 回転処理後のビットマップ画像
 */
Gdiplus::Bitmap* rotateBitmap(Gdiplus::Bitmap *srcImage, double angle) {
	Gdiplus::Bitmap *destImage;	//出力画像
	Gdiplus::BitmapData srcImageData;
	Gdiplus::BitmapData dstImageData;
	Gdiplus::Rect rect;

	// サイズを取得する
	const int SrcWidth = srcImage->GetWidth();
	const int SrcHeight = srcImage->GetHeight();

	// SIN,COS値を計算
	const double cosAngle = cos(angle);
	const double sinAngle = sin(angle);

	// 出力画像のサイズを計算
	const int DstWidth = (int) (fabs(SrcWidth * cosAngle)
			+ fabs(SrcHeight * sinAngle) + 0.5);
	const int DstHeight = (int) (fabs(SrcWidth * sinAngle)
			+ fabs(SrcHeight * cosAngle) + 0.5);

	// 出力画像用のメモリ領域を確保する
	destImage = new Gdiplus::Bitmap(DstWidth, DstHeight, PixelFormat32bppARGB);

	// 入力画像のピクセルデータを取得する
	rect.X = 0;
	rect.Y = 0;
	rect.Width = SrcWidth;
	rect.Height = SrcHeight;
	srcImage->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB,
			&srcImageData);
	int *const srcImagePixels = (int*) (srcImageData.Scan0);

	// 出力画像のピクセルデータを取得する
	rect.X = 0;
	rect.Y = 0;
	rect.Width = DstWidth;
	rect.Height = DstHeight;
	destImage->LockBits(&rect, Gdiplus::ImageLockModeWrite,
	PixelFormat32bppARGB, &dstImageData);
	int *const dstImagePixels = (int*) (dstImageData.Scan0);

	// 入出力画像の中心座標を計算する
	const int SrcCX = SrcWidth / 2;
	const int SrcCY = SrcHeight / 2;
	const int DstCX = DstWidth / 2;
	const int DstCY = DstHeight / 2;

	// SIN,COS値を整数値に変換
	const int intCos = (int) (cosAngle * 1024);
	const int intSin = (int) (sinAngle * 1024);

	// 画像回転処理
	// 出力画像の座標でループを行う
#	ifdef _OPENMP
#	pragma omp parallel for
#	endif
	for (int y2 = 0; y2 < DstHeight; y2++) {
		for (int x2 = 0; x2 < DstWidth; x2++) {
			// 出力画像座標(x2,y2)から入力画像の座標(x1,y1)を逆計算する
			const int x0 = (x2 - DstCX) * intCos - (y2 - DstCY) * intSin;
			const int y0 = (x2 - DstCX) * intSin + (y2 - DstCY) * intCos;
			const int x1 = (x0 >> 10) + SrcCX;
			const int y1 = (y0 >> 10) + SrcCY;

			// x1, y1がともに入力画像の有効範囲にあれば出力へコピーを行う
			if (x1 >= 0 && x1 < SrcWidth && y1 >= 0 && y1 < SrcHeight) {
				const int dx = x0 & 0x3ff;
				const int dy = y0 & 0x3ff;
				dstImagePixels[x2 + y2 * DstWidth] = bilinearInterpolation(
						srcImagePixels, SrcWidth, SrcHeight, x1, y1, dx, dy);
			}
		}
	}

	// 取得したピクセルデータを解放する
	srcImage->UnlockBits(&srcImageData);
	destImage->UnlockBits(&dstImageData);

	return destImage;
}

void setAlphaChannel(Gdiplus::Bitmap *srcImage, double alpha) {
	if (alpha < 0) {
		alpha = 0;
	}
	if (alpha > 1.0) {
		alpha = 1.0;
	}

	Gdiplus::BitmapData srcImageData;
	Gdiplus::Rect rect;

	// サイズを取得する
	const int width = srcImage->GetWidth();
	const int height = srcImage->GetHeight();

	// 入力画像のピクセルデータを取得する
	rect.X = 0;
	rect.Y = 0;
	rect.Width = width;
	rect.Height = height;
	srcImage->LockBits(&rect,
			Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite,
			PixelFormat32bppARGB, &srcImageData);
	int *const srcImagePixels = (int*) (srcImageData.Scan0);

#	ifdef _OPENMP
#	pragma omp parallel for
#	endif
	for (int y2 = 0; y2 < height; y2++) {
		for (int x2 = 0; x2 < width; x2++) {
			int *pixel = &srcImagePixels[x2 + y2 * width];
			const int alphaChannel = ((*pixel) >> 24) & 0xff;
			if (alphaChannel > 0) {
				*pixel = ((*pixel) & 0xffffff)
						| (int) (((unsigned int) (alphaChannel * alpha)) << 24);
			}
		}
	}

	// 取得したピクセルデータを解放する
	srcImage->UnlockBits(&srcImageData);
	return;
}

/**
 * ビットマップの伸縮処理@n
 * CPUが処理するため極めて低速である。
 * 高速化のため補間は行わない。
 * @param[in] srcImage ビットマップの入力画像
 * @param[in] ratio 倍率
 * @return 処理後のビットマップ画像
 */
Gdiplus::Bitmap* trimBitmap(Gdiplus::Bitmap *srcImage, int x, int y, int width,
		int height) {
	Gdiplus::Bitmap *destImage;	//出力画像
	Gdiplus::Rect rect;

	// 出力画像用のメモリ領域を確保する
	destImage = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);

	Gdiplus::Graphics *graphics = new Gdiplus::Graphics(destImage);
	graphics->DrawImage(srcImage, 0, 0, x, y, width, height,
			Gdiplus::UnitPixel);
	delete graphics;
	graphics = NULL;
	return destImage;
}

}
