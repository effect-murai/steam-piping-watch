/*
 *
 *  Created on: 2016/01/08
 *      Author: PC-EFFECT-012
 */

// 1. カメラ調整
#include "MainWindow.h"
#include "ResultData.h"
#include "Graphics.h"

//------------------------------------------------------------------------------
// External Global Variables
//------------------------------------------------------------------------------

extern ResultData *result;

//------------------------------------------------------------------------------
// MainWindow Class
//------------------------------------------------------------------------------

bool MainWindow::canvas_Main_CameraAdjustmentOnClick(void) {
	// デフォルト処理を実行
	return true;
}

bool MainWindow::canvas_Main_CameraAdjustmentOnDblClick(void) {
	// デフォルト処理を実行
	return true;
}

bool MainWindow::canvas_Main_CameraAdjustmentUpdate(void) {
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
	for (int pictureType = 0; pictureType < 2; pictureType++) {
		// ファイル名
		TCHAR *fileName = result->getFilePath(id, pictureType);
		// 画像の縮尺 , カメラ特性MENUで写真のsizeなどを変わる事
		double ratio = 1;

		if (pictureType == ResultData::INFRARED_IMAGE) {
			ratio = controlPanel->getZoom();
		}
		// 画像の回転
		double direction = 0;
		if (pictureType == ResultData::INFRARED_IMAGE) {
			direction = toRad(controlPanel->getTurn());
		}

		// Bitmapイメージを読み込む (2019/11/15LEE) pictureTypeを追加。

		Gdiplus::Bitmap *img = Graphics::loadBitmap(fileName, pictureType,
				ratio, -direction);

		// 熱画像半透明
		if (pictureType == ResultData::INFRARED_IMAGE) {
			Graphics::setAlphaChannel(img, 0.8);
		}

		//　mainCanvas中心(可視画像位置固定)
		int x = (rect.left + rect.right) / 2;
		int y = (rect.bottom + rect.top) / 2;

		// 熱画像設定位置

		if (pictureType == 1) {
			x = (rect.left + rect.right) / 2 + controlPanel->getMoveH();
			y = (rect.bottom + rect.top) / 2 - controlPanel->getMoveV();
		}

		graphics->DrawImage(img, x - (int) img->GetWidth() / 2,
				y - (int) img->GetHeight() / 2);

		// Bitmapオブジェクトを破棄
		delete img;

	}
	// Graphicsオブジェクトを破棄
	delete graphics;
	// ID表示
	this->canvas_Main->drawId(id, result->getDataCount());
	return true;
}

// (2019/11/06LEE) 追加。
bool MainWindow::Reset(void) {
	// 確認メッセージ表示
	int ret = this->showMessageBox(IDS_PANEL_RESET_TITLE, IDS_PANEL_RESET,
	MB_YESNO);
	if (ret == IDYES) {
		// 縮尺から高度を求めてresultに格納する(全画像)
		for (int id = 0; id < result->getDataCount(); id++) {
			result->resetalldata(id);	// (2019/11/29LEE) 追加
		}
		return true;
	} else {
		return false;
	}

}

LRESULT MainWindow::canvas_Main_CameraAdjustmentProc(HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam) {
	return 0;
}
