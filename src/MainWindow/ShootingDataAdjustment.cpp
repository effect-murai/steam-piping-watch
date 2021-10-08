/*
 * ShootingDataAdjustment.cpp
 *
 *  Created on: 2016/01/08
 *      Author: PC-EFFECT-012
 */

// 2. 撮影位置・方位調整
#include "app.h"
#include "MainWindow.h"
#include "ResultData.h"
#include "Graphics.h"

//------------------------------------------------------------------------------
// External Global Variables
//------------------------------------------------------------------------------

extern ResultData *result;

inline double getDefaultHeight() {
	TCHAR value[Resource::MAX_LOADSTRING];
	Resource::getString(IDS_DEFAULT_HEIGHT, value);
	return toDouble(value);
}

//------------------------------------------------------------------------------
// MainWindow Class
//------------------------------------------------------------------------------

bool MainWindow::canvas_Main_ShootingDataAdjustmentOnClick(void) {
	// デフォルト処理を実行
	return true;
}

bool MainWindow::canvas_Main_ShootingDataAdjustmentOnDblClick(void) {
	// デフォルト処理を実行
	return true;
}

bool MainWindow::canvas_Main_ShootingDataAdjustmentUpdate(void) {
	int i;
	// 対象画像ID取得
	int id = getSelectedId();
	if (id == -1) {
		return false;
	}

	// キャンバスサイズを取得する
	RECT rect;
	canvas_Main->getClientRect(&rect);

	// GDI+のGraphicsオブジェクトを作成
	Gdiplus::Graphics *graphics = new Gdiplus::Graphics(
			canvas_Main->getBackBuffer());
	// 画像種別
	int pictureType;

	if (!result->hasVisible()) {
		pictureType = 1;
	} else {
		pictureType = 0;
	}

	// ファイル名
	TCHAR *fileName = result->getFilePath(id, pictureType);

	// 画像の縮尺
	const double ratio = controlPanel->getZoom();

	// 画像の回転
	double direction = toRad(
			result->getBaseDirection() + controlPanel->getTurn());

	// パネルの大きさ
	double panelSizeX, panelSizeY;
	getPanelSize(&panelSizeX, &panelSizeY);

	Gdiplus::Bitmap *img = Graphics::loadBitmap(fileName, pictureType, ratio,
			-direction);

	// 画像を読み込んだBitmapを描画
	int x = (rect.left + rect.right) / 2;
	int y = (rect.bottom + rect.top) / 2;

	graphics->DrawImage(img, x - (int) img->GetWidth() / 2,
			y - (int) img->GetHeight() / 2);

	// Bitmapオブジェクトを破棄
	delete img;

	// Graphicsオブジェクトを破棄
	delete graphics;

	//　縦中央グリッド線開始、終了
	int xDepthStart = (rect.left + rect.right) / 2;
	int yDepthStart = rect.top;
	int yDepthEnd = rect.bottom;

	//ペンの作成
	int penWidth = 2;
	HPEN blackPen = CreatePen(PS_SOLID, penWidth, RGB(0, 0, 0));
	HPEN oldPen = (HPEN) canvas_Main->selectGdiObject(blackPen);
	HPEN redPen = CreatePen(PS_SOLID, penWidth, RGB(255, 0, 0));
	canvas_Main->selectGdiObject(redPen);
	HBRUSH oldBrush = (HBRUSH) canvas_Main->selectGdiObject(
			GetStockObject(NULL_BRUSH));

	int infraredWidth = result->getInfraredWidth();
	double viewAngle = result->getViewAngle();

	//　x軸幅　(ピクセル)
	double xDepthSpace = result->meterToPixel(infraredWidth, viewAngle,
			getDefaultHeight(), panelSizeX * 3);
	if (xDepthSpace < penWidth) {
		// グリッド間隔がペン幅よりも広い場合はペン幅にする
		xDepthSpace = penWidth;
	}

	// 縦のグリッド線
	for (i = 0; true; i++) {
		int lineX = xDepthStart + i * xDepthSpace;
		if (rect.right > lineX) {
			// 右側
			canvas_Main->drawLine(lineX, yDepthStart, lineX, yDepthEnd);
			// 左側
			int leftLineX = xDepthStart - i * xDepthSpace;
			canvas_Main->drawLine(leftLineX, yDepthStart, leftLineX, yDepthEnd);
		} else {
			break;
		}
	}

	//　横中央グリッド線開始、終了
	int xWidthStart = rect.left;
	int yWidthStart = (rect.bottom + rect.top) / 2;
	int xWidthEnd = rect.right;

	//　ｙ軸幅　(ピクセル)
	double yWidthSpace = result->meterToPixel(infraredWidth, viewAngle,
			getDefaultHeight(), panelSizeY * 3);
	if (yWidthSpace < penWidth) {
		// グリッド間隔がペン幅よりも広い場合はペン幅にする
		yWidthSpace = penWidth;
	}

	//　横のグリッド線
	for (i = 0; true; i++) {
		int lineY = yWidthStart + i * yWidthSpace;
		if (rect.bottom > lineY) {
			// 下側
			canvas_Main->drawLine(xWidthStart, lineY, xWidthEnd, lineY);
			// 上側
			int topLineY = yWidthStart - i * yWidthSpace;
			canvas_Main->drawLine(xWidthStart, topLineY, xWidthEnd, topLineY);
		} else {
			break;
		}
	}

	//ペン破棄
	canvas_Main->selectGdiObject(oldPen);
	DeleteObject(blackPen);
	DeleteObject(redPen);
	canvas_Main->selectGdiObject(oldBrush);

	// ID表示
	this->canvas_Main->drawId(id, result->getDataCount());
	return true;
}

LRESULT MainWindow::canvas_Main_ShootingDataAdjustmentProc(HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam) {
	return 0;
}

bool MainWindow::canvas_Main_ShootingDataAdjustmentResultSet(void) {
	// 縮尺から高度を求めてresultに格納する
	int id = getSelectedId();
	// (2016/12/20YM)高度の計算式を変更
	double height = getDefaultHeight() * controlPanel->getZoom();
	result->setHeight(id, height);
	return true;
}

bool MainWindow::canvas_Main_ShootingDataAdjustmentResultGet(void) {
	// resultの高度から縮尺を求める
	int id = getSelectedId();
	// (2016/12/20YM)高度の計算式を変更
	controlPanel->setZoom(result->getHeight(id) / getDefaultHeight());
	return true;
}

void MainWindow::SetHeightAll() {
	// 確認メッセージ表示
	int ret = this->showMessageBox(IDS_PANEL_ZOOM_ALL_CONF, IDS_PANEL_ZOOM_ALL,
	MB_YESNO);
	if (ret == IDYES) {
		// 縮尺から高度を求めてresultに格納する(全画像)
		// (2016/12/20YM)高度の計算式を変更
		double height = getDefaultHeight() * controlPanel->getZoom();
		for (int id = 0; id < result->getDataCount(); id++) {
			result->setHeight(id, height);
		}
	}
}

// (2017/4/3YM追加)『角度一括適用』ボタン追加
void MainWindow::SetTurnAll() {
	// 確認メッセージ表示
	int ret = this->showMessageBox(IDS_PANEL_TURN_ALL_CONF, IDS_PANEL_TURN_ALL,
	MB_YESNO);
	if (ret == IDYES) {
		// 縮尺から高度を求めてresultに格納する(全画像)
		double turn = toRad(controlPanel->getTurn());
		for (int id = 0; id < result->getDataCount(); id++) {
			result->setDirection(id, turn);
		}
	}
}
