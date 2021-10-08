/*
 * PictureWindow.cpp
 *
 *  Created on: 2015/11/24
 *      Author: PC-EFFECT-012
 */

#include <windows.h>
#include "Controls.h"
#include "ResultData.h"
#include "resource.h"
#include "Graphics.h"
#include "MainWindow.h"
#include "HotspotLinker.h"
#include "SubWindows/PictureWindow.h"
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include "MainWindow.h"
#include "app.h"

//------------------------------------------------------------------------------
// Compiler Options
//------------------------------------------------------------------------------
#ifdef KEYPOINT_MATCHING_DEBUG
#define	_PREWITT_OPERATOR		0
#define	_PREWITT_OPERATOR_XY	1
#define EDGE_DITECTION_OPERATOR 2

#define ENABLE_GAUSSIAN_BLUR
#endif

//------------------------------------------------------------------------------
// Global variables
//------------------------------------------------------------------------------

extern ResultData *result;
extern HotspotLinker *hotspotLinker;
extern MainWindow *mainWindow;
HWND Popuphandle; // (2019/12/04LEE) PopupWindowのHandleを保存するために追加。
int selectnumber; // (2019/12/03LEE) PictureWindowのClassのPictureIdを読むために追加。
#ifdef HOTSPOT_DETECTION_DEBUG
Gdiplus::Bitmap* detectHotspotBitmap(int id, int threshold);
extern volatile int _threshold;
#endif

//------------------------------------------------------------------------------
// Global functions
//------------------------------------------------------------------------------

inline MainWindow* getParent(HWND hWnd) {
	return (MainWindow*) Window::fromHandle(GetParent(hWnd));
}

inline LRESULT callWindowClassProc(HWND hWnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam) {
	return CallWindowProc((WNDPROC) GetClassLongPtr(hWnd, GCLP_WNDPROC), hWnd,
			uMsg, wParam, lParam);
}

inline int getHotspotCircleRadius(void) {
	return 10;
}

//------------------------------------------------------------------------------
// PictureWindow class
//------------------------------------------------------------------------------
PictureWindow::PictureWindow(HWND handle, int pictureType, int id) :
		Dialog(handle) {
	this->pictureType = pictureType;
	this->pictureBox = NULL;
	this->pictureBox_clickX = 0;
	this->pictureBox_clickY = 0;
	this->PictureId = id;

	update();

	pictureBox->setHandler(handlePictureBoxEvent);

	UpdateWindow(getHandle());
	this->show();
}

PictureWindow::~PictureWindow(void) {
	DestroyWindow(pictureBox->getHandle());
	delete pictureBox;
	pictureBox = NULL;
}

void PictureWindow::update(void) {

	// (2019/12/03LEE) ClassのObjectで使用するために変更

	int showId = PictureId;

	// Bitmapイメージを読み込む
#ifndef HOTSPOT_DETECTION_DEBUG
	TCHAR *fileName = result->getFilePath(showId, pictureType);
	// (2019/11/15LEE) loadBitmapnに変数pictureTpyeを追加。
	Gdiplus::Bitmap *img = Graphics::loadBitmap(fileName, pictureType, 1, 0);
#else
	Gdiplus::Bitmap* img = detectHotspotBitmap(showId, _threshold);
#endif

	angle = MainWindow::getInstance()->getDirection(showId);

	// 画像を回転させる
	Gdiplus::Bitmap *img2 = Graphics::rotateBitmap(img, -angle);

	const int canvasWidth = (int) img2->GetWidth();
	const int canvasHeight = (int) img2->GetHeight();

	// ウィンドウサイズを取得
	RECT windowRect, clientRect;
	getWindowRect(&windowRect);
	getClientRect(&clientRect);

	// ウィンドウの幅と高さを計算
	const int windowWidth = windowRect.right - windowRect.left;
	const int windowHeight = windowRect.bottom - windowRect.top;

	// ウィンドウ内部の幅と高さを計算
	const int clientWidth = clientRect.right - clientRect.left;
	const int clientHeight = clientRect.bottom - clientRect.top;

	// ウィンドウ枠の幅と高さを計算
	const int borderWidth = windowWidth - clientWidth;
	const int borderHeight = windowHeight - clientHeight;

	// ウィンドウサイズを変更する
	int width = borderWidth + canvasWidth;
	int height = borderHeight + canvasHeight;

	RECT desktopRect;
	GetWindowRect(GetDesktopWindow(), &desktopRect);

	int left = (desktopRect.right - desktopRect.left - width) / 2;
	int top = (desktopRect.bottom - desktopRect.top - height) / 2;
	if (top < 0) {
		top = 0;
	}

	if (windowWidth != width || windowHeight != height) {
		move(left, top, width, height);
	}

	if (pictureBox == NULL) {
		pictureBox = new Canvas(0, 0, canvasWidth, canvasHeight, this, false);
	} else {
		pictureBox->resize(canvasWidth, canvasHeight);
		pictureBox->resizeBackBuffer();
	}
	pictureBox->clear((HBRUSH) GetStockObject(BLACK_BRUSH));

	// GDI+のGraphicsオブジェクトを作成
	Gdiplus::Graphics *graphics = new Gdiplus::Graphics(
			pictureBox->getBackBuffer());

	// 画像を読み込んだBitmapを描画
	graphics->DrawImage(img2, 0, 0);

	// ホットスポットは熱画像のときのみ表示する
	if (pictureType == ResultData::INFRARED_IMAGE) {
		int centerX = img->GetWidth() / 2;
		int centerY = img->GetHeight() / 2;
		HPEN whitePen = CreatePen(PS_SOLID, 4, RGB(255, 255, 255));
		HPEN redPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
		HPEN bluePen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
		HBRUSH oldBrush = (HBRUSH) pictureBox->selectGdiObject(
				GetStockObject(NULL_BRUSH));

		const int radius = getHotspotCircleRadius();

		// 点の表示位置を計算し、点を表示する
		for (int i = 0; i < result->getHotspotCount(showId); i++) {
			POINT p;
			result->getHotspot(showId, i, &p);
			p.x -= centerX;
			p.y -= centerY;
			int x, y;
			x = p.x * cos(angle) - p.y * sin(angle);
			y = p.x * sin(angle) + p.y * cos(angle);
			x = x + canvasWidth / 2;
			y = y + canvasHeight / 2;
			HPEN oldPen = (HPEN) pictureBox->selectGdiObject(whitePen);
			pictureBox->drawCircle(x, y, radius);
			if (result->getHotspotType(showId, i) == HOTSPOT_TYPE_KEYPOINT) {
				pictureBox->selectGdiObject(bluePen);
			} else {
				pictureBox->selectGdiObject(redPen);
			}
			pictureBox->drawCircle(x, y, radius);
			pictureBox->selectGdiObject(oldPen);
		}

		DeleteObject(whitePen);
		DeleteObject(redPen);
		DeleteObject(bluePen);
		pictureBox->selectGdiObject(oldBrush);
	}

	// Bitmapオブジェクトを破棄
	delete img;
	delete img2;
	delete graphics;

	// タイトルバーを更新する
	TCHAR *text = new TCHAR[MAX_PATH + 1];
	stprintf(text, TEXT("%s"), result->getFilePath(showId, pictureType));
	setText(text);
	delete text;

	// キャンバスを更新する
	pictureBox->update();

	// メインウィンドウに対してキャンバス更新を要求する
	getParent(getHandle())->canvasUpdate();
}

// (2019/12/25LEE) Pictureboxで移動を対応
void PictureWindow::update2(void) {

	// (2019/12/03LEE) ClassのObjectで使用するために変更

	int showId = mainWindow->getSelectedId();
	PictureId = showId;
	// Bitmapイメージを読み込む
#ifndef HOTSPOT_DETECTION_DEBUG
	TCHAR *fileName = result->getFilePath(showId, pictureType);
	// (2019/11/15LEE) loadBitmapnに変数pictureTpyeを追加。
	Gdiplus::Bitmap *img = Graphics::loadBitmap(fileName, pictureType, 1, 0);
#else
	Gdiplus::Bitmap* img = detectHotspotBitmap(showId, _threshold);
#endif

	angle = MainWindow::getInstance()->getDirection(showId);

	// 画像を回転させる
	Gdiplus::Bitmap *img2 = Graphics::rotateBitmap(img, -angle);

	const int canvasWidth = (int) img2->GetWidth();
	const int canvasHeight = (int) img2->GetHeight();

	// ウィンドウサイズを取得
	RECT windowRect, clientRect;
	getWindowRect(&windowRect);
	getClientRect(&clientRect);

	// ウィンドウの幅と高さを計算
	const int windowWidth = windowRect.right - windowRect.left;
	const int windowHeight = windowRect.bottom - windowRect.top;

	// ウィンドウ内部の幅と高さを計算
	const int clientWidth = clientRect.right - clientRect.left;
	const int clientHeight = clientRect.bottom - clientRect.top;

	// ウィンドウ枠の幅と高さを計算
	const int borderWidth = windowWidth - clientWidth;
	const int borderHeight = windowHeight - clientHeight;

	// ウィンドウサイズを変更する
	int width = borderWidth + canvasWidth;
	int height = borderHeight + canvasHeight;

	RECT desktopRect;
	GetWindowRect(GetDesktopWindow(), &desktopRect);

	int left = (desktopRect.right - desktopRect.left - width) / 2;
	int top = (desktopRect.bottom - desktopRect.top - height) / 2;
	if (top < 0) {
		top = 0;
	}

	if (windowWidth != width || windowHeight != height) {
		move(left, top, width, height);
	}

	if (pictureBox == NULL) {
		pictureBox = new Canvas(0, 0, canvasWidth, canvasHeight, this, false);
	} else {
		pictureBox->resize(canvasWidth, canvasHeight);
		pictureBox->resizeBackBuffer();
	}

	pictureBox->clear((HBRUSH) GetStockObject(BLACK_BRUSH));

	// GDI+のGraphicsオブジェクトを作成
	Gdiplus::Graphics *graphics = new Gdiplus::Graphics(
			pictureBox->getBackBuffer());

	// 画像を読み込んだBitmapを描画
	graphics->DrawImage(img2, 0, 0);

	// ホットスポットは熱画像のときのみ表示する
	if (pictureType == ResultData::INFRARED_IMAGE) {
		int centerX = img->GetWidth() / 2;
		int centerY = img->GetHeight() / 2;
		HPEN whitePen = CreatePen(PS_SOLID, 4, RGB(255, 255, 255));
		HPEN redPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
		HPEN bluePen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
		HBRUSH oldBrush = (HBRUSH) pictureBox->selectGdiObject(
				GetStockObject(NULL_BRUSH));

		const int radius = getHotspotCircleRadius();

		// 点の表示位置を計算し、点を表示する
		for (int i = 0; i < result->getHotspotCount(showId); i++) {
			POINT p;
			result->getHotspot(showId, i, &p);
			p.x -= centerX;
			p.y -= centerY;
			int x, y;
			x = p.x * cos(angle) - p.y * sin(angle);
			y = p.x * sin(angle) + p.y * cos(angle);
			x = x + canvasWidth / 2;
			y = y + canvasHeight / 2;
			HPEN oldPen = (HPEN) pictureBox->selectGdiObject(whitePen);
			pictureBox->drawCircle(x, y, radius);
			if (result->getHotspotType(showId, i) == HOTSPOT_TYPE_KEYPOINT) {
				pictureBox->selectGdiObject(bluePen);
			} else {
				pictureBox->selectGdiObject(redPen);
			}
			pictureBox->drawCircle(x, y, radius);
			pictureBox->selectGdiObject(oldPen);
		}

		DeleteObject(whitePen);
		DeleteObject(redPen);
		DeleteObject(bluePen);
		pictureBox->selectGdiObject(oldBrush);
	}

	// Bitmapオブジェクトを破棄
	delete img;
	delete img2;

	delete graphics;

	// タイトルバーを更新する
	TCHAR *text = new TCHAR[MAX_PATH + 1];
	stprintf(text, TEXT("%s"), result->getFilePath(showId, pictureType));
	setText(text);
	delete text;

	// キャンバスを更新する
	pictureBox->update();

	// メインウィンドウに対してキャンバス更新を要求する
	getParent(getHandle())->canvasUpdate();
}

void PictureWindow::hotspot(int x, int y, int type) {
	if (pictureType != ResultData::INFRARED_IMAGE) {
		// 熱画像以外の場合は何もしない
		return;
	}

	int showId = PictureId;
	RECT clientRect;
	this->getClientRect(&clientRect);

	const int infraredWidth = result->getInfraredWidth();
	const int infraredHeight = result->getInfraredHeight();

	// ウィンドウ内部の幅と高さを計算
	const int canvasWidth = clientRect.right - clientRect.left;
	const int canvasHeight = clientRect.bottom - clientRect.top;
	int deleted = 0;
	const int radius = getHotspotCircleRadius();
	for (int i = 0; i < result->getHotspotCount(showId); i++) {
		POINT p;
		result->getHotspot(showId, i, &p);
		p.x -= infraredWidth / 2;
		p.y -= infraredHeight / 2;
		int lx, ly;
		lx = p.x * cos(angle) - p.y * sin(angle);
		ly = p.x * sin(angle) + p.y * cos(angle);
		lx = lx + canvasWidth / 2;
		ly = ly + canvasHeight / 2;
		int dx = x - lx;
		int dy = y - ly;
		if (sqrt(dx * dx + dy * dy) < radius) {
			hotspotLinker->removeHotspot(showId, i);
			deleted++;
			i--;
		}
	}
	if (deleted == 0) {
		int px, py;
		px = pictureBox_clickX;
		py = pictureBox_clickY;
		px -= canvasWidth / 2;
		py -= canvasHeight / 2;
		int lx, ly;
		lx = px * cos(-angle) - py * sin(-angle);
		ly = px * sin(-angle) + py * cos(-angle);
		lx = lx + infraredWidth / 2;
		ly = ly + infraredHeight / 2;
		if ((lx >= 0 && lx < infraredWidth)
				&& (ly >= 0 && ly < infraredHeight)) {
			result->addHotspot(showId, lx, ly, type);
		}
	} else {
		// 削除した場合はホットスポットリンク情報も保存する
		hotspotLinker->saveTo(result->getHotspotLinkFileName());
	}
	this->update();
	result->saveHotspot();
	MainWindow::getInstance()->updateHotspot();
}

//------------------------------------------------------------------------------
// Global functions
//------------------------------------------------------------------------------

// キャンバスのイベントを処理するためのコールバック関数
LRESULT CALLBACK PictureWindow::handlePictureBoxEvent(HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam) {
	PictureWindow *pictureWindow = (PictureWindow*) Window::fromHandle(
			GetParent(hWnd));
	switch (uMsg) {
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEMOVE:
		pictureWindow->pictureBox_clickX = LOWORD(lParam);
		pictureWindow->pictureBox_clickY = HIWORD(lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		SendMessage(pictureWindow->getHandle(), uMsg, wParam, lParam);
		break;
	}
	return callWindowClassProc(hWnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK PictureWindow::handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam) {
	PictureWindow *pictureWindow = (PictureWindow*) fromHandle(hWnd);
	if (pictureWindow == NULL) {
		if (uMsg == WM_INITDIALOG) {
			int pictureType = (
					(lParam & 0x80000000) ?
							ResultData::INFRARED_IMAGE :
							ResultData::VISIBLE_LIGHT_IMAGE);

			pictureWindow = new PictureWindow(hWnd, pictureType, selectnumber);
			PopWindowHandle(hWnd);
		} else {
			return (INT_PTR) FALSE;
		}
	}

	switch (uMsg) {
	case WM_INITDIALOG:
		return (INT_PTR) TRUE;

	case WM_ACTIVATE:		// (2019/12/04LEE) 更新するために追加
		pictureWindow->update();
		break;

	case WM_CLOSE:
		pictureWindow->close(1);
		DestroyWindow(pictureWindow->getHandle());
		return (INT_PTR) TRUE;

	case WM_DESTROY:
		PopWindowHandle(NULL);
		delete pictureWindow;
		break;

	case WM_DRAWITEM:
		if (pictureWindow->pictureBox != NULL) {
			if (*(pictureWindow->pictureBox)
					== ((LPDRAWITEMSTRUCT) lParam)->hwndItem) {
				pictureWindow->pictureBox->transfer(
						((LPDRAWITEMSTRUCT) lParam)->hDC);
			}
		}
		return (INT_PTR) TRUE;

	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case STN_CLICKED:
			break;

		case STN_DBLCLK:
			pictureWindow->pictureBoxOnDblClick();
			break;
		}
		break;

	case WM_KEYDOWN:
		switch (wParam) {
		case VK_LEFT:
			getParent(hWnd)->selectPrev();
			pictureWindow->update2();
			break;

		case VK_RIGHT:
			getParent(hWnd)->selectNext();
			pictureWindow->update2();
			break;

		case VK_ESCAPE:
			pictureWindow->close(1);
			return (INT_PTR) TRUE;

		default:
			break;
		}
		break;

	default:
		break;
	}
	return (INT_PTR) FALSE;
}

void PictureWindow::pictureBoxOnDblClick(void) {
	int type;

	if (GetKeyState(VK_MENU) & 0x8000) {
		type = HOTSPOT_TYPE_KEYPOINT;
	} else {
		type = HOTSPOT_TYPE_ENABLED;
	}
	this->hotspot(this->pictureBox_clickX, this->pictureBox_clickY, type);
}

void PictureWindow::create(Window *parent, int pictureType, int id) {
	selectnumber = id;
	LPARAM param = (
			pictureType == ResultData::VISIBLE_LIGHT_IMAGE ? 0 : 0x80000000);
	Dialog::create(parent, handleEvent, param);
}
// (2019/12/04LEE) MainCanvasで変化がある場合対応。
void PictureWindow::Windowupdate(HWND parent, int id) {
	if (Popuphandle != NULL) {
		PictureWindow *pictureWindow = (PictureWindow*) fromHandle(Popuphandle);
		pictureWindow->update();
	}
}

void PictureWindow::Windowupdate2(HWND parent) {

	if (Popuphandle != NULL) {
		PictureWindow *pictureWindow = (PictureWindow*) fromHandle(Popuphandle);
		pictureWindow->update2();
	}
}

// (2019/12/04LEE) PopupWindowのHandleを　保存
void PictureWindow::PopWindowHandle(HWND GetPopupHandle) {
	Popuphandle = GetPopupHandle;
}

#ifdef KEYPOINT_MATCHING_DEBUG
int getId(int offset) 
{
	int id = MainWindow::getInstance()->getSelectedId() + offset;
	while (id < 0) 
	{
		id += result->getDataCount();
	}
	
	while (id >= result->getDataCount()) 
	{
		id -= result->getDataCount();
	}
	
	return id;
}

cv::Mat cvGetTempData(int idOffset) 
{
	int id = getId(idOffset);
	//return Graphics::loadCVImage(result->getInfraredFilePath(id));
	
	int width, height;
	float* _tempData = result->getTempData(id, &width, &height, NULL);
	
	if (_tempData == NULL) 
	{
		cv::Mat tempData;
		return tempData;
	}
	
	float minTemp = result->getTempMin();
	float maxTemp = result->getTempMax();
	
#ifdef COLOR_TEMP_MAP
	int convTable[1596], count = 0;
	for (int i = 128 - 5; i < 255; i++, count++) 
	{
		convTable[count] = RGB(0, 0, i);
	}
	for (int i = 1; i < 255; i++, count++) 
	{
		convTable[count] = RGB(0, i, 255);
	}
	for (int i = 1; i < 255; i++, count++) 
	{
		convTable[count] = RGB(0, 255, 255 - i);
	}
	for (int i = 1; i < 255; i++, count++) 
	{
		convTable[count] = RGB(i, 255, 0);
	}
	for (int i = 1; i < 255; i++, count++) 
	{
		convTable[count] = RGB(255, 255 - i, 0);
	}
	for (int i = 1; i < 255; i++, count++) 
	{
		convTable[count] = RGB(255, i, i);
	}
	
	cv::Mat tempData(cv::Size(width, height), CV_8UC3);
	cv::cvtColor(tempData, tempData, CV_BGR2HSV);
	for (int y = 0; y < height; y++) 
	{
		for (int x = 0; x < width; x++) 
		{
			unsigned int pos = y * tempData.step + x * tempData.elemSize();
			int data = convTable[(int)((_tempData[x + y * width] - minTemp) / (maxTemp - minTemp) * 255)];
			tempData.data[pos]     = data;
			tempData.data[pos + 1] = data;
			tempData.data[pos + 2] = data;
		}
	}
	cv::cvtColor(tempData, tempData, CV_HSV2BGR);
#else
	
	// 温度データをOpenCV Matrixに変換
	cv::Mat tempData(cv::Size(width, height), CV_8UC1);
	for (int y = 0; y < height; y++) 
	{
		for (int x = 0; x < width; x++) 
		{
			unsigned int pos = y * tempData.step + x * tempData.elemSize();
			tempData.data[pos] = (unsigned char)((_tempData[x + y * width] - minTemp) / (maxTemp - minTemp) * 255);
		}
	}
#endif

	delete _tempData;
	return tempData;
}

void keypointMatching(double angle) 
{
	int threshold = 25;
	cv::Mat tempData1 = cvGetTempData(0);
	cv::Mat tempData2 = cvGetTempData(1);
	
	std::vector<cv::KeyPoint> keypoints1;
	std::vector<cv::KeyPoint> keypoints2;
	std::vector<cv::DMatch> goodMatches;
	
	// ORB 検出器に基づく特徴点検出
	// n_features=300, params=default
	// default:
	//     最大検出数=500、ピラミッドレイヤー間の縮小比率=1.2f、ピラミッドレベル数=8、
	//     エッジしきい値=31、最初のレベル=0、WTA_K=2、スコアタイプ=0、パッチサイズ=31
	cv::OrbFeatureDetector detector1(300, 1.2, 7, 31, 0, 5, 0, 31);
	cv::OrbFeatureDetector detector2(300, 1.2, 7, 31, 0, 5, 0, 31);
	detector1.detect(tempData1, keypoints1);
	detector2.detect(tempData2, keypoints2);
	
	// ホットスポットを特徴点に追加
	int id0 = getId(0), id1 = getId(1);
	for (int i = 0; i < result->getHotspotCount(id0); i++) 
	{
		POINT p;
		result->getHotspot(id0, i, &p);
		keypoints1.push_back(cv::KeyPoint(cv::Point2f(p.x, p.y), 1));
	}
	for (int i = 0; i < result->getHotspotCount(id1); i++) 
	{
		POINT p;
		result->getHotspot(id1, i, &p);
		keypoints2.push_back(cv::KeyPoint(cv::Point2f(p.x, p.y), 1));
	}
	
	// 記述子を抽出
	cv::OrbDescriptorExtractor extractor;
	cv::Mat descriptors1, descriptors2;
	extractor.compute(tempData1, keypoints1, descriptors1);
	extractor.compute(tempData2, keypoints2, descriptors2);
	
	// マッチング
	std::vector<cv::KeyPoint> goodKeypoints1, goodKeypoints2;
	if (!descriptors1.empty() && !descriptors2.empty()) 
	{
		std::vector<cv::DMatch> matches;
		cv::BFMatcher matcher(cv::NORM_HAMMING, true);
		matcher.match(descriptors1, descriptors2, matches);
		for (unsigned int i = 0; i < matches.size(); i++) 
		{
			if (matches[i].distance < threshold) 
			{
				cv::Point2f p1 = keypoints1[matches[i].queryIdx].pt;
				cv::Point2f p2 = keypoints2[matches[i].trainIdx].pt;
				cv::Point2f _pt, pt;
				_pt.x = p2.x - p1.x;
				_pt.y = p2.y - p1.y;
				pt.x = _pt.x * cos(angle) - _pt.y * sin(angle);
				pt.y = _pt.x * sin(angle) + _pt.y * cos(angle);
				double direction = atan2(-pt.x, -pt.y);
				if (direction < 0) 
				{
					direction += M_PI * 2;
				}
				
				double dirError = fabs(direction - result->getGPSCardinalDirection(getId(1)));
				
				if (dirError > M_PI / 4) 
				{
					continue;
				}
				
				goodMatches.push_back(matches[i]);
				goodKeypoints1.push_back(keypoints1[matches[i].queryIdx]);
				goodKeypoints2.push_back(keypoints2[matches[i].trainIdx]);
			}
		}
	}
	
	cv::Mat dst_img;
	cv::drawMatches(tempData1, keypoints1, tempData2, keypoints2, goodMatches, dst_img);
	
	if (goodMatches.size() >= 1) 
	{
		cv::Mat img1, img2;
		img1 = Graphics::loadCVImage(result->getInfraredFilePath(getId(0)));
		img2 = Graphics::loadCVImage(result->getInfraredFilePath(getId(1)));
		if (img1.channels() == 1) 
		{
			cv::cvtColor(img1, img1, CV_GRAY2BGR);
		}
		if (img2.channels() == 1) 
		{
			cv::cvtColor(img2, img2, CV_GRAY2BGR);
		}
		
		cv::Point2f pt;
		for (unsigned int i = 0; i < goodMatches.size(); i++) 
		{
			cv::Point2f p1 = goodKeypoints1[i].pt;
			cv::Point2f p2 = goodKeypoints2[i].pt;
			pt.x += p1.x - p2.x;
			pt.y += p1.y - p2.y;
		}
		
		pt.x /= goodMatches.size();
		pt.y /= goodMatches.size();
		int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
		int w = img1.cols, h = img1.rows;
		if (pt.x > 0) 
		{
			x1 = 0;
			x2 = pt.x;
			w = img2.cols + x2;
		} 
		else 
		{
			x1 = -pt.x;
			x2 = 0;
			w = img1.cols + x1;
		}
		if (pt.y > 0) 
		{
			y1 = 0;
			y2 = pt.y;
			h = img2.rows + y2;
		} 
		else 
		{
			y1 = -pt.y;
			y2 = 0;
			h = img1.rows + y1;
		}
		cv::Mat dst_img2(cv::Size(w, h), CV_8UC3);
		img1.copyTo(cv::Mat(dst_img2, cv::Rect(x1, y1, img1.cols, img1.rows)));
		img2.copyTo(cv::Mat(dst_img2, cv::Rect(x2, y2, img2.cols, img2.rows)));
	}
}
#endif // KEYPOINT_MATCHING_DEBUG

#ifdef HOTSPOT_DETECTION_DEBUG

template <typename _Tp>

	class Matrix 
	{
		public:
			Matrix(int width, int height) :
			array(array),
			width(width), height(height) 
			{array = new _Tp[width * height];}
	
	virtual ~Matrix(void) 
	{
		delete array;
	}
	inline int getWidth(void) 
	{
		return (int)width;
	}
	inline int getHeight(void) 
	{
		return (int)height;
	}
	inline int arraySize(void) 
	{
		return (int)(width * height);
	}
	inline void clear(void) 
	{
		ZeroMemory(array, width * height * sizeof(_Tp));
	}
	inline void set(unsigned int x, unsigned int y, _Tp val) 
	{
		array[x + y * width] = val;
	}
	inline _Tp get(unsigned int x, unsigned int y) 
	{
		return array[x + y * width];
	}
private:
	_Tp* array;
	unsigned int width;
	unsigned int height;
};

template <typename _Tp> inline _Tp matGet(cv::Mat* mat, int x, int y) 
{
	return mat->at<_Tp>(y, x);
}

/**
 * ホットスポット検出@n
 * 画像処理を行いエッジ画像を出力する。エッジ部分を黒で表現した2値ビットマップ画像@n
 * CPUが処理するため極めて低速である。
 * @param[in] id 飛行データ番号
 * @param[out] hotspots ホットスポットデータ配列
 * @return ホットスポット個数
 */
Gdiplus::Bitmap* detectHotspotBitmap(int id, int threshold) 
{
	int width, height;
	
	// ---------------------- 温度データの取得 --------------------------
	int* tempData = result->getTempData(id, &width, &height);
	
	if (tempData == NULL) 
	{
		return 0;
	}
	// 出力用ビットマップを作成する
	Gdiplus::Bitmap* destImage;
	wchar_t* fileName;
#ifndef UNICODE
	// パス長の最大まで確保する
	fileName = new wchar_t[MAX_PATH];
	MultiByteToWideChar(CP_OEMCP, MB_COMPOSITE, getFilePath(id, ResultData::INFRARED_IMAGE), -1, fileName, MAX_PATH);
#else
	// UNICODE版の場合はそのまま使える
	fileName = result->getFilePath(id, ResultData::INFRARED_IMAGE);
#endif // UNICODE
	destImage = new Gdiplus::Bitmap(fileName);
#ifndef UNICODE
	delete filename;
#endif
	if (destImage == NULL) 
	{
		// ファイルが開けなかった場合、とりあえず新しいビットマップを作成する
		destImage = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
	}
	
	// 出力画像のピクセルデータを取得する
	Gdiplus::Rect rect;
	rect.X = 0;
	rect.Y = 0;
	rect.Width = width;
	rect.Height = height;
	
	Gdiplus::BitmapData dstImageData;
	
	destImage->LockBits(&rect, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &dstImageData);
	int* const dstImagePixels = (int*)(dstImageData.Scan0);
	
	// 検出したエッジを保持する領域を確保する
	char* const edgeData = new char[width * height];
	ZeroMemory(edgeData, width * height * sizeof(char));
	
#define f(x, y) (tempData[(x) + (y) * width])
	
	// Gaussian filter
#ifdef ENABLE_GAUSSIAN_BLUR
	int gaussianWeight[25] = {
			1,  4,  6,  4, 1,
			4, 16, 24, 16, 4,
			6, 24, 36, 24, 6,
			4, 16, 24, 16, 4,
			1,  4,  6,  4, 1
	};
	for (int y = 0; y < height; y++) 
	{
		for (int x = 0; x < width; x++) 
		{
			int data = 0, id = 0, color = 0;
			for (int j = -2; j <= 2; j++) 
			{
				for (int i = -2; i <= 2; i++, id++) 
				{
					if (x + i < 0 || x + i >= width || y + j < 0 || y + j >= height) 
					{
						color = f(x, y);
					} 
					else 
					{
						color = f(x + i, y + j);
					}
					data += gaussianWeight[id] * color;
				}
			}
			tempData[x + y * width] = (data >> 8);
		}
	}
#endif
	
	for (int y = 1; y < height - 1; y++) 
	{
		for (int x = 1; x < width - 1; x++) 
		{
#if EDGE_DITECTION_OPERATOR == _PREWITT_OPERATOR

#elif EDGE_DITECTION_OPERATOR == _PREWITT_OPERATOR_XY
			// Prewitt operator
			const int prewitt_x =
					-1 * f(x - 1, y - 1) +  1 * f(x, y - 1) + 1 * f(x + 1, y - 1) +
					-1 * f(x - 1, y    ) + -2 * f(x, y    ) + 1 * f(x + 1, y    ) +
					-1 * f(x - 1, y + 1) +  1 * f(x, y + 1) + 1 * f(x + 1, y + 1);
			const int prewitt_y =
					-1 * f(x - 1, y - 1) + -1 * f(x, y - 1) + -1 * f(x + 1, y - 1) +
					 1 * f(x - 1, y    ) + -2 * f(x, y    ) +  1 * f(x + 1, y    ) +
					 1 * f(x - 1, y + 1) +  1 * f(x, y + 1) +  1 * f(x + 1, y + 1);
			int edge = fabs(prewitt_x + prewitt_y) / 2;
#else
			// Sobel operator
			const int sobel_x =
					-1 * f(x - 1, y - 1) + 0 * f(x, y - 1) + 1 * f(x + 1, y - 1) +
					-2 * f(x - 1, y    ) + 0 * f(x, y    ) + 2 * f(x + 1, y    ) +
					-1 * f(x - 1, y + 1) + 0 * f(x, y + 1) + 1 * f(x + 1, y + 1);
			const int sobel_y =
					-1 * f(x - 1, y - 1) + -2 * f(x, y - 1) + -1 * f(x + 1, y - 1) +
					 0 * f(x - 1, y    ) +  0 * f(x, y    ) +  0 * f(x + 1, y    ) +
					 1 * f(x - 1, y + 1) +  2 * f(x, y + 1) +  1 * f(x + 1, y + 1);
			int edge = fabs(sobel_x + sobel_y) / 2;
#endif

			edgeData[x + y * width] = edge;
			if (edge > 255) 
			{
				edge = 255;
			} 
			else if (edge < 0) 
			{
				edge = 0;
			}
		}
	}
	
	// 平均を計算
	double average = 0;
	const int dataCount = (height - 2) * (width - 2);
	for (int y = 1; y < height - 1; y++) 
	{
		for (int x = 1; x < width - 1; x++) 
		{
			average += edgeData[x + y * width];
		}
	}
	average /= dataCount;
	
	// 分散を計算
	double variance = 0;
	for (int y = 1; y < height - 1; y++) 
	{
		for (int x = 1; x < width - 1; x++) 
		{
			const double diff = edgeData[x + y * width] - average;
			variance += diff * diff;
		}
	}
	variance /= dataCount;
	
	// 標準偏差を計算
	double stdDeviation = sqrt(variance);
	
	// 平均と標準偏差から閾値を決定
	const double threshold_high = average + stdDeviation * 1.5;
	const double threshold_low  = average + stdDeviation * 0.75;
	
	int* edgeData2 = new int[width * height];
	ZeroMemory(edgeData2, width * height);
	// 閾値(低)を上回った箇所をエッジ候補として検出する
	for (int y = 1; y < height - 1; y++) 
	{
		for (int x = 1; x < width - 1; x++) 
		{
			if (threshold_low < edgeData[x + y * width]) 
			{
				edgeData2[x + y * width] = 1;
			} 
			else 
			{
				edgeData2[x + y * width] = 0;
			}
		}
	}
	
	// 閾値(高)を上回った箇所をエッジとして検出する
	for (int y = 1; y < height - 1; y++) 
	{
		for (int x = 1; x < width - 1; x++) 
		{
			if (threshold_high < edgeData[x + y * width]) 
			{
				edgeData[x + y * width] = 1;
			} 
			else if (edgeData2[x + y * width] == 1) 
			{
				int count = 0;
				for (int dx = -1; dx <= 1; dx++) 
				{
					if (dx == 0)
					{
						continue;
					}
					if (edgeData2[(x + dx) + y * width] == 1) 
					{
						count++;
					}
				}
				for (int dy = -1; dy <= 1; dy++) 
				{
					if (dy == 0) 
					{
						continue;
					}
					if (edgeData2[x + (y + dy) * width] == 1) 
					{
						count++;
					}
				}
				// ひとつ以上隣接している場合はエッジとする
				if (count > 0) 
				{
					edgeData[x + y * width] = 1;
				}
			} 
			else 
			{
				edgeData[x + y * width] = 0;
			}
		}
	}
	
	delete edgeData2;
	
	// ---------------------- Hough変換 --------------------------
	
	// 直線検出用頻度カウンタ
	const int THETA_MAX = 2048;
	const int RHO_MAX = 1024;
	int* counter = new int[THETA_MAX * 2 * RHO_MAX];
	const double PIK = M_PI / THETA_MAX;
	ZeroMemory(counter, THETA_MAX * 2 * RHO_MAX * sizeof(int));
	
	// 三角関数テーブル（サイン）
	float* sn = new float[THETA_MAX];
	// 三角関数テーブル（コサイン）
	float* cs = new float[THETA_MAX];
	
	// 三角関数テーブルを作成
	for (int i = 0; i < THETA_MAX; i++) 
	{
    	sn[i] = sinf(PIK * i);
    	cs[i] = cosf(PIK * i);
	}
	
#define counterIndex(theta, rho) ((theta) * 2 * RHO_MAX + (rho))
	for (int y = 0; y < height - 1; y++) 
	{
		for (int x = 0; x < width; x++) 
		{
			if (edgeData[x + y * width] == 1) 
			{
				for (int theta = 0; theta < THETA_MAX; theta++) 
				{
					const int rho = (int)(x * cs[theta] + y * sn[theta] + 0.5);
					counter[counterIndex(theta, rho + RHO_MAX)]++;
				}
			}
		}
	}
	
	// --------------------- Hough逆変換 -------------------------
	
	const int MIN_DETECT_PIXEL = 60;	// 最小検出ピクセル数
	const int LINE_MAX = 56;			// 最大検出数
	
	int count = 0;			// 検出された直線または円の個数カウンタ
	
	int* line_rho = new int[LINE_MAX];
	int* line_theta = new int[LINE_MAX];
	int* line_type = new int[LINE_MAX];
	
	ZeroMemory(line_type, LINE_MAX * sizeof(int));
	ZeroMemory(line_rho, LINE_MAX * sizeof(int));
	ZeroMemory(line_theta, LINE_MAX * sizeof(int));
	
	std::vector<int> lineX;
	std::vector<int> lineY;
	
	int* _counter = new int[THETA_MAX * 2 * RHO_MAX];
	do 
	{
		int theta_max = 0;
		int rho_max = -RHO_MAX;
		int base_theta = 0;
		bool end_flag = false;	// 繰り返しを終了させるフラグ
		int detectErrorMax = THETA_MAX / 128; // 検出する角度誤差の最大
		lineX.clear();
		lineY.clear();
		count = 0;
		memcpy(_counter, counter, THETA_MAX * 2 * RHO_MAX * sizeof(int));
		do 
		{
			count++;
			volatile int counter_max = 0;
			// counterが最大になるtheta_maxとrho_maxを求める
			for (int theta = 0; theta < THETA_MAX; theta++) 
			{
				for (int rho = -RHO_MAX; rho < RHO_MAX; rho++) 
				{
					const int index = counterIndex(theta, rho + RHO_MAX);
					if (_counter[index] > counter_max) 
					{
						counter_max = _counter[index];
						// 50ピクセル以下の直線になれば検出を終了
						if (counter_max <= MIN_DETECT_PIXEL) 
						{
							end_flag = 1;
						} 
						else 
						{
							end_flag = 0;
						}
						theta_max = theta;
						rho_max = rho;
					}
				}
			}
			if (count == 1) 
			{
				base_theta = theta_max;
				if (fabs(base_theta - THETA_MAX / 2) >= THETA_MAX / 4) 
				{
					base_theta = (base_theta + THETA_MAX / 2) % THETA_MAX;
				}
			}
			
			// 直線の方向を算出
			const int d_theta = fabs(theta_max - base_theta);
			if (d_theta < detectErrorMax) 
			{
				// 横方向
				line_type[count - 1] = 1;
				lineX.push_back(count - 1);
			} 
			else if (fabs(d_theta - THETA_MAX / 2) < detectErrorMax) 
			{
				// 縦方向
				line_type[count - 1] = 2;
				lineY.push_back(count - 1);
			} 
			else 
			{
				// ずれている場合は無視
				line_type[count - 1] = 0;
			}
			
			// 直線のデータを保存
			if (line_type[count - 1] != 0) 
			{
				line_rho[count - 1] = rho_max;
				line_theta[count - 1] = theta_max;
			} 
			else 
			{
				count--;
			}
			
			// 近傍の直線を消す
			for (int j = -15; j <= 15; j++) 
			{
				for (int i = -45; i <= 45; i++)
				{
					if (theta_max + i < 0)
					{
						theta_max += THETA_MAX;
						rho_max = -rho_max;
					}
					if (theta_max + i >= THETA_MAX) 
					{
						theta_max -= THETA_MAX;
						rho_max = -rho_max;
					}
					if (rho_max + j < -RHO_MAX || rho_max + j >= RHO_MAX) 
					{
						continue;
					}
					const int index = counterIndex(theta_max + i, rho_max + RHO_MAX + j);
					_counter[index] = 0;
					if (count == 1) 
					{
						counter[index] = 0;
					}
				}
			}
			// 長さが60ピクセル以下なら終了
		} while (end_flag == 0 && count < LINE_MAX - 4);
	} while ((lineX.size() <= 5 || lineY.size() <= 5) && (count < 10) && (count != 0));
	delete _counter;
	
	if (lineX.size() > 0 && lineY.size() > 0) 
	{
		// 画像の端にグリッド線を入れる
		// 横方向
		line_rho[count] = 0;
		line_theta[count] = THETA_MAX / 2;
		line_type[count] = 2;
		lineX.push_back(count);
		count++;
		line_rho[count] = height - 1;
		line_theta[count] = THETA_MAX / 2;
		line_type[count] = 2;
		lineX.push_back(count);
		count++;
		
		// 縦方向
		line_rho[count] = 0;
		line_theta[count] = 0;
		line_type[count] = 2;
		lineY.push_back(count);
		count++;
		line_rho[count] = width - 1;
		line_theta[count] = 0;
		line_type[count] = 2;
		lineY.push_back(count);
		count++;
		
		// 横線を rho 順に並び替える
		double* y0List = new double[lineX.size()];
		for (unsigned int i = 0; i < lineX.size(); i++) 
		{
			double theta = line_theta[lineX[i]];
			double rho = line_rho[lineX[i]];
			y0List[i] = rho / cs[(int)theta];
		}
		for (unsigned int i = 0; i < lineX.size() - 1; i++) 
		{
			unsigned int min_id = i;
			double min_rho = y0List[i];
			for (unsigned int j = i + 1; j < lineX.size(); j++) 
			{
				double rho = y0List[j];
				if (rho < min_rho) 
				{
					min_rho = rho;
					min_id = j;
				}
			}
			if (min_id != i) 
			{
				std::swap(lineX[i], lineX[min_id]);
				std::swap(y0List[i], y0List[min_id]);
			}
		}
		delete y0List;
		
		// 縦線を rho 順に並び替える
		double* x0List = new double[lineY.size()];
		for (unsigned int i = 0; i < lineY.size(); i++) 
		{
			double theta = line_theta[lineY[i]];
			double rho = line_rho[lineY[i]];
			x0List[i] = rho / sn[(int)theta];
		}
		for (unsigned int i = 0; i < lineY.size() - 1; i++) 
		{
			unsigned int min_id = i;
			int min_rho = line_rho[lineY[i]];
			for (unsigned int j = i + 1; j < lineY.size(); j++) 
			{
				const int rho = line_rho[lineY[j]];
				if (rho < min_rho) 
				{
					min_rho = rho;
					min_id = j;
				}
			}
			if (min_id != i) 
			{
				std::swap(lineY[i], lineY[min_id]);
				std::swap(x0List[i], x0List[min_id]);
			}
		}
		delete x0List;
		
		for (int i = 0; i < count; i++) 
		{
			int rho = line_rho[i];
			int theta = line_theta[i];
			if (line_type[i] == 0) 
			{
				continue;
			}
			//検出した直線の描画
			//xを変化させてyを描く（垂直の線を除く）
			if(theta!=0)
			{
				for(int x=0;x<width;x++)
				{
					int y=(int)((rho-x*cs[theta])/sn[theta]);
					if(y>=height || y<0) continue;
					dstImagePixels[x + y * width] = (i < count - 4 ? 0xffff0000 : 0xffff00ff);
				}
			}
			
			//yを変化させてxを描く（水平の線を除く）
			if(theta!=THETA_MAX/2)
			{
				for(int y=0;y<height;y++)
				{
					int x=(int)((rho-y*sn[theta])/cs[theta]);
					if(x>=width || x<0) continue;
					dstImagePixels[x + y * width] = (i < count - 4 ? 0xffff0000 : 0xffff00ff);
				}
			}
		}
		
		// 交点を求めて、四角で囲まれた範囲を抽出する
		int* crossV = new int[lineX.size() * lineY.size()];
		int* crossX = new int[lineX.size() * lineY.size()];
		int* crossY = new int[lineX.size() * lineY.size()];
		
		for (unsigned int i = 0; i < lineX.size(); i++) 
		{
			int id1 = lineX[i];
			int theta = line_theta[id1];
			double rho = line_rho[id1] / cs[theta];
			double tan_theta = sn[theta] / cs[theta];
			for (unsigned int j = 0; j < lineY.size(); j++) 
			{
				int id2 = lineY[j];
				int _theta = line_theta[id2];
				double _tan_theta = sn[_theta] / cs[_theta];
				double _rho = line_rho[id2] / cs[_theta];
				int y = (rho - _rho) / (tan_theta - _tan_theta);
				int x = _rho - y * _tan_theta;
				if (x >= width || x < 0 || y >= height || y < 0) 
				{
					crossV[i + j * lineX.size()] = 0;
					continue;
				}
				crossV[i + j * lineX.size()] = 1;
				crossX[i + j * lineX.size()] = x;
				crossY[i + j * lineX.size()] = y;
			}
		}
		
		// 四角で囲まれた範囲のピクセルを解析する
#define cx(x, y) (crossX[(x) + (y) * lineX.size()])
#define cy(x, y) (crossY[(x) + (y) * lineX.size()])
#define cv(x, y) (crossV[(x) + (y) * lineX.size()])
		char* inPolygonMap = new char[width * height];
		for (unsigned int j = 0; j < lineY.size() - 1; j++) 
		{
			for (unsigned int i = 0; i < lineX.size() - 1; i++) 
			{
				if (cv(i, j) && cv(i + 1, j) && cv(i, j + 1) && cv(i + 1, j + 1)) 
				{
					POINT vertex[4];
					RECT rect;
					vertex[0].x = cx(i, j);
					vertex[0].y = cy(i, j);
					vertex[1].x = cx(i + 1, j);
					vertex[1].y = cy(i + 1, j);
					vertex[2].x = cx(i + 1, j + 1);
					vertex[2].y = cy(i + 1, j + 1);
					vertex[3].x = cx(i, j + 1);
					vertex[3].y = cy(i, j + 1);
					rect.left = rect.right = vertex[0].x;
					rect.top = rect.bottom = vertex[0].y;
					for (int k = 1; k < 4; k++) 
					{
						if (rect.left > vertex[k].x) 
						{
							rect.left = vertex[k].x;
						} 
						else if (rect.right < vertex[k].x) 
						{
							rect.right = vertex[k].x;
						}
						if (rect.top > vertex[k].y) 
						{
							rect.top = vertex[k].y;
						} 
						else if (rect.bottom < vertex[k].y) 
						{
							rect.bottom = vertex[k].y;
						}
					}
					
					ZeroMemory(inPolygonMap, width * height);
					
					// 平均値を求める
					int count = 0;
					average = 0;
					for (int y = rect.top; y <= rect.bottom; y++) 
					{
						for (int x = rect.left; x <= rect.right; x++) 
						{
							POINT p = {x, y};
							if (isPointInPolygon(p, 4, vertex)) 
							{
								// 多角形の範囲内の場合、合計を求める
								inPolygonMap[x + y * width] = 1;
								average += f(x, y);
								count++;
							}
						}
					}
					average /= count;
					
					stdDeviation = 0;
					for (int y = rect.top; y <= rect.bottom; y++) 
					{
						for (int x = rect.left; x <= rect.right; x++)
						{
							if (inPolygonMap[x + y * width] == 1) 
							{
								// 多角形の範囲内の場合、合計を求める
								double val = f(x, y) - average;
								stdDeviation += val * val;
							}
						}
					}
					stdDeviation = sqrt(stdDeviation / count);
					
					double xa = 0, ya = 0;
					const double th = average + threshold;
					for (int y = rect.top; y <= rect.bottom; y++) 
					{
						for (int x = rect.left; x <= rect.right; x++) 
						{
							if ((inPolygonMap[x + y * width] == 1)) 
							{
								if ((threshold > 0 ? (f(x, y) > th) : (f(x, y) < th))) 
								{
									dstImagePixels[x + y * width] = 0xffffff00;
								}
							}
						}
					}
					
					int detected = 0;
					
					for (int y = rect.top; y <= rect.bottom; y++) 
					{
						for (int x = rect.left; x <= rect.right; x++) 
						{
							if ((inPolygonMap[x + y * width] == 1) && (threshold > 0 ? (f(x, y) > th) : (f(x, y) < th))) 
							{
								int lcount = 0;
								int ok = 1;
								for (int j = -2; j <= 2; j++) 
								{
									int _y = y + j;
									if (_y < 0 || _y >= height) 
									{
										continue;
									}
									if ((inPolygonMap[x + _y * width] == 1) && (threshold > 0 ? (f(x, _y) > th) : (f(x, _y) < th))) 
									{
										lcount++;
									} 
									else 
									{
										lcount = 0;
									}
								}
								if (lcount < 5) 
								{
									ok = 0;
								}
								lcount = 0;
								for (int i = -2; i <= 2; i++) 
								{
									int _x = x + i;
									if (_x < 0 || _x >= width) 
									{
										continue;
									}
									if ((inPolygonMap[_x + y * width] == 1) && (threshold > 0 ? (f(_x, y) > th) : (f(_x, y) < th))) 
									{
										lcount++;
									} 
									else 
									{
										lcount = 0;
									}
								}
								if (lcount < 5) 
								{
									ok = 0;
								}
								lcount = 0;
								for (int k = 0; k < 2; k++) 
								{
									int dist = 10 - k;
									for (int j = -dist; j <= dist; j++) 
									{
										int _y = y + j;
										if (_y < 0 || _y >= height) 
										{
											continue;
										}
										for (int i = -dist; i <= dist; i++) 
										{
											if (i <= -dist || i == dist || j == -dist || j == dist) 
											{
												int _x = x + i;
												if (_x < 0 || _x >= width) 
												{
													continue;
												}
												if (threshold > 0 ? (f(_x, _y) > th) : (f(_x, _y) < th)) 
												{
													lcount++;
												}
											}
										}
									}
								}
								if (lcount < 5 && ok == 1) 
								{
									detected = 1;
									xa = x;
									ya = y;
									x = rect.right;
									y = rect.bottom;
								}
							}
						}
					}
					
					if (detected == 1) 
					{
						if (xa != 0 && ya != 0) 
						{
							// ホットスポットの位置を保存する
							for (int i = 0; i < 100; i++) 
							{
								int x = xa + sin(i * M_PI / 50) * 15;
								int y = ya + cos(i * M_PI / 50) * 15;
								if (x < 0 || y < 0 || x >= width || y >= height) 
								{
									continue;
								}
								dstImagePixels[x + y * width] = 0xffff0000;
							}
						}
					}
				}
			}
		}
		
		delete inPolygonMap;
		delete crossX;
		delete crossY;
		delete crossV;
	}
	
	// 後片付け
	delete line_type;
	delete line_theta;
	delete line_rho;
	delete tempData;
	delete edgeData;
	delete counter;
	delete cs;
	delete sn;
	destImage->UnlockBits(&dstImageData);
	
	return destImage;
}
#endif
