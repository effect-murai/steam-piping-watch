/*
 * MainCanvas.cpp
 *
 *  Created on: 2016/03/15
 *      Author: PC-EFFECT-011
 */

#include "Graphics.h"
#include "MainCanvas.h"
#include "ResultData.h"
#include "PanelInfo.h"
#include "stdio.h"
// (2017/4/12YM)構造体変数宣言
typedef struct {
	Gdiplus::Bitmap *PicImg;
	double PicRat;
	double PicDir;
	int PicTyp;
} PictureData;

// (2017/4/12YM)グローバル変数宣言
PictureData Picdat[9999];

//------------------------------------------------------------------------------
// External Global Variables
//------------------------------------------------------------------------------
extern ResultData *result;
extern PanelData *panelData;
// (2017/4/12YM)externを追加
extern MainWindow *mainWindow;

//------------------------------------------------------------------------------
// Inline Functions
//------------------------------------------------------------------------------
inline void splitLines(LineSegment *line1, LineSegment *line2, int splitCount,
		LineSegment *splitLines);
inline bool isValidInternalPanel(int id, POINT *internalId);

#define inRange(min, max, value) (((min) <= (value)) && ((value) <= (max)))

// (2017/4/12YM)インライン関数を追加
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

	// 画像の一部分を抽出する
	Gdiplus::Bitmap *original = new Gdiplus::Bitmap(wcFileName);

	if (pictureType == ResultData::VISIBLE_LIGHT_IMAGE) {
		// (2019/11/21LEE)
		if (original->GetWidth() != 1600 || original->GetHeight() != 1200) {
			cv::Mat VBIMAGE = Graphics::loadCVImage(wcFileName);
			cv::Mat dst;
			cv::resize(VBIMAGE, dst, cv::Size(1600, 1200), 0, 0, CV_INTER_NN);
			VBIMAGE = dst;

			delete original;
			original = Graphics::toBitmap(&VBIMAGE);
		}

		double ratioV, ratioH;
		result->getCameraRatio(&ratioV, &ratioH);

		ratio /= (ratioH + ratioV) / 2;
		int width = result->getInfraredWidth() * ratioH;
		int height = result->getInfraredHeight() * ratioV;
		double x, y;
		result->getCameraOffset(&x, &y);
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

//------------------------------------------------------------------------------
// Debugging Options
//------------------------------------------------------------------------------
//#define USE_WHOLEPICTURE // 合成後の画像を使用

//------------------------------------------------------------------------------
// MainCanvas Class
//------------------------------------------------------------------------------
/**
 * スタイルを指定してメインキャンバスを作成する。
 * @param style スタイル
 * @param x 左の座標
 * @param y 上の座標
 * @param width 幅
 * @param height 高さ
 * @param parent 親ウィンドウ
 */
MainCanvas::MainCanvas(int style, int x, int y, int width, int height,
		WindowContainer *parent) :
		Canvas(style, x, y, width, height, parent) {
	// (2020/01/07LEE) Fontを区分して使用するために追加
	const TCHAR *fontFamily = NULL;
	fontFamily = TEXT("Meiryo UI");
	mainCanvasFont = CreateFont(-20, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE,
	DEFAULT_CHARSET,
	OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	DEFAULT_PITCH, fontFamily);
}

/**
 * デストラクタ
 */
MainCanvas::~MainCanvas() {
	DeleteObject(mainCanvasFont); // (2020/01/07LEE) Fontを区分して使用するために追加
}

/**
 * 画像表示(指定)
 * @param id 画像ID
 * @param pictureType 画像種別
 * ※ データ(resultData)有無は、上位で確認すること
 */
void MainCanvas::drawPicture(int id, int pictureType) {
	// GDI+のGraphicsオブジェクトを作成
	Gdiplus::Graphics *graphics = new Gdiplus::Graphics(this->getBackBuffer());

	// Bitmapイメージを読み込む
	Gdiplus::Bitmap *img =
			(Gdiplus::Bitmap*) MainWindow::getInstance()->loadPicture(id,
					pictureType);

	// 画像を読み込んだBitmapを描画
	int x, y;
	MainWindow::getInstance()->gpsPosToPixel(id, &x, &y);
	graphics->DrawImage(img, x - (int) img->GetWidth() / 2,
			y - (int) img->GetHeight() / 2);

	// Bitmapオブジェクトを破棄
	delete img;

	// Graphicsオブジェクトを破棄
	delete graphics;
}

/**
 * 画像表示(全部)
 * @param pictureType 画像種別
 * ※ データ(resultData)有無は、上位で確認すること
 */
void MainCanvas::drawPictureAll(int pictureType) {
	// GDI+のGraphicsオブジェクトを作成
	Gdiplus::Graphics *graphics = new Gdiplus::Graphics(this->getBackBuffer());

#ifdef USE_WHOLEPICTURE // 画像を張り合わせる処理
	double viewPixelPerMeter = MainWindow::getInstance()->getViewPixelPerMeter();
	double viewX = MainWindow::getInstance()->getViewX();
	double viewY = MainWindow::getInstance()->getViewY();
	
	// 俯瞰合成画像を表示
	double viewRatio2 = viewRatio * (viewRatio / viewPixelPerMeter);
	wchar_t* fileName;
	
	if (pictureType == ResultData::INFRARED_IMAGE) 
	{
		fileName = result->getfileWholeIr();
	} 
	else 
	{
		fileName = result->getfileWholeVb();
	}
	Gdiplus::Bitmap* base = new Gdiplus::Bitmap(fileName);
	
	// 中心位置をずらすのはここではない
	// マウス操作をしたときにずらす
	// 合成画像は100px/mなので100倍する
	int x1 = viewX * 100;
	int y1 = -viewY * 100;
	
	//表示幅、高さ
	int width = this->clientWidth() / viewRatio2 + 2;
	int height = this->clientHeight() / viewRatio2 + 2;
	
	Gdiplus::Bitmap* trim = Graphics::trimBitmap(base, x1 - width / 2, y1 - height / 2, width, height);
	Gdiplus::Bitmap* stretch = Graphics::stretchBitmap(trim, viewRatio2);
	
	graphics->DrawImage(stretch, 0, 0);
	
	delete base;
	base=NULL;
	delete trim;
	trim=NULL;
	delete stretch;
	stretch=NULL;
	delete graphics;
	graphics=NULL;
#else // 画像を張り合わせる処理
	for (int id = 0; id < result->getDataCount(); id++) {

		// 除外対象かどうかを確認し、除外対象の場合は描画処理を飛ばす
		if (MainWindow::getInstance()->excludePicture(id, pictureType)) {
			continue;
		}

		// (2017/4/12YM)画像データ読み込みを追加
		// 取得した高度から画像の縮尺を計算する
		const double ratio = (mainWindow->getPictureRatio(id));

		// 撮影方位を取得する
		double direction = mainWindow->getDirection(id);
		// ファイル名
		TCHAR *fileName;
		result->getFilePath(id, pictureType, &fileName);
		//　データ比較
		bool LoadPicFlag = true;
		if (Picdat[id].PicTyp == pictureType) {
			if (Picdat[id].PicRat == ratio) {
				if (Picdat[id].PicDir == direction) {
					//　データ読み込みフラグをオフ
					LoadPicFlag = false;
				}
			}
		}
		// Bitmapイメージを読み込む
		if (LoadPicFlag == true) {
			// (2017/4/16YM)Bitmap解放
			delete (Picdat[id].PicImg);
			// Bitmapイメージを読み込む
			Picdat[id].PicImg = loadBitmap(fileName, ratio, -direction,
					pictureType);
			//　画像データをセット
			Picdat[id].PicTyp = pictureType;
			Picdat[id].PicRat = ratio;
			Picdat[id].PicDir = direction;
		}
		// 後片付け
		delete fileName;
		// 画像を読み込んだBitmapを描画
		int x, y;
		MainWindow::getInstance()->gpsPosToPixel(id, &x, &y);
		graphics->DrawImage(Picdat[id].PicImg,
				x - (int) Picdat[id].PicImg->GetWidth() / 2,
				y - (int) Picdat[id].PicImg->GetHeight() / 2);
	}
#endif // 画像を張り合わせる処理

	// Graphicsオブジェクトを破棄
	delete graphics;
	graphics = NULL;
}

/**
 * 画像表示(管理番号入力)
 * @param id 画像ID
 * @param pictureType 画像種別
 * ※ データ(resultData)有無は、上位で確認すること
 */
void MainCanvas::setNumberdrawPictureAll(int pictureType, int size) {
	// 管理パネルを選択していない状態であれば、通常の画像表示(全部)を行う
	if (size == 0) {
		if (panelData->getSelectedPanelId() == PanelData::NOT_SELECTED) {
			drawPictureAll(pictureType);
			return;
		}
	}

	// GDI+のGraphicsオブジェクトを作成
	Gdiplus::Graphics *graphics = new Gdiplus::Graphics(this->getBackBuffer());

	Gdiplus::Bitmap *img_all = new Gdiplus::Bitmap(this->clientWidth(),
			this->clientHeight(), PixelFormat32bppARGB); // 読み込んだ全体画像用

	// 全体画像作成用Graphicsオブジェクト作成
	Gdiplus::Graphics *graphics2 = new Gdiplus::Graphics(img_all);

	// 画像を張り合わせる処理
	for (int id = 0; id < result->getDataCount(); id++) {
		// 除外対象かどうかを確認し、除外対象の場合は描画処理を飛ばす
		if (MainWindow::getInstance()->excludePicture(id, pictureType)) {
			continue;
		}

		// Bitmapイメージを読み込む
		Gdiplus::Bitmap *img =
				(Gdiplus::Bitmap*) MainWindow::getInstance()->loadPicture(id,
						pictureType);

		// 画像を読み込んだBitmapを描画
		int x, y;
		MainWindow::getInstance()->gpsPosToPixel(id, &x, &y);
		graphics2->DrawImage(img, x - (int) img->GetWidth() / 2,
				y - (int) img->GetHeight() / 2);

		// Bitmapオブジェクトを破棄
		delete img;
	}

	// Graphicsオブジェクトを破棄
	delete graphics2;

	// 描画処理
	// 読み込んだ画像のBitmapを描画
	Graphics::setAlphaChannel(img_all, 0.7);
	graphics->DrawImage(img_all, 0, 0);

	// Bitmapオブジェクトを破棄
	delete img_all;

	// Graphicsオブジェクトを破棄
	delete graphics;
}

/**
 * 画像枠表示(指定)
 * @param id 画像ID
 * @param pictureType 画像種別
 * ※ データ(resultData)有無は、上位で確認すること
 */
void MainCanvas::drawPictureFrame(int id, int pictureType) {
	if ((id >= 0) && (id < result->getDataCount())) {
		// 選択中の画像の周りに線を表示する
		POINT p[4];
		MainWindow::getInstance()->getPictureRect(id,
				ResultData::INFRARED_IMAGE, p);

		HPEN oldPen = (HPEN) this->selectGdiObject(
				CreatePen(PS_SOLID | PS_INSIDEFRAME, 2, RGB(255, 224, 224)));
		HBRUSH oldBrush = (HBRUSH) this->selectGdiObject(
				GetStockObject(NULL_BRUSH));
		this->drawPolygon(p, 4);
		DeleteObject(this->selectGdiObject(oldPen));
		this->selectGdiObject(oldBrush);
	}
}

/**
 * 画像ID表示
 * @param id 画像ID
 * @param maxCount 全画像数
 * ※ データ(resultData)有無は、上位で確認すること
 */
void MainCanvas::drawId(int id, int maxCount) {
	// 表示テキスト作成(idは0開始なので、+1)
	TCHAR textId[32];

	// (2017/2/9YM)IDがない場合の処理を追加
	if (id == -1) {
		snwprintf(textId, 32, TEXT("---/%03d"), maxCount);
	} else {
		snwprintf(textId, 32, TEXT("%03d/%03d"), id + 1, maxCount);
	}

	// キャンバスサイズを取得する
	RECT rect;
	this->getClientRect(&rect);

	// 表示位置設定
	int x = rect.left + 2;
	int y = rect.top + 2;

	// フォント、表示色設定
	HFONT font = this->mainCanvasFont; //(2020/01/07LEE) 

	HFONT sysFont = (HFONT) this->selectGdiObject(font);
	COLORREF defTextColor = GetTextColor(this->getBackBuffer());
	int defBkMode = SetBkMode(this->getBackBuffer(), TRANSPARENT);

	// 影の表示
	SetTextColor(this->getBackBuffer(), RGB(0, 0, 0));
	this->drawText(x - 1, y - 1, textId);	// 左上
	this->drawText(x - 1, y + 1, textId);	// 左下
	this->drawText(x + 1, y - 1, textId);	// 右上
	this->drawText(x + 1, y + 1, textId);	// 右下

	// IDの表示
	SetTextColor(this->getBackBuffer(), RGB(255, 255, 255));
	this->drawText(x, y, textId);

	// フォント、表示色を元に戻す
	SetBkMode(this->getBackBuffer(), defBkMode);
	SetTextColor(this->getBackBuffer(), defTextColor);
	this->selectGdiObject(sysFont);
}

/**
 * ホットスポット表示
 * @param radius ホットスポット表示サイズ(半径)
 * ※ データ(resultData)有無は、上位で確認すること
 */
void MainCanvas::drawHotspotAll(int radius) {
	HPEN whitePen = CreatePen(PS_SOLID, 6, RGB(255, 255, 255)); // (2019/10/29LEE) Penのsizeを変更 4=>6
	HPEN bluePen = CreatePen(PS_SOLID, 4, RGB(0, 255, 255)); // (2019/10/29LEE) Penのsizeを変更 2=>4
	HPEN orangePen = CreatePen(PS_SOLID, 2, RGB(255, 150, 0));
	HPEN redPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0)); //　(2019/10/03LEE)追加。
	HBRUSH oldBrush = (HBRUSH) this->selectGdiObject(
			GetStockObject(NULL_BRUSH));
	// (2017/7/25YM)ペンを追加
	HPEN paintPen2 = CreatePen(PS_SOLID, 2, RGB(0, 96, 255));

	// (2019/12/23LEE) 四角線対応

	HotspotNumber hotnum;
	HotspotNumber checknum;
	// 点の表示位置を計算し、点を表示する
	for (int i = 0; i < (int) result->getAllHotspotCount(); i++) {
		checknum = result->getAllHotspotNo(i);
		int x, y;
		Vector2D pos = result->getAllHotspotPos(i);
		MainWindow::getInstance()->gpsPosToPixel(pos.x, pos.y, &x, &y);
		// 出力するホットスポットリストのidと一致すればオレンジペン
		bool reportOut = false;
		// 出力対象のホットスポットかどうかを確認する
		std::vector<HotspotIdList> hotspotIdList =
				MainWindow::getInstance()->getHotspotIdList();
		for (std::vector<HotspotIdList>::iterator item = hotspotIdList.begin();
				item != hotspotIdList.end(); item++) {
			if (item->picnum == checknum.pictureNo
					&& item->hotnum == checknum.pointNo) {
				MainWindow::getInstance()->resaveHotspotIdList(i, item->picnum,
						item->hotnum);
				reportOut = true;
				hotnum = result->getAllHotspotNo(item->id);
				MainWindow::getInstance()->saveHotspotIdList();
				break;
			}
		}

		//ホットスポットの点を描画
		HPEN oldPen = (HPEN) this->selectGdiObject(whitePen);
		this->drawCircle(x, y, radius);
		if (reportOut) {
			//(2019/10/03LEE)　HOMEで詳細表示個所を選択、その際、報告書の表示範囲を四角等で表示するコード。
			//(2019/12/17LEE) RECTのSizeを対応

			//(2019/12/23LEE) 四角船対応 (2020/02/06LEE) 表示はMinimapでして取り合えず、コメント処理
			this->selectGdiObject(orangePen);
		} else {
			// (2017/7/25YM)ホットスポットの温度別に色分けする機能を追加
			float temp = result->getHotspotTemp(i);
			float panelavetemp = result->getPanelAveTemp(i);
			double prevthre = mainWindow->getThresholdTemp() + panelavetemp;
			if (prevthre < 0) {
				prevthre = 0;
			}
			if (temp >= (prevthre + 20)) {
				this->selectGdiObject(bluePen);
			} else {
				this->selectGdiObject(paintPen2);
			}
		}
		this->drawCircle(x, y, radius);
		this->selectGdiObject(oldPen);
	}

	DeleteObject(whitePen);
	DeleteObject(redPen); // (2019/10/03LEE) 追加。
	DeleteObject(bluePen);
	DeleteObject(orangePen);
	// (2017/7/25YM)ペンの削除を追加
	DeleteObject(paintPen2);
	this->selectGdiObject(oldBrush);
}

/**
 * ホットスポット表示(選択画像内)
 * @param id 画像ID
 * @param radius ホットスポット表示サイズ(半径)
 * ※ データ(resultData)有無は、上位で確認すること
 */
void MainCanvas::drawHotspot(int id, int radius) {
	// ホットスポットを赤い点で描画
	COLORREF color = RGB(255, 0, 0);
	// 熱画像サイズを取得する
	const int infraredWidth = result->getInfraredWidth();
	const int infraredHeight = result->getInfraredHeight();
	// 取得した高度から画像の縮尺を計算する
	const double ratio = MainWindow::getInstance()->getPictureRatio(id);

	// 撮影方位を取得する
	const double direction = MainWindow::getInstance()->getDirection(id);

	int cx, cy;
	// 画像の表示位置を計算する
	MainWindow::getInstance()->gpsPosToPixel(id, &cx, &cy);	//　(2017/3/30YM)Noiの画像の表示位置を計算する

	// 描画ペンとブラシを設定する
	HPEN outsidePen = CreatePen(PS_SOLID, 6, RGB(255, 255, 255)); // (2019/10/29LEE) circle size 4=>6
	HPEN insidePen = CreatePen(PS_SOLID, 4, color);	// (2019/10/29LEE) circle size 2=>4
	HBRUSH oldBrush = (HBRUSH) this->selectGdiObject(
			GetStockObject(NULL_BRUSH));

	// 外側のペンをデフォルトとして選択する
	HPEN oldPen = (HPEN) this->selectGdiObject(outsidePen);

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
			this->drawCircle(x, y, radius);
			this->selectGdiObject(insidePen);
			this->drawCircle(x, y, radius);
			this->selectGdiObject(outsidePen);
		}
	}

	// ペンとブラシを元に戻す
	this->selectGdiObject(oldBrush);
	this->selectGdiObject(oldPen);

	DeleteObject(outsidePen);
	DeleteObject(insidePen);
}

/**
 * ホットスポット表示(解析範囲内)
 * @param radius ホットスポット表示サイズ(半径) drawHotspot
 * ※ データ(resultData)有無は、上位で確認すること
 */
void MainCanvas::drawHotspotArea(int radius) {
	HRGN HotSpotAreaRgn = MainWindow::getInstance()->getHotSpotAreaRgn();
	// ホットスポットを表示する
	//@TODO
	HPEN whitePen = CreatePen(PS_SOLID, 4, RGB(255, 255, 255));
	HPEN bluePen = CreatePen(PS_SOLID, 2, RGB(255, 0, 255));
	HBRUSH oldBrush = (HBRUSH) this->selectGdiObject(
			GetStockObject(NULL_BRUSH));
	// 点の表示位置を計算し、点を表示する
	for (int i = 0; i < (int) result->getAllHotspotCount(); i++) {
		int x, y;
		Vector2D pos = result->getAllHotspotPos(i);
		MainWindow::getInstance()->gpsPosToPixel(pos.x, pos.y, &x, &y);
		if (HotSpotAreaRgn != NULL) {
			// 解析範囲設定がある場合
			if (PtInRegion(HotSpotAreaRgn, x, y) != 0) {
				// 範囲内のみ表示
				HPEN oldPen = (HPEN) this->selectGdiObject(whitePen);
				this->drawCircle(x, y, radius);
				this->selectGdiObject(bluePen);
				this->drawCircle(x, y, radius);
				this->selectGdiObject(oldPen);
			}
		} else {
			// 全部表示
			HPEN oldPen = (HPEN) this->selectGdiObject(whitePen);
			this->drawCircle(x, y, radius);
			this->selectGdiObject(bluePen);
			this->drawCircle(x, y, radius);
			this->selectGdiObject(oldPen);
		}
	}
	DeleteObject(whitePen);
	DeleteObject(bluePen);
	this->selectGdiObject(oldBrush);
}

/**
 * ホットスポット解析範囲表示
 */
void MainCanvas::drawAreaSetting() {
	const HotSpotArea &HotSpotAreas =
			MainWindow::getInstance()->getHotSpotAreas();
	// 解析範囲設定がない場合は、処理なし
	if (HotSpotAreas.empty()) {
		return;
	}
	// ペンを変更
	HPEN whitePen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
	HPEN oldPen = (HPEN) this->selectGdiObject(whitePen);

	// 始点
	Vector2D pos0 = (*HotSpotAreas.begin());
	Vector2D pos = pos0;
	for (HotSpotArea::const_iterator it = HotSpotAreas.begin();
			it != HotSpotAreas.end(); it++) {
		POINT prevPos;
		POINT currentPos;
		MainWindow::getInstance()->gpsPosToPixel(&pos, &prevPos);
		Vector2D _pos = *it;
		MainWindow::getInstance()->gpsPosToPixel(&_pos, &currentPos);

		// 点を表示する
		const int radius = 2;
		this->drawCircle(currentPos.x, currentPos.y, radius);
		// 点を結ぶ
		this->drawLine(prevPos.x, prevPos.y, currentPos.x, currentPos.y);
		pos = *it;
	}
	// 終点
	if (MainWindow::getInstance()->IsAreaComplete()) {
		POINT prevPos;
		POINT currentPos;
		MainWindow::getInstance()->gpsPosToPixel(&pos, &prevPos);
		MainWindow::getInstance()->gpsPosToPixel(&pos0, &currentPos);
		// 終点と始点を結ぶ
		this->drawLine(prevPos.x, prevPos.y, currentPos.x, currentPos.y);
	}

	// ペンを元に戻す
	this->selectGdiObject(oldPen);
	DeleteObject(whitePen);
}

/**
 * 管理パネル表示
 */
void MainCanvas::drawPanelSetting() {
	// 管理パネルデータがない場合は、処理なし
	if (panelData == NULL) {
		return;
	}

	const int panelCount = panelData->getPanelNameCountMax();
	const int selectedId = panelData->getSelectedPanelId();
	HPEN whitePen = CreatePen(PS_SOLID, 3, RGB(255, 255, 255));
	HPEN blackPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	HBRUSH oldBrush = (HBRUSH) this->selectGdiObject(
			(HBRUSH) GetStockObject(NULL_BRUSH));

	for (int id = 0; id < panelCount; id++) {
		if ((selectedId != -1) && (id == selectedId)) {
			continue;
		}

		POINT pt[4];
		// 4点の実世界座標をキャンバス座標に変換
		for (int i = 0; i < 4; i++) {
			Vector2D realPt = panelData->getPoint(id, i);
			MainWindow::getInstance()->gpsPosToPixel(&realPt, &pt[i]);
		}
		HPEN oldPen = (HPEN) this->selectGdiObject(whitePen);
		this->drawPolygon(pt, 4);
		this->selectGdiObject(blackPen);
		this->drawPolygon(pt, 4);
		this->selectGdiObject(oldPen);
		this->drawPanelName(id);
	}

	this->selectGdiObject(oldBrush);
	DeleteObject(whitePen);
	DeleteObject(blackPen);
	this->drawSelectedPanel(selectedId);
}

void MainCanvas::drawSelectedPanel(int id) {
	const int PANEL_COUNT = panelData->getPanelNameCountMax();
	if (!inRange(0, PANEL_COUNT - 1, id)) {
		return;
	}
	// 4点の実世界座標をキャンバス座標に変換
	POINT canvasPt[4];
	Vector2D realPt[4];
	for (int i = 0; i < 4; i++) {
		realPt[i] = panelData->getPoint(id, i);
		MainWindow::getInstance()->gpsPosToPixel(&realPt[i], &canvasPt[i]);
	}

	SIZE split;
	int rows, columns;
	panelData->getInternalSplitCount(id, &split);
	columns = split.cx;
	rows = split.cy;

	LineSegment line1, line2;
	LineSegment hSplitLines[columns + 1];
	LineSegment vSplitLines[rows + 1];

	// 各辺を分割する
	// pt[0] - pt[3]
	// 水平
	line1.pt1 = realPt[0];
	line1.pt2 = realPt[3];
	// pt[1] - pt[2]
	// 水平
	line2.pt1 = realPt[1];
	line2.pt2 = realPt[2];
	splitLines(&line1, &line2, columns, hSplitLines);

	// pt[0] - pt[1]
	// 垂直
	line1.pt1 = realPt[0];
	line1.pt2 = realPt[1];
	// pt[3] - pt[2]
	// 垂直
	line2.pt1 = realPt[3];
	line2.pt2 = realPt[2];
	splitLines(&line1, &line2, rows, vSplitLines);

	int x1, y1, x2, y2;
	//　白線で内部の分割線を描く
	HPEN whiteDotPen = CreatePen(PS_DOT, 0, RGB(255, 255, 255));
	HPEN oldPen = (HPEN) this->selectGdiObject(whiteDotPen);
	for (int j = 1; j < columns; j++) {
		MainWindow::getInstance()->gpsPosToPixel(hSplitLines[j].pt1.x,
				hSplitLines[j].pt1.y, &x1, &y1);
		MainWindow::getInstance()->gpsPosToPixel(hSplitLines[j].pt2.x,
				hSplitLines[j].pt2.y, &x2, &y2);
		this->drawLine(x1, y1, x2, y2);
	}
	for (int i = 1; i < rows; i++) {
		MainWindow::getInstance()->gpsPosToPixel(vSplitLines[i].pt1.x,
				vSplitLines[i].pt1.y, &x1, &y1);
		MainWindow::getInstance()->gpsPosToPixel(vSplitLines[i].pt2.x,
				vSplitLines[i].pt2.y, &x2, &y2);
		this->drawLine(x1, y1, x2, y2);
	}
	this->selectGdiObject(oldPen);
	DeleteObject(whiteDotPen);

	HPEN greenPen = CreatePen(PS_SOLID, 3, RGB(21, 255, 21));
	HBRUSH oldBrush = (HBRUSH) this->selectGdiObject(
			(HBRUSH) GetStockObject(NULL_BRUSH));

	this->selectGdiObject(greenPen);
	this->drawPolygon(canvasPt, 4);

	this->selectGdiObject(oldPen);
	DeleteObject(greenPen);

	POINT internalId = panelData->getSelectedInternalId();
	if (isValidInternalPanel(id, &internalId)) {
		// 四角の描画
		Vector2D panelArea[4];
		panelData->getInternalSquare(id, internalId.x, internalId.y, panelArea);
		HPEN bluePen = CreatePen(PS_SOLID, 2, RGB(0, 255, 255));
		HPEN oldPen = (HPEN) this->selectGdiObject(bluePen);
		POINT canvasArea[4];
		for (int i = 0; i < 4; i++) {
			MainWindow::getInstance()->gpsPosToPixel(&panelArea[i],
					&canvasArea[i]);
		}
		this->drawPolygon(canvasArea, 4);
		this->selectGdiObject(oldPen);
		DeleteObject(bluePen);
	}
	this->selectGdiObject(oldBrush);
	this->drawPanelName(id);
}

void MainCanvas::drawMultiSelectedPanels(int *selected, int count) {
	HPEN bluePen = CreatePen(PS_SOLID, 3, RGB(255, 21, 21));
	HPEN oldPen = (HPEN) this->selectGdiObject(bluePen);
	HBRUSH oldBrush = (HBRUSH) this->selectGdiObject(
			(HBRUSH) GetStockObject(NULL_BRUSH));

	// 4点の実世界座標をキャンバス座標に変換
	POINT pt[4];
	for (int j = 0; j < count; j++) {
		for (int i = 0; i < 4; i++) {
			Vector2D realPt = panelData->getPoint(selected[j], i);
			MainWindow::getInstance()->gpsPosToPixel(&realPt, &pt[i]);
		}
		this->drawPolygon(pt, 4);
	}
	this->selectGdiObject(oldPen);
	this->selectGdiObject(oldBrush);
	DeleteObject(bluePen);
}

void MainCanvas::drawPanelName(int id) {
	double x = 0, y = 0;
	// 4点の実世界座標をキャンバス座標に変換
	for (int i = 0; i < 4; i++) {
		Vector2D realPt = panelData->getPoint(id, i);
		x += realPt.x;
		y += realPt.y;
	}
	x /= 4;
	y /= 4;
	int cx, cy;
	MainWindow::getInstance()->gpsPosToPixel(x, y, &cx, &cy);
	COLORREF defTextColor = GetTextColor(this->getBackBuffer());
	HFONT font = (HFONT) SendMessage(this->getHandle(), WM_GETFONT, (WPARAM) 0,
			(LPARAM) 0);
	HFONT sysFont = (HFONT) this->selectGdiObject(font);
	int defBkMode = SetBkMode(this->getBackBuffer(), TRANSPARENT);
	RECT rect;
	ZeroMemory(&rect, sizeof(RECT));
	DrawText(this->getBackBuffer(), panelData->getPanelName(id), -1, &rect,
	DT_CALCRECT | DT_LEFT | DT_TOP);
	cx -= rect.right / 2;
	cy -= rect.bottom / 2;

	SetTextColor(this->getBackBuffer(), RGB(255, 255, 255));
	for (int j = -1; j <= 1; j++) {
		for (int i = -1; i <= 1; i++) {
			this->drawText(cx + i, cy + j, panelData->getPanelName(id));
		}
	}

	SetTextColor(this->getBackBuffer(), RGB(192, 0, 64));
	this->drawText(cx, cy, panelData->getPanelName(id));

	SetBkMode(this->getBackBuffer(), defBkMode);
	SetTextColor(this->getBackBuffer(), defTextColor);
	this->selectGdiObject(sysFont);
}

/**
 * 座標のクリップ
 */
void MainCanvas::clipPos(POINT &pos) {
	// ウィンドウサイズを取得
	RECT rect;
	this->getClientRect(&rect);

	// ウィンドウ枠のサイズを取得
	int clip_size_x = GetSystemMetrics(SM_CXBORDER);
	int clip_siez_y = GetSystemMetrics(SM_CYBORDER);

	// クリップ範囲設定
	rect.left += (LONG) clip_size_x;
	rect.right -= (LONG) clip_size_x;
	rect.top += (LONG) clip_siez_y;
	rect.bottom -= (LONG) clip_siez_y;

	// X座標クリップ
	if (pos.x < rect.left) {
		pos.x = rect.left;
	} else if (pos.x > rect.right) {
		pos.x = rect.right;
	}

	// Y座標クリップ
	if (pos.y < rect.top) {
		pos.y = rect.top;
	} else if (pos.y > rect.bottom) {
		pos.y = rect.bottom;
	}
}

void MainCanvas::drawHotspotShowMode(void) {
	int cx = this->clientWidth() - 10;
	int cy = this->clientHeight() - 10;

	HBRUSH oldBrush = (HBRUSH) this->selectGdiObject(
			(HBRUSH) GetStockObject(NULL_BRUSH));

	COLORREF defTextColor = GetTextColor(this->getBackBuffer());
	HFONT font = this->mainCanvasFont; //(2020/01/07LEE) Font 変更

	HFONT sysFont = (HFONT) this->selectGdiObject(font);
	int defBkMode = SetBkMode(this->getBackBuffer(), TRANSPARENT);

	const int MENU_COUNT = 4;
	struct {
		LPCTSTR text;
		bool mode;
		COLORREF color[2];
	} menu[MENU_COUNT], *menuItem;

	TCHAR title[256];
	int step = MainWindow::getInstance()->getHotspotShowMode();
	switch (step) {
	case 0:
		wsprintf(title, TEXT("%s"), TEXT("全非表示モード"));
		break;
	case 1:
		wsprintf(title, TEXT("%s"), TEXT("リンク表示モード"));
		break;
	case 2:
		wsprintf(title, TEXT("%s"), TEXT("全表示モード"));
		break;

	}

	menuItem = &menu[0];
	menuItem->text = (LPCTSTR) title;
	menuItem->mode = false;
	menuItem->color[0] = RGB(255, 255, 255);
	menuItem->color[1] = RGB(255, 255, 255);

	menuItem = &menu[1];
	menuItem->text = TEXT("特徴点");
	menuItem->mode = MainWindow::getInstance()->getKeypointShowMode();
	menuItem->color[0] = RGB(0, 0, 255);
	menuItem->color[1] = RGB(0, 0, 128);

	menuItem = &menu[2];
	menuItem->text = TEXT("ホットスポット");
	menuItem->mode = MainWindow::getInstance()->getEnabledHotspotShowMode()
			| MainWindow::getInstance()->getHotspotCandidateShowMode();
	menuItem->color[0] = RGB(255, 0, 0);
	menuItem->color[1] = RGB(128, 0, 0);

	menuItem = &menu[3];
	menuItem->text = TEXT("ホットスポット(削除候補)");
	menuItem->mode = MainWindow::getInstance()->getDisabledHotspotShowMode();
	menuItem->color[0] = RGB(192, 192, 192);
	menuItem->color[1] = RGB(96, 96, 96);

	int maxWidth = 0, maxHeight = 0;
	for (int i = 0; i < MENU_COUNT; i++) {
		RECT rect;
		ZeroMemory(&rect, sizeof(RECT));
		DrawText(this->getBackBuffer(), menu[i].text, -1, &rect,
		DT_CALCRECT | DT_LEFT | DT_TOP);
		if (maxWidth < rect.right) {
			maxWidth = rect.right;
		}
		if (maxHeight < rect.bottom) {
			maxHeight = rect.bottom;
		}
	}

	cx -= maxWidth;
	for (int type = MENU_COUNT - 1; type >= 0; type--) {
		cy -= maxHeight;
		COLORREF shadowColor;
		if (menu[type].mode == true) {
			shadowColor = RGB(255, 255, 255);
		} else {
			shadowColor = RGB(128, 128, 128);
		}
		SetTextColor(this->getBackBuffer(), shadowColor);
		for (int j = -1; j <= 1; j++) {
			for (int i = -1; i <= 1; i++) {
				this->drawText(cx + i, cy + j, menu[type].text);
			}
		}

		COLORREF textColor;
		if (menu[type].mode == true) {
			textColor = menu[type].color[0];
		} else {
			textColor = menu[type].color[1];
		}
		SetTextColor(this->getBackBuffer(), textColor);
		this->drawText(cx, cy, menu[type].text);

		if (menu[type].mode == true) {
			// 表示されているときのみ凡例の円を表示する
			HPEN outsidePen = CreatePen(PS_SOLID, 4, shadowColor);
			HPEN insidePen = CreatePen(PS_SOLID, 2, textColor);
			HPEN oldPen = (HPEN) this->selectGdiObject(outsidePen);
			this->drawCircle(cx - maxHeight + 10, cy + maxHeight / 2,
					maxHeight / 2 - 4);
			this->selectGdiObject(insidePen);
			this->drawCircle(cx - maxHeight + 10, cy + maxHeight / 2,
					maxHeight / 2 - 4);
			this->selectGdiObject(oldPen);
			DeleteObject(insidePen);
			DeleteObject(outsidePen);
		}
	}

	this->selectGdiObject(oldBrush);

	SetBkMode(this->getBackBuffer(), defBkMode);
	SetTextColor(this->getBackBuffer(), defTextColor);
	this->selectGdiObject(sysFont);
}

// (2017/4/16YM)画像タイプを再読み込みフラグに変更する関数追加
void MainCanvas::PicTypReset(void) {
	for (int i = 0; i < result->getDataCount(); i++) {
		Picdat[i].PicTyp = 9;
	}
}

//------------------------------------------------------------------------------
// For drawSelectedPanel Function
//------------------------------------------------------------------------------

inline Vector2D* splitLine(LineSegment *line, int splitCount) {
	if (splitCount <= 0) {
		return NULL;
	}

	Vector2D *splitPoints = new Vector2D[splitCount + 1];

	double dx = line->pt2.x - line->pt1.x;
	double dy = line->pt2.y - line->pt1.y;
	if (dx != 0) {
		// ※垂直は計算不可
		const double m = dy / dx;
		const double n = line->pt1.y - m * line->pt1.x;
		for (int i = 1; i < splitCount; i++) {
			splitPoints[i].x = dx * i / splitCount + line->pt1.x;
			splitPoints[i].y = m * splitPoints[i].x + n;
		}
	} else {
		// 垂直の場合
		for (int i = 1; i < splitCount; i++) {
			splitPoints[i].x = line->pt1.x;
			splitPoints[i].y = dy * i / splitCount + line->pt1.y;
		}
	}
	return splitPoints;
}

inline void splitLines(LineSegment *line1, LineSegment *line2, int splitCount,
		LineSegment *splitLines) {
	// line1
	Vector2D *splitPoints1 = splitLine(line1, splitCount);
	// line2
	Vector2D *splitPoints2 = splitLine(line2, splitCount);

	for (int j = 1; j < splitCount; j++) {
		splitLines[j].pt1 = splitPoints1[j];
		splitLines[j].pt2 = splitPoints2[j];
	}

	delete splitPoints1;
	delete splitPoints2;
}

inline bool isValidInternalPanel(int id, POINT *internalId) {
	if (id < 0) {
		return false;
	}

	SIZE splitCount;
	panelData->getInternalSplitCount(id, &splitCount);
	if (inRange(1, splitCount.cx + 1,
			internalId->x) && inRange(1, splitCount.cy + 1, internalId->y)) {
		return true;
	}
	return false;
}
