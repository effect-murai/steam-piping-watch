/*
 * DefaultMode.cpp
 *
 *  Created on: 2016/01/08
 *      Author: PC-EFFECT-012
 */

#include "app.h"
#include "MainWindow.h"
#include "ResultData.h"
#include "Graphics.h"
#include "FileUtils.h"
#include "StringUtils.h"
#include "MainCanvas.h"
#include <math.h>
#include "resource.h"
#include <stdio.h>
#include <assert.h>
#include "Functions.h"
#include "opencv2/highgui/highgui.hpp"
// (2017/5/23YM)インクルードファイル宣言追加
#include "HotspotLinker.h"

// foreach構文
#define __foreach(item_type, value, list) \
	for (\
		std::vector<item_type>::iterator \
		value = list.begin(); \
		value != list.end(); \
		value++\
	)

#define isKeyDown(keycode) ((GetKeyState(keycode) & 0x8000) != 0)

//------------------------------------------------------------------------------
// Debugging Options
//------------------------------------------------------------------------------
//#define ZOOM_FUNCTION_TEST // ズーム表示練習用
//#define SHOW_GRIDLINE // キャンバス中央に線を表示
#define DRAW_PICTURE_FRAME // 選択画像の枠を表示

//------------------------------------------------------------------------------
// External Global Variables
//------------------------------------------------------------------------------

extern ResultData *result;
//　(2017/5/23YM)外部変数追加
extern HotspotLinker *hotspotLinker;

#ifdef SHOW_GRIDLINE
inline void showGridLine(Canvas* canvas, int weight, COLORREF color) 
{
	HPEN oldPen = (HPEN)canvas->selectGdiObject(CreatePen(PS_SOLID, weight, color));
	canvas->drawLine(0, canvas->clientHeight() / 2,
			canvas->clientWidth(), canvas->clientHeight() / 2);
	canvas->drawLine(
			canvas->clientWidth() / 2, 0,
			canvas->clientWidth() / 2, canvas->clientHeight());
	DeleteObject(canvas->selectGdiObject(oldPen));
}
#endif // SHOW_GRIDLINE

/**
 * ビットマップを読み込む
 */
inline Gdiplus::Bitmap* loadBitmap(const TCHAR *fileName, double ratio,
		double direction, int pictureType) {
	if ((ratio < 0) || (ratio > 10)) {
		ratio = 1;
	}

	wchar_t *wcFileName;
#ifndef UNICODE
	// パス長の最大まで確保する
	wcFileName = new wchar_t[MAX_PATH];
	MultiByteToWideChar(CP_OEMCP, MB_COMPOSITE, fileName, -1, wcFileName, MAX_PATH);
#else // UNICODE
	// UNICODE版の場合はそのまま使える
	wcFileName = (wchar_t*) fileName;
#endif // UNICODE

	Gdiplus::Bitmap *original = new Gdiplus::Bitmap(wcFileName);

	if (pictureType == ResultData::VISIBLE_LIGHT_IMAGE) {
		// (2019/11/15LEE)
		cv::Mat VBIMAGE = Graphics::loadCVImage(wcFileName);

		int a = VBIMAGE.size().width;		// (2019/11/18LEE) 追加。
		int b = VBIMAGE.size().height;

		// (2019/11/25LEE) resize
		if (a != 1600 || b != 1200) {
			cv::Mat dst;			// (2019/11/15LEE) 追加。
			cv::resize(VBIMAGE, dst, cv::Size(1600, 1200), 0, 0, CV_INTER_NN);// (2019/11/19LEE) 追加。
			VBIMAGE = dst;			// (2019/11/15LEE) 追加。

			delete original;
			original = Graphics::toBitmap(&VBIMAGE);
		}

		double ratioV, ratioH;
		result->getCameraRatio(&ratioV, &ratioH);		//(2019/11/19LEE) 変更。。
		ratio /= (ratioH + ratioV) / 2;
		int width = result->getInfraredWidth() * ratioH;
		int height = result->getInfraredHeight() * ratioV;
		double x, y;

		result->getCameraOffset(&x, &y);	//(2019/11/19LEE) 変更。。
		int left = (original->GetWidth() - width) / 2 + x;
		int top = (original->GetHeight() - height) / 2 - y;
		Gdiplus::Bitmap *trim = Graphics::trimBitmap(original, left, top, width,
				height);
		std::swap(original, trim);
		delete trim;
	}

#ifdef NO_OPENCV
	// OpenCVを利用しない
	Gdiplus::Bitmap* img = original;
#else // NO_OPENCV
	// OpenCVを利用する
	cv::Mat img = Graphics::toCVMatrix(original);
	delete original;
#endif // NO_OPENCV

#ifndef UNICODE
	delete wcFileName;
#endif // UNICODE

#ifdef NO_OPENCV
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
#else // NO_OPENCV
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

#ifndef NO_OPENCV_ROTATE
	Gdiplus::Bitmap *img3 = Graphics::toBitmap(&img2);
	Gdiplus::Bitmap *imgRet = img3;
	if (direction != 0) {
		imgRet = Graphics::rotateBitmap(img3, direction);
		delete img3;
	}
#else // NO_OPENCV_ROTATE
	float angle = -(direction * 180.0 / M_PI), scale = 1.0;
	// 中心：画像中心
	cv::Point2f center(img2.cols * 0.5, img2.rows * 0.5);
	// 以上の条件から2次元の回転行列を計算
	const cv::Mat affine_matrix = cv::getRotationMatrix2D(center, angle, scale);
	
	cv::Mat img3;
	cv::warpAffine(img2, img3, affine_matrix, img2.size());
	Gdiplus::Bitmap* imgRet = Graphics::toBitmap(&img3);
#endif // NO_OPENCV_ROTATE

#endif // NO_OPENCV
	return imgRet;
}

void* MainWindow::loadPicture(int id, int pictureType) {
	// 取得した高度から画像の縮尺を計算する
	const double ratio = this->getPictureRatio(id);

	// 撮影方位を取得する
	double direction = this->getDirection(id);

	// ファイル名
	TCHAR *fileName;
	result->getFilePath(id, pictureType, &fileName);

	// Bitmapイメージを読み込む
	Gdiplus::Bitmap *img = loadBitmap(fileName, ratio, -direction, pictureType);

	// 後片付け
	delete fileName;

	return img;
}

void* MainWindow::loadPicture(int id, int pictureType, double pixelPerMeter) {
	// 取得した高度から画像の縮尺を計算する
	const double ratio = this->getPictureRatio(id, pixelPerMeter);

	// 撮影方位を取得する
	double direction = this->getDirection(id);

	// ファイル名
	TCHAR *fileName;
	result->getFilePath(id, pictureType, &fileName);

	// Bitmapイメージを読み込む
	Gdiplus::Bitmap *img = loadBitmap(fileName, ratio, -direction, pictureType);

	// 後片付け
	delete fileName;

	return img;
}

void MainWindow::gpsPosToPixel(double gpsX, double gpsY, int *x, int *y) {
	gpsPosToPixel(gpsX, gpsY, x, y, viewX, viewY, viewPixelPerMeter);// GPSXはｍ単位
}

void MainWindow::gpsPosToPixel(Vector2D *real, POINT *projective) {
	int x, y;
	gpsPosToPixel(real->x, real->y, &x, &y);
	projective->x = x;
	projective->y = y;
}

void MainWindow::gpsPosToPixel(double gpsX, double gpsY, int *x, int *y,
		double offsetX, double offsetY, double pixelPerMeter) {
	// @todo 仮
	const int width = canvas_Main->clientWidth();	// (2017/3/30YM)俯瞰表示画面の横幅
	const int height = canvas_Main->clientHeight();	// (2017/3/30YM)俯瞰表示画面の縦幅

	result->gpsPosToPixel(gpsX - offsetX, gpsY - offsetY, x, y, pixelPerMeter);

	*x += width / 2;
	*y += height / 2;
}

void MainWindow::gpsPosToPixel(int id, int *x, int *y) {
	double x0, y0;
	result->getXY(id, &x0, &y0);		// Noiの画像のXY座標（ｍ）
	gpsPosToPixel(x0, y0, x, y);		// x0,y0:Noiの画像のXY座標（単位はｍ）
}

void MainWindow::gpsPosToPixel(int id, int *x, int *y, double offsetX,
		double offsetY, double pixelPerMeter) {
	double x0, y0;
	result->getXY(id, &x0, &y0);
	gpsPosToPixel(x0, y0, x, y, offsetX, offsetY, pixelPerMeter);
}

double MainWindow::getPictureRatio(int id) {
	return getPictureRatio(id, viewPixelPerMeter);

}

double MainWindow::getPictureRatio(int id, double pixelPerMeter) {
	// 撮影高度を取得する
	const double height = getHeight(id);

	// 熱画像サイズを取得する
	const int infraredWidth = result->getInfraredWidth();

	// 視野角を取得する
	const double viewAngle = result->getViewAngle();

	// 取得した高度から画像の縮尺を計算する
	// @todo 仮
	return result->getPictureRatio(infraredWidth, height, viewAngle,
			pixelPerMeter);

}

/**
 * 各画像のホットスポットを描画する
 * @param id 画像番号
 * @param radius ホットスポットのサイズ
 */
void MainWindow::drawHotspotOnEachPicture(int id, COLORREF color, int radius) {
	// 熱画像サイズを取得する
	const int infraredWidth = result->getInfraredWidth();
	const int infraredHeight = result->getInfraredHeight();

	// 取得した高度から画像の縮尺を計算する
	const double ratio = getPictureRatio(id);

	// 撮影方位を取得する
	const double direction = getDirection(id);

	int cx, cy;
	// 画像の表示位置を計算する
	gpsPosToPixel(id, &cx, &cy);

	// 描画ペンとブラシを設定する
	HPEN outsidePen = CreatePen(PS_SOLID, 4, RGB(255, 255, 255));
	HPEN insidePen = CreatePen(PS_SOLID, 2, color);
	HBRUSH oldBrush = (HBRUSH) canvas_Main->selectGdiObject(
			GetStockObject(NULL_BRUSH));

	// 外側のペンをデフォルトとして選択する
	HPEN oldPen = (HPEN) canvas_Main->selectGdiObject(outsidePen);

	// 点の表示位置を計算し、点を表示する
	for (int i = 0; i < result->getHotspotCount(id); i++) {
		if (result->isHotspot(id, i)) {
			POINT p;
			result->getHotspot(id, i, &p);
			const double fx = (p.x - infraredWidth / 2) * ratio;
			const double fy = (p.y - infraredHeight / 2) * ratio;
			int x = fx * cos(direction) - fy * sin(direction);
			int y = fx * sin(direction) + fy * cos(direction);
			x = x + cx;
			y = y + cy;
			canvas_Main->drawCircle(x, y, radius);
			canvas_Main->selectGdiObject(insidePen);
			canvas_Main->drawCircle(x, y, radius);
			canvas_Main->selectGdiObject(outsidePen);
		}
	}

	// ペンとブラシを元に戻す
	canvas_Main->selectGdiObject(oldBrush);
	canvas_Main->selectGdiObject(oldPen);

	DeleteObject(outsidePen);
	DeleteObject(insidePen);
}

void MainWindow::getPictureRect(int id, int pictureType, POINT *rect) {
	// 画像サイズ
	const int pictureWidth = (result->getPictureWidth(pictureType));
	const int pictureHeight = (result->getPictureHeight(pictureType));

	// 撮影方位を取得する
	const double direction = getDirection(id);

	// 表示倍率を計算する
	const double ratio = getPictureRatio(id);

	// 画像の表示位置を計算する
	int cx, cy;
	gpsPosToPixel(id, &cx, &cy);

	// 矩形の各頂点の値をセットする
	rect[0].x = 0;
	rect[0].y = 0;
	rect[1].x = 0;
	rect[1].y = pictureHeight;
	rect[2].x = pictureWidth;
	rect[2].y = pictureHeight;
	rect[3].x = pictureWidth;
	rect[3].y = 0;

	// 矩形を方位ぶん回転させる
	for (int i = 0; i < 4; i++) {
		POINT *const vertex = rect + i;
		const double fx = (vertex->x - pictureWidth / 2) * ratio;
		const double fy = (vertex->y - pictureHeight / 2) * ratio;
		const int x = fx * cos(direction) - fy * sin(direction);
		const int y = fx * sin(direction) + fy * cos(direction);
		vertex->x = x + cx;
		vertex->y = y + cy;
	}
}

void MainWindow::getPictureRect(int id, int pictureType, POINT *rect,
		double pixelPerMeter) {
	// 画像サイズ
	const int pictureWidth = result->getPictureWidth(pictureType);
	const int pictureHeight = result->getPictureHeight(pictureType);

	// 撮影方位を取得する
	const double direction = getDirection(id);

	// 表示倍率を計算する
	const double ratio = getPictureRatio(id, pixelPerMeter);

	// 画像の表示位置を計算する
	int cx, cy;
	gpsPosToPixel(id, &cx, &cy, 0, 0, pixelPerMeter);

	// 矩形の各頂点の値をセットする
	rect[0].x = 0;
	rect[0].y = 0;
	rect[1].x = 0;
	rect[1].y = pictureHeight;
	rect[2].x = pictureWidth;
	rect[2].y = pictureHeight;
	rect[3].x = pictureWidth;
	rect[3].y = 0;

	// 矩形を方位ぶん回転させる
	for (int i = 0; i < 4; i++) {
		POINT *const vertex = rect + i;
		const double fx = (vertex->x - pictureWidth / 2) * ratio;
		const double fy = (vertex->y - pictureHeight / 2) * ratio;
		const int x = fx * cos(direction) - fy * sin(direction);
		const int y = fx * sin(direction) + fy * cos(direction);
		vertex->x = x + cx;
		vertex->y = y + cy;
	}
}

//------------------------------------------------------------------------------
// MainWindow Class
//------------------------------------------------------------------------------

bool MainWindow::canvas_Main_DefaultOnClick(void) {
	if (result == NULL) {
		zoomIn(0, mainX, mainY);
		canvasUpdate();
	}

	// フォルダを指定する前は何もしない
	if (result == NULL) {
		return false;
	}

	// データがない場合も何もしない
	if (result->getDataCount() == 0) {
		return false;
	}

	int selected = NOT_SELECTED;

	// Shift + クリックで選択できるようにする
	if ((GetKeyState(VK_SHIFT) & 0x8000)) {
		//　クリックした画像を最前面に出す
		for (int i = result->getDataCount() - 1; i >= 0; i--) {
			int id = i;
			if (selectedPictureId != NOT_SELECTED) {
				if (i == result->getDataCount() - 1) {
					id = selectedPictureId;
				} else if (i >= selectedPictureId) {
					id = i + 1;
				}
			}

			// 矩形の各頂点の値をセットする
			POINT rect[4];
			getPictureRect(id, ResultData::INFRARED_IMAGE, rect);

			// ホットスポット
			POINT mousePos = { mainX, mainY };
			if (isPointInPolygon(mousePos, 4, rect)) {
				selected = id;
				break;
			}
		}
	}
	if (selected != selectedPictureId) {
		selectedPictureId = selected;
		canvasUpdate();
	}

	return true;
}

bool MainWindow::canvas_Main_DefaultOnDblClick(void) {
	// ホットスポット上をダブルクリックした場合はそちらを優先する

	int hotspotId = getHotspotId(mainX, mainY);

	if (hotspotId != -1) {
		if (isKeyDown(VK_CONTROL)) {
			// Controlを押しながらの場合は削除
			this->removeHotspotIdList(hotspotId);
		} else {
			this->makeHotpotIdList(hotspotId, getViewRatio());
		}
		// 保存する

		this->saveHotspotIdList();
		canvasUpdate();
		return true;
	}

	// 画像が選択されていない場合も何もしない
	if (getSelectedId() == NOT_SELECTED) {
		this->controlPanel->disableButton();
		return false;
	}

	/** @todo 暫定 */
	if (isKeyDown(VK_CONTROL) && isKeyDown(VK_SHIFT)) {
		// GPS位置情報を補正する
		result->adjustPosition(getSelectedId());
		// 補正したら以下の処理も必ず実行すること
		// ---- ここから ----
		// 全体のホットスポット情報を更新する
		this->getHotspotOverallPosition();
		// 俯瞰合成画像を作成しなおす
		for (int pictureType = 0; pictureType < 2; pictureType++) {
			const TCHAR *pattern;
			if (pictureType == ResultData::INFRARED_IMAGE) {
				pattern = TEXT("infrared_*.png");
			} else {
				pattern = TEXT("visible_*.png");
			}
			std::vector<TCHAR*> files;
			searchFiles(result->getDataPath(), pattern, files);
			__foreach (TCHAR*, file, files)
			{
				TCHAR *filePath = new TCHAR[MAX_PATH];
				wsprintf(filePath, TEXT("%s\\%s"), result->getDataPath(),
						*file);
				DeleteFile(filePath);
				delete filePath;
			}
			searchFiles_cleanup(files);
		}
		createWholePictures();
		// ---- ここまで ----

		canvasUpdate();

		hotspotSumMessage();

		return true;
	}
	// ここまで暫定

	resultDataWrite();
	canvasUpdate();

	return true;
}

bool MainWindow::canvas_Main_DefaultUpdate(void) {
#ifdef ZOOM_FUNCTION_TEST  //　練習用
	if (result == NULL) 
	{
		Gdiplus::Bitmap* base = new Gdiplus::Bitmap(L"C://Users//PC-EFFECT-006//Desktop//infrared.png")；　// (2019/09/27LEE) PC-EFFECT-020 => 006に変更。
	 	Gdiplus::Graphics* graphics = new Gdiplus::Graphics(canvas_Main->getBackBuffer());
		graphics->DrawImage(base, 0, 0);
		//図形を描画する処理
		HPEN outsidePen = CreatePen(PS_SOLID, 4, RGB(255, 255, 255));
		HPEN oldPen = (HPEN)canvas_Main->selectGdiObject(outsidePen);
		int x = 10;
		int y = 10;
		int radius = 10;
		canvas_Main->drawCircle(x, y, radius);
		delete outsidePen;
		
		delete graphics;
		delete base;
#ifdef SHOW_GRIDLINE
		//グリット線作成
		showGridLine(canvas_Main, 3, RGB(255, 0, 0));
#endif // SHOW_GRIDLINE
	}
#endif // 練習用

	// フォルダを指定する前は何もしない
	if (result == NULL) {
		return false;
	}

	// データがない場合も何もしない
	if (result->getDataCount() == 0) {
		return false;
	}

	// 画像種別
	int pictureType = controlPanel->getPictureType();

	// 画像表示
	this->canvas_Main->drawPictureAll(pictureType);

	const int radius = (getViewRatio() < 1 ? 2 : getViewRatio() * 2);

	// 熱画像のときのみホットスポットを表示する
	// (2016/12/20YM)可視光画像の時にもホットスポットを表示する
	this->canvas_Main->drawHotspotAll(radius);

#ifdef DRAW_PICTURE_FRAME // 選択画像の表示はMiniMapのみとする
	// 選択中の画像、画像枠、ホットスポットを表示する
	int id = selectedPictureId;
	if (id != NOT_SELECTED) {
		// 画像表示
		this->canvas_Main->drawPicture(id, pictureType);
		// 画像枠表示
		this->canvas_Main->drawPictureFrame(id, pictureType);
		// 熱画像のときのみホットスポットを表示する
		// (2016/12/20YM)可視光画像の時にもホットスポットを表示する
		this->canvas_Main->drawHotspot(id, radius);
	}
#endif

#ifdef SHOW_GRIDLINE
	//グリット線作成
	showGridLine(canvas_Main, 3, RGB(255, 0, 0));
#endif // SHOW_GRIDLINE

	// (2017/2/9YM)画像ID表示処理を追加
	int selected = getSelectedId();
	this->canvas_Main->drawId(selected, result->getDataCount());

	return true;
}

LRESULT MainWindow::canvas_Main_DefaultProc(HWND hWnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam) {
	switch (uMsg) {
	case WM_LBUTTONDOWN:
		prevMainPos.x = mainX;
		prevMainPos.y = mainY;
		SetCapture(hWnd);
		break;

	case WM_RBUTTONDOWN:
		prevMainPos.x = mainX;
		prevMainPos.y = mainY;
		break;

	case WM_LBUTTONUP:
		ReleaseCapture();
		break;

	case WM_MOUSEMOVE:
		if (isKeyDown(VK_LBUTTON) || isKeyDown(VK_RBUTTON)) {
			scroll();
		} else {
			showHotspotDetail();
		}
		break;
	}
	return 0;
}

bool MainWindow::saveWholePicture(const TCHAR *path, int pictureType) {
	// ファイル名を保存する領域を確保
	wchar_t *fileName;
#ifndef UNICODE
	// パス長の最大まで確保する
	fileName = new wchar_t[MAX_PATH];
	MultiByteToWideChar(CP_OEMCP, MB_COMPOSITE, path, -1, fileName, MAX_PATH);
#else
	// UNICODE版の場合はそのまま使える
	fileName = new wchar_t[MAX_PATH];
	wcscpy(fileName, path);
#endif // UNICODE

	// 俯瞰画像全体サイズを設定
	double left, right, top, bottom;
	result->getDataAreaSize2(&left, &bottom, &right, &top);
	int width = (right - left) * PIXELS_PER_METER;
	int height = (top - bottom) * PIXELS_PER_METER;

	// 管理番号とホットスポットの表示倍率
	double objectRatio = height / 1000.0;

	// 画像種別
	Gdiplus::Bitmap *destImage = new Gdiplus::Bitmap(width, height,
	PixelFormat32bppARGB);
	Gdiplus::Graphics *graphics = new Gdiplus::Graphics(destImage);

	for (int id = 0; id < result->getDataCount(); id++) {
		// (2017/2/8YM)2016/10/4田崎さん修正内容反映（ホットスポット一覧報告書画像の修正）
		// Bitmapイメージを読み込む
		Gdiplus::Bitmap *img = (Gdiplus::Bitmap*) loadPicture(id, pictureType,
				PIXELS_PER_METER);

		// 画像を読み込んだBitmapを描画
		int x, y;
		result->gpsPosToPixel(id, &x, &y, PIXELS_PER_METER);
		graphics->DrawImage(img, x - (int) img->GetWidth() / 2,
				y - (int) img->GetHeight() / 2);

		// Bitmapオブジェクトを破棄
		delete img;
	}

	// ホットスポット表示円の直径(ピクセル数)
	int diameter = fmax(10 * objectRatio, 8);

	//　ペン作成
	Gdiplus::Pen *red = new Gdiplus::Pen(Gdiplus::Color::Red, 2.5);
	Gdiplus::Pen *frm = new Gdiplus::Pen(Gdiplus::Color::White, 3);
	// 点の表示位置を計算し、点を表示する
	for (int i = 0; i < (int) result->getAllHotspotCount(); i++) {
		int x, y;
		Vector2D pos = result->getAllHotspotPos(i);
		result->gpsPosToPixel(pos.x, pos.y, &x, &y, PIXELS_PER_METER);
		// 出力するホットスポットリストのidと一致すればオレンジペン
		bool reportOut = false;
		// 出力対象のホットスポットかどうかを確認する
		std::vector<HotspotIdList> hotspotIdList =
				MainWindow::getInstance()->getHotspotIdList();
		for (std::vector<HotspotIdList>::iterator item = hotspotIdList.begin();
				item != hotspotIdList.end(); item++) {
			if (item->id == i) {
				reportOut = true;
				break;
			}
		}
		if (reportOut == true) {
			// ホットスポットの点を描画
			graphics->DrawEllipse(frm, x - (diameter / 2), y - (diameter / 2),
					diameter, diameter);
			graphics->DrawEllipse(red, x - (diameter / 2), y - (diameter / 2),
					diameter, diameter);
		} else {
			// 出力対象外のホットスポットも描画
			graphics->DrawEllipse(frm, x - (diameter / 2), y - (diameter / 2),
					diameter, diameter);
			graphics->DrawEllipse(red, x - (diameter / 2), y - (diameter / 2),
					diameter, diameter);
		}
	}
	delete red;
	delete frm;

	// Save the altered image.
	CLSID pngClsid;
	Graphics::GetEncoderClsid(L"image/png", &pngClsid);
	destImage->Save(fileName, &pngClsid, NULL);

	delete fileName;

	delete destImage;
	delete graphics;

	return true;
}

/**
 * メインキャンバスと重なっていない画像を除外する
 * @param id 確認対象の画像番号
 * @param pictureType 画像種別
 * @return true:除外対象/false:除外対象ではない
 */
bool MainWindow::excludePicture(int id, int pictureType) {
	bool ret = true;
	// キャンバスのクライアント領域を図形として取得する
	HRGN canvasRgn;
	POINT canvasVertex[4];
	RECT canvasRect;
	canvas_Main->getClientRect(&canvasRect);
	rectToPointArray(&canvasRect, canvasVertex);
	canvasRgn = CreatePolygonRgn(canvasVertex, 4, ALTERNATE);

	POINT p[4];
	getPictureRect(id, ResultData::INFRARED_IMAGE, p);

	HRGN pictureRgn = CreatePolygonRgn(p, 4, ALTERNATE);

	// canvasRgn と pictureRgn が重なっているかどうか確認する
	if (CombineRgn(pictureRgn, pictureRgn, canvasRgn, RGN_AND) != NULLREGION) {
		// 重なっている部分がある場合
		ret = false;
	}

	DeleteObject(pictureRgn);

	DeleteObject(canvasRgn);

	return ret;
}

/**
 * ある枠と重なっていない画像を除外する
 */
bool MainWindow::excludePictureForScope(RECT scopeRect, int id, int pictureType,
		double pixelPerMeter) {
	bool ret = true;
	HRGN scopeRgn = CreateRectRgn(scopeRect.left, scopeRect.top,
			scopeRect.right, scopeRect.bottom);

	POINT pic[4];
	getPictureRect(id, pictureType, pic, pixelPerMeter);

	HRGN pictureRgn = CreatePolygonRgn(pic, 4, ALTERNATE);

	// scopeRgn と pictureRgn が重なっているかどうか確認する
	if (CombineRgn(pictureRgn, pictureRgn, scopeRgn, RGN_AND) != NULLREGION) {
		// 重なっている部分がある場合
		ret = false;
	}

	DeleteObject(pictureRgn);

	DeleteObject(scopeRgn);

	return ret;
}

void MainWindow::saveWholePictureSplit(int pictureType, TCHAR *splitFileName,
		RECT *rect) {
	// 分割後の画像サイズを再計算する
	int bitmapWidth = rect->right - rect->left + 1;
	int bitmapHeight = rect->bottom - rect->top + 1;

	// 画像を作成する
	Gdiplus::Bitmap *splitImage = new Gdiplus::Bitmap(bitmapWidth, bitmapHeight,
	PixelFormat32bppARGB);
	Gdiplus::Graphics *graphics = new Gdiplus::Graphics(splitImage);

	const int count = result->getDataCount();
#	ifdef _OPENMP
#	pragma omp parallel for
#	endif
	for (int id = 0; id < count; id++) {
		// 除外対象かどうかを確認し、除外対象の場合は保存処理を飛ばす
		if (excludePictureForScope(*rect, id, pictureType, PIXELS_PER_METER)) {
			continue;
		}

		// (2017/2/8YM)2016/10/4田崎さん修正内容反映（ホットスポット一覧報告書画像の修正）
		// Bitmapイメージを読み込む
		Gdiplus::Bitmap *img = (Gdiplus::Bitmap*) loadPicture(id, pictureType,
				PIXELS_PER_METER);

		// 画像を読み込んだBitmapを描画
		int x, y;
		result->gpsPosToPixel(id, &x, &y, PIXELS_PER_METER);
		x = x - (int) img->GetWidth() / 2 - rect->left;
		y = y - (int) img->GetHeight() / 2 - rect->top;
		graphics->DrawImage(img, x, y);

		// Bitmapオブジェクトを破棄
		delete img;
	}
	// 保存処理
	CLSID pngClsid;
	Graphics::GetEncoderClsid(L"image/png", &pngClsid);
	if (pictureType == ResultData::INFRARED_IMAGE) {
		splitImage->Save(splitFileName, &pngClsid, NULL);
	} else {
		splitImage->Save(splitFileName, &pngClsid, NULL);
	}

	delete graphics;
	delete splitImage;
}

/**
 * 1枚の画像を分割して保存する
 */
void MainWindow::saveWholePictureSplit(int *progress) {
	// 撮影範囲の大きさを取得し、全体画像のサイズを計算する
	double left, right, top, bottom;
	result->getDataAreaSize2(&left, &bottom, &right, &top);
	// baseの4隅の座標を取得？
	int width = (right - left) * PIXELS_PER_METER;
	int height = (top - bottom) * PIXELS_PER_METER;

	// 取得した座標を元に分割？
	int lastCol = width / splitWidth;
	if ((width % splitWidth) != 0) {
		lastCol++;
	}
	int lastRow = height / splitHeight;
	if ((height % splitHeight) != 0) {
		lastRow++;
	}

	RECT rect;
	int splitCountV = lastRow; // 縦に2分割にしてみる
	int splitCountH = lastCol; // 横に2分割にしてみる

	const int count = splitCountV * splitCountH * 2;
	int completed = 0;

	for (int pictureType = 0; pictureType < 2; pictureType++) {
		// 最初から画像を分割して作成する
		for (int row = 1; row <= splitCountV; row++) {
			// 分割した画像の4隅の座標を取得
			if (row < lastRow) {
				rect.top = (row - 1) * splitHeight;
				rect.bottom = row * splitHeight - 1;
			} else {
				rect.top = (row - 1) * splitHeight;
				rect.bottom = height - 1;
			}

			for (int col = 1; col <= splitCountH; col++) {
				// 分割した画像の4隅の座標を取得
				if (col < lastCol) {
					rect.left = (col - 1) * splitWidth;
					rect.right = col * splitWidth - 1;
				} else {
					rect.left = (col - 1) * splitWidth;
					rect.right = width - 1;
				}
				const TCHAR *prefix;
				if (pictureType == ResultData::INFRARED_IMAGE) {
					prefix = TEXT("infrared");
				} else {
					prefix = TEXT("visible");
				}

				// 分割後のファイル名
				TCHAR *splitFileName;

				splitFileName = new TCHAR[MAX_PATH];
				wsprintf(splitFileName, TEXT("%s\\%s_%02dx%02d.png"),
						result->getDataPath(), prefix, col, row);

				if (!existFile(splitFileName)) {
					saveWholePictureSplit(pictureType, splitFileName, &rect);
				}
				delete splitFileName;

				completed++;
				*progress = 100 * completed / count;
			}
		}
	}
	*progress = 100;
}

/**
 * 指定したホットスポット周辺の画像を切り取って方位を補正して保存する。 (2020/01/17LEE)
 */
// (2020/01/21LEE) Detailのホットスポットの写真を保存する事とHomeで報告書に保存する写真のSIZEを探して赤の線表示する事をここで全部処理
double MainWindow::saveHotspotPictureAlgorithm(int id, double viewRatio,
		int setmode) {
	// setmode=1 : save Hotspot Picture, semode=2 : 報告書の出力Detailホットsポットの写真のDirectionをReturn

	int Rad157;		// 90deg毎の回転数
	double direction2;
	// Ratioチェック
	if ((viewRatio < 0) || (viewRatio > 10)) {
		viewRatio = 1;
	}

	//　(2017/5/23YM)出力画像用変数宣言
	int dataId;
	int spotId;
	POINT pt;

	// (2020/01/17LEE) 面積が一番大きい事を探すために追加
	int bestdataId;
	int bestspotId;
	POINT bestpt;

	// (2017/5/23YM)リンクがない場合の初期値を設定
	//　ホットスポットIDから画像NoとホットスポットNoを取得
	HotspotNumber hn;
	hn = result->getAllHotspotNo(id);
	dataId = hn.pictureNo;
	spotId = hn.pointNo;

	//　ホットスポット座標取得
	result->getHotspot(dataId, spotId, &pt);
	int pictureType;

	// (2017/5/23YM)ホットスポットのリンク情報から最も中心にホットスポットが近いものを取得
	//　ホットスポット固有ID取得
	int hsid = result->getHotspotId(dataId, spotId);

	//　リンク有無チェック
	if (hsid > -1) {
		POINT minpt;
		double minblackpixel = 1;
		double pixelpercent = 0;
		int checkpixel = 0;

		HotspotLink link = (*hotspotLinker)[hsid];
		//(2020/01/17LEE)一番いい写真のためこの過程を実行
		__foreach (HotspotNumber, hotspot, link)
		{
			result->getHotspot(hotspot->pictureNo, hotspot->pointNo, &minpt);
			checkpixel = 0;
			pt.x = minpt.x;
			pt.y = minpt.y;
			dataId = hotspot->pictureNo;
			spotId = hotspot->pointNo;

			double viewPixelRatio = viewRatio * 5;
			double Ratio = getPictureRatio(dataId, viewPixelRatio) * 2;

			pictureType = 1;
			// 撮影方位を取得する
			double direction = this->getDirection(dataId);		// (rad)

			// (2017/5/12YM)90度毎に分割
			Rad157 = 0;
			direction2 = direction;
			// (2017/8/23YM)初期値が－だった場合の処理を追加
			if (direction2 < 0) {
				while (direction2 < -1.57079633) {
					Rad157++;
					direction2 += 1.57079633;
				}
				Rad157 = Rad157 % 4;		//　０～３の値に変換
				Rad157 = fabs(Rad157 - 3);
			} else {
				while (direction2 > 1.57079633) {
					Rad157++;
					direction2 -= 1.57079633;
				}
				Rad157 = Rad157 % 4;		//　０～３の値に変換
			}

			// ファイル名
			TCHAR *fileName;
			result->getFilePath(dataId, pictureType, &fileName);
			// Bitmapイメージを読み込む
			int width;
			int height;
			width = 640 * fabs(cos(-direction)) + 512 * fabs(sin(-direction));
			height = 640 * fabs(sin(-direction)) + 512 * fabs(cos(-direction));
			Gdiplus::Bitmap *outImg = new Gdiplus::Bitmap(width, height,
			PixelFormat32bppARGB);
			Gdiplus::Graphics *graphics = new Gdiplus::Graphics(outImg);
			Gdiplus::Bitmap *img = loadBitmap(fileName, Ratio, -direction,
					pictureType);

			// ホットスポットの座標を画像別に計算
			double x1 = pt.x;
			double y1 = pt.y;
			double x = x1 * cos(direction) - y1 * sin(direction);
			double y = x1 * sin(direction) + y1 * cos(direction);

			//　座標系別処理
			switch (Rad157) {
			case 0:
				x = x + 512 * fabs(sin(-direction));
				break;

			case 1:
				y = y + 512 * fabs(cos(-direction));
				break;

			case 2:
				x = x - 512 * fabs(sin(-direction));
				break;

			case 3:
				y = y - 512 * fabs(cos(-direction));
				break;
			}

			//　座標補正
			if (x < 0) {
				x = x + width;
			}
			if (y < 0) {
				y = y + height;
			}

			//　画像を中心に描き込み
			int xx = (int) (width / 2) - (x * Ratio);
			int yy = (int) (height / 2) - (y * Ratio);
			graphics->DrawImage(img, xx, yy);

			// 保存する
			//saveFileにファイルパスとファイル名が入る。
			TCHAR *saveFile;

			if (setmode == 1) {
				int number = this->getHotspotIdListListNo(id);
				saveFile = result->getfileNameIr(number);
			} else {
				saveFile = result->selcetIR();
			}

			CLSID pngClsid;
			Graphics::GetEncoderClsid(L"image/jpeg", &pngClsid);
			outImg->Save(saveFile, &pngClsid, NULL);

			if (pictureType == ResultData::INFRARED_IMAGE) {
				cv::Mat check_image = Graphics::loadCVImage(saveFile);
				int height = check_image.rows;
				int width = check_image.cols;
				//データに問題がある場合
				if (height < 0 || width < 0) {
					height = 640;
					width = 512;
				}
				for (int y = 0; y < height; y++) {
					uchar *pointer_input = check_image.ptr<uchar>(y);
					uchar *pointer_ouput = check_image.ptr<uchar>(y);
					for (int x = 0; x < width; x++) {
						uchar b = pointer_input[x * 3 + 0];
						uchar g = pointer_input[x * 3 + 1];
						uchar r = pointer_input[x * 3 + 2];

						pointer_ouput[x] = (r + g + b) / 3.0;
						if (0 == (int) pointer_ouput[x]) {
							checkpixel++;
						}
					}
				}
				pixelpercent = (double) checkpixel / (double) (height * width);
			}
			if (pixelpercent < minblackpixel) {
				minblackpixel = pixelpercent;
				bestdataId = dataId;
				bestspotId = spotId;
				bestpt.x = pt.x;
				bestpt.y = pt.y;
			}

			delete outImg;
			delete graphics;
			delete img;
		}

		spotId = bestspotId;
		dataId = bestdataId;
		pt.x = bestpt.x;
		pt.y = bestpt.y;
	}

	if (setmode == 2) {
		return dataId;
	}

	double viewPixelRatio = viewRatio * 5;
	double Ratio = getPictureRatio(dataId, viewPixelRatio) * 2;

	// ２種類の画像を出力
	for (int pictureType = 0; pictureType < 2; pictureType++) {
		// 撮影方位を取得する
		double direction = this->getDirection(dataId);		// (rad)
		// (2017/5/12YM)90度毎に分割
		Rad157 = 0;
		direction2 = direction;
		// (2017/8/23YM)初期値が－だった場合の処理を追加
		if (direction2 < 0) {
			while (direction2 < -1.57079633) {
				Rad157++;
				direction2 += 1.57079633;
			}
			Rad157 = Rad157 % 4;		//　０～３の値に変換
			Rad157 = fabs(Rad157 - 3);
		} else {
			while (direction2 > 1.57079633) {
				Rad157++;
				direction2 -= 1.57079633;
			}
			Rad157 = Rad157 % 4;		//　０～３の値に変換
		}

		// ファイル名
		TCHAR *fileName;
		result->getFilePath(dataId, pictureType, &fileName);

		// Bitmapイメージを読み込む
		int width;
		int height;
		width = 640 * fabs(cos(-direction)) + 512 * fabs(sin(-direction));
		height = 640 * fabs(sin(-direction)) + 512 * fabs(cos(-direction));
		Gdiplus::Bitmap *outImg = new Gdiplus::Bitmap(width, height,
		PixelFormat32bppARGB);
		Gdiplus::Graphics *graphics = new Gdiplus::Graphics(outImg);

		// ホットスポットの座標を画像別に計算
		double x1 = pt.x;
		double y1 = pt.y;
		double x = x1 * cos(direction) - y1 * sin(direction);
		double y = x1 * sin(direction) + y1 * cos(direction);

		//　座標系別処理
		switch (Rad157) {
		case 0:
			x = x + 512 * fabs(sin(-direction));
			break;

		case 1:
			y = y + 512 * fabs(cos(-direction));
			break;

		case 2:
			x = x - 512 * fabs(sin(-direction));
			break;

		case 3:
			y = y - 512 * fabs(cos(-direction));
			break;
		}

		//　座標補正
		if (x < 0) {
			x = x + width;
		}
		if (y < 0) {
			y = y + height;
		}

		int checksize = 0;
		checksize = Picturesizecheck(width, height, Ratio, x, y);

		if (checksize == 0) {
			Ratio = 1;
		}
		int xx = (int) (width / 2) - (x * Ratio);
		int yy = (int) (height / 2) - (y * Ratio);

		Gdiplus::Bitmap *img = loadBitmap(fileName, Ratio, -direction,
				pictureType);
		graphics->DrawImage(img, xx, yy);

		// 保存する
		//saveFileにファイルパスとファイル名が入る。
		int number = this->getHotspotIdListListNo(id);

		TCHAR *saveFile;
		if (pictureType == ResultData::INFRARED_IMAGE) {
			saveFile = result->getfileNameIr(number);
		} else {
			saveFile = result->getfileNameVb(number);
		}

		CLSID pngClsid;
		Graphics::GetEncoderClsid(L"image/jpeg", &pngClsid);
		outImg->Save(saveFile, &pngClsid, NULL);

		delete outImg;
		delete graphics;
		delete img;
	}

	return 1;

}

/**
 * ある点を中心に構成後の画像を切り出して保存する
 * @param id ホットスポット固有id
 * @param viewRatio　表示倍率
 */
void MainWindow::trimConstitutionPicture(int id, double viewRatio) {
	// @todo 暫定比率で表示倍率から表示上のppmを求める
	const double viewPixelPerMeter = viewRatio * 10;

	Vector2D hotspot;
	int iMax, jMax;
	int picRight, picBottom;
	{
		//ホットスポットの場所を指定。
		hotspot = result->getAllHotspotPos(id);
		double left, right, top, bottom;
		result->getDataAreaSize2(&left, &bottom, &right, &top);
		// 左上を基点(0,0)にした座標に変換
		hotspot.x = hotspot.x - left;
		hotspot.y = (top - bottom) - (hotspot.y - bottom);
		picRight = (int) ((right - left) * PIXELS_PER_METER);
		picBottom = (int) ((top - bottom) * PIXELS_PER_METER);
		iMax = (picRight / splitWidth) + 1;
		jMax = (picBottom / splitHeight) + 1;
	}

	const int size = canvas_Main->clientWidth() / viewPixelPerMeter
			* PIXELS_PER_METER;
	const int width = size;
	const int height = size * 240 / 280;
	const int outputSize = 560;
	double ratio = (double) outputSize / (double) size;
	const int outputWidth = outputSize;
	const int outputHeight = outputSize * 240 / 280;

	for (int pictureType = 0; pictureType < 2; pictureType++) {
		Gdiplus::Bitmap *outImg = new Gdiplus::Bitmap(outputWidth, outputHeight,
		PixelFormat32bppARGB);
		Gdiplus::Graphics *output = new Gdiplus::Graphics(outImg);

		const int left = floor(hotspot.x * PIXELS_PER_METER - width / 2);
		const int right = floor(hotspot.x * PIXELS_PER_METER + width / 2) - 1;
		const int xSplitMin = (left / splitWidth) + 1; // 一番左の画像の番号
		const int xSplitMax = (right / splitWidth) + 1; // 一番右の画像の番号
		const int top = floor(hotspot.y * PIXELS_PER_METER - height / 2);
		const int bottom = floor(hotspot.y * PIXELS_PER_METER + height / 2) - 1;
		const int ySplitMin = (top / splitHeight) + 1; // 一番上の画像の番号
		const int ySplitMax = (bottom / splitHeight) + 1; // 一番下の画像の番号

		int y = 0;
		for (int j = ySplitMin; j <= ySplitMax; j++) {
			int picY;
			if (j == ySplitMin) // 一番左側の画像
					{
				picY = top % splitHeight;
			} else {
				// それ以外
				picY = 0;
			}
			int picHeight;
			if (j == ySplitMax) {
				// 一番右側の画像
				int absPicY = picY + ((j - 1) * splitHeight); // 全体画像の左端からの位置
				picHeight = picBottom - absPicY;
			} else {
				// それ以外
				picHeight = splitHeight - picY;
			}
			int x = 0;
			for (int i = xSplitMin; i <= xSplitMax; i++) {
				// 元画像(分割後)の左側の座標を計算する
				int picX;
				if (i == xSplitMin) {
					// 一番左側の画像
					picX = left % splitWidth;
				} else {
					// それ以外
					picX = 0;
				}
				int picWidth;
				if (i == xSplitMax) {
					// 一番右側の画像
					int absPicX = picX + ((i - 1) * splitWidth); // 全体画像の左端からの位置
					picWidth = picRight - absPicX;
				} else {
					picWidth = splitWidth - picX;
				}

				if ((i >= 1) && (i <= iMax) && (j >= 1) && (j <= jMax)) {
					//allHotspotNo[id].pictureNoが含まれる分割画像の特定
					TCHAR *fileName = new TCHAR[1024];

					// 画像種別によってファイル名を変える
					if (pictureType == ResultData::INFRARED_IMAGE) {
						wsprintf(fileName, TEXT("%s\\infrared_%02dx%02d.png"),
								result->getDataPath(), i, j);
					} else {
						wsprintf(fileName, TEXT("%s\\visible_%02dx%02d.png"),
								result->getDataPath(), i, j);
					}

					Gdiplus::Bitmap *base = new Gdiplus::Bitmap(fileName);
					delete fileName;

					Gdiplus::Bitmap *trim = Graphics::trimBitmap(base, picX,
							picY, picWidth, picHeight);
					delete base;
					Gdiplus::Bitmap *stretch = Graphics::stretchBitmap(trim,
							ratio);
					delete trim;
					output->DrawImage(stretch, x, y);

					// 俯瞰画像の一部を保存する
#ifdef DEBUG
					TCHAR* bitName = new TCHAR[1024];
					
					// 画像種別によってファイル名を変える
					if(pictureType == ResultData::INFRARED_IMAGE)
					{
						wsprintf(bitName, TEXT("%s\\B%02d_%02dx%02dir.png"),
							result->getDataPath(), id, i, j);
					}
					else
					{
						wsprintf(bitName, TEXT("%s\\B%02d_%02dx%02dvb.png"),
							result->getDataPath(), id, i, j);
					}
					CLSID pngClsid;
					Graphics::GetEncoderClsid(L"image/png", &pngClsid);
					stretch->Save(bitName, &pngClsid, NULL);
					delete bitName;
					
					wprintf(TEXT("%d, %d:x = %d(abs:%d) , w = %d == %d * %lg == %d?\n"), i, j, x, picX + ((i - 1) * splitWidth), stretch->GetWidth(), picWidth, ratio, (int)(picWidth * ratio));
					fflush(stdout);
#endif // DEBUG

					delete stretch;
				}

				x += picWidth * ratio;
			}

			y += picHeight * ratio;
		}

		delete output;

		// 保存する
		//saveFileにファイルパスとファイル名が入る。
		int number = this->getHotspotIdListListNo(id);
		TCHAR *saveFile;
		if (pictureType == ResultData::INFRARED_IMAGE) {
			saveFile = result->getfileNameIr(number);
		} else {
			saveFile = result->getfileNameVb(number);
		}
		CLSID pngClsid;
		Graphics::GetEncoderClsid(L"image/png", &pngClsid);
		outImg->Save(saveFile, &pngClsid, NULL);

		delete outImg;
	}
}

/**
 * 報告書に出力するホットスポットのリストを作成
 * @param id ホットスポットID
 * @param viewRatio 表示倍率
 */
bool MainWindow::makeHotpotIdList(int id, double viewRatio) {
	__foreach (HotspotIdList, eachItem, hotspotIdList)
	{
		// リスト中の全てのidをチェックして
		// 既にあるホットスポットidは表示倍率だけ変更する

		HotspotNumber checknum = result->getAllHotspotNo(id);
		if (eachItem->picnum == checknum.pictureNo
				&& eachItem->hotnum == checknum.pointNo) {
			eachItem->id = id;
			eachItem->viewRatio = viewRatio;
			eachItem->bestpicnum =
					MainWindow::getInstance()->saveHotspotPictureAlgorithm(id,
							viewRatio, 2);
			return true;
		}
	}

	HotspotIdList newItem;
	newItem.id = id;
	newItem.viewRatio = viewRatio;

	// (2020/01/14LEE)
	HotspotNumber checknum = result->getAllHotspotNo(id);
	newItem.picnum = checknum.pictureNo;
	newItem.hotnum = checknum.pointNo;

	// (2020/01/20LEE) setmode=2 :　面積が一番大きい写真のDirectionの値をreturn
	newItem.bestpicnum = MainWindow::getInstance()->saveHotspotPictureAlgorithm(
			id, viewRatio, 2);

	hotspotIdList.push_back(newItem);
	return true;
}

/**
 * ホットスポットリストの内容を1つ削除する（チェックを外す）
 */
void MainWindow::removeHotspotIdList(int id) {
	__foreach (HotspotIdList, eachItem, hotspotIdList)
	{
		// リスト中の全てのidをチェックして
		// 一致するものがあれば削除する
		if (eachItem->id == id) {
			hotspotIdList.erase(eachItem);
			break;
		}
	}
}

// (2020/01/10LEE)
void MainWindow::removeHotspotALLIdList() {
	__foreach (HotspotIdList, eachItem, hotspotIdList)
	{
		// リスト中の全てのidをチェックして
		// 一致するものがあれば削除する

		hotspotIdList.erase(eachItem);
	}

	saveHotspotIdList();
}

void MainWindow::clearHotspotIdList(void) {
	// 先にリストをクリアする
	hotspotIdList.clear();
	// メモリを解放する
	std::vector<HotspotIdList>().swap(hotspotIdList);
}

// (2017/10/26YM)ホットスポットIDの変更処理を追加
void MainWindow::refreshHotspotIdList(int id) {
	__foreach (HotspotIdList, eachItem, hotspotIdList)
	{
		// リスト中の全てのidをチェックして
		// 大きいものがあればIDNoを１減らす
		if (eachItem->id > id) {
			eachItem->id--;
		}

	}
}
/**
 * 出力するホットスポットのリストをファイルに保存する、(2020/01/14LEE) picnumとhotnumを追加
 */
bool MainWindow::saveHotspotIdListFile(TCHAR *filePath) {
	// データをファイルに保存する
	FILE *file = fileOpen(filePath, TEXT("w+"));

	if (file == NULL) {
		return false;
	}
	__foreach (HotspotIdList, eachItem, hotspotIdList)
	{
		fprintf(file,
				"hotspotId : %d, viewRatio : %lf, picnum : %d, hotnum : %d, bestpicnum : %d\n",
				eachItem->id, eachItem->viewRatio, eachItem->picnum,
				eachItem->hotnum, eachItem->bestpicnum);
	}
	fclose(file);

	return true;
}

bool MainWindow::saveHotspotIdList(void) {
	TCHAR *filePath = result->getFileHotspotIdList();
	return saveHotspotIdListFile(filePath);
}

// (2020/01/14LEE) Detail ホットスポットが変わるバグに対応するために追加
void MainWindow::resaveHotspotIdList(int idnum, int picnum, int hotnum) {
	__foreach (HotspotIdList, eachItem, hotspotIdList)
	{
		if (eachItem->picnum == picnum && eachItem->hotnum == hotnum) {
			eachItem->id = idnum;
		}
	}
}

/**
 * ホットスポットリストのファイルを読み込む
 */
bool MainWindow::readHotspotIdListFile(TCHAR *filePath) {
	// 先にリストをクリアする
	this->clearHotspotIdList();

	FILE *file = fileOpen(filePath, TEXT("r"));
	if (file == NULL) {
		return false;
	}

	while (!feof(file)) {
		HotspotIdList newItem;
		int readCount =
				fscanf(file,
						"hotspotId : %d, viewRatio : %lf, picnum : %d, hotnum : %d, bestpicnum : %d\n",
						&newItem.id, &newItem.viewRatio, &newItem.picnum,
						&newItem.hotnum, &newItem.bestpicnum);
		// データ数が2こあるか確認
		if (readCount == 5) {
			hotspotIdList.push_back(newItem);
		} else {
			fclose(file);
			return false;
		}
	}

	fclose(file);
	return true;
}

bool MainWindow::readHotspotIdList(void) {
	TCHAR *filePath = result->getFileHotspotIdList();
	return readHotspotIdListFile(filePath);
}

int MainWindow::getHotspotIdListSize(void) {
	int size = (int) hotspotIdList.size();
	return size;
}

int MainWindow::getHotspotIdListId(int listNo) {
	int id = hotspotIdList[listNo].id;
	return id;
}

double MainWindow::getHotspotIdListViewRatio(int listNo) {
	double viewRatio = hotspotIdList[listNo].viewRatio;
	return viewRatio;
}

int MainWindow::getHotspotIdListListNo(int id) {
	int listNo = -1;
	for (int i = 0; i < (int) hotspotIdList.size(); i++) {
		if (hotspotIdList[i].id == id) {
			listNo = i;
		}
	}
	return listNo;
}

// (2020/02/12LEE) 報告書を出力する時、Imageが小さい場合拡大
int MainWindow::Picturesizecheck(int width, int height, double Ratio, double x,
		double y) {
	int cavasX;
	int cavasY;
	int checksize = 0;
	// (2020/02/12LEE) testing

	cavasX = (int) (width / 2) - (x * Ratio);
	cavasY = (int) (height / 2) - (y * Ratio);

	if (cavasX <= 0 || cavasY <= 0) {
		checksize++;
	}
	cavasX = (int) (width / 2) + (x * Ratio);
	cavasY = (int) (height / 2) - (y * Ratio);

	if (cavasX >= width || cavasY <= 0) {
		checksize++;
	}
	cavasX = (int) (width / 2) - (x * Ratio);
	cavasY = (int) (height / 2) + (y * Ratio);

	if (cavasX <= 0 || cavasY >= height) {
		checksize++;
	}
	cavasX = (int) (width / 2) + (x * Ratio);
	cavasY = (int) (height / 2) + (y * Ratio);

	if (cavasX >= width || cavasY >= height) {
		checksize++;
	}
	return checksize;

}
