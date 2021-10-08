/*
 * MinimapCanvas.cpp
 *
 *  Created on: 2016/02/10
 *      Author: PC-EFFECT-012
 */

#include "MainWindow.h"
#include "ResultData.h"
#include "HotspotLinker.h"

//------------------------------------------------------------------------------
// External Global Variables
//------------------------------------------------------------------------------

extern ResultData *result;
extern HotspotLinker *hotspotLinker;

//------------------------------------------------------------------------------
// Global Functions
//------------------------------------------------------------------------------

inline void gpsPosToMiniMapPixel(double x, double y, int width, int height,
		int *mapX, int *mapY) {
	// ミニマップの縦横を計算
	double west, south, east, north;
	result->getDataAreaSize2(&west, &south, &east, &north);
	const double right = east - west;
	const double top = north - south;

	// 長辺を1とした縦横の倍率を求める
	double ratioVertical = 1, ratioHorizontal = 1;
	if (top < right) {
		// 横長の場合は縦の倍率が1未満になる
		ratioVertical = top / right;
	} else {
		// 縦長の場合は横の倍率が1未満になる
		ratioHorizontal = right / top;
	}

	const double dx = (x - west) / right;
	const double dy = (y - south) / top;

	// x座標を計算
	*mapX = floor(dx * width * ratioHorizontal);

	// y座標を計算(pixel座標系ではY軸が反転しているため、y=height-y'で計算
	*mapY = floor(height - dy * height * ratioVertical);
}

void gpsPosToMiniMapPixel(int id, int width, int height, int *x, int *y) {
	double gpsX, gpsY;
	result->getXY(id, &gpsX, &gpsY);
	// 座標を計算
	gpsPosToMiniMapPixel(gpsX, gpsY, width, height, x, y);
}

void miniMapPixelToGPSPos(int x, int y, int width, int height, double *gpsX,
		double *gpsY) {
	// ミニマップの縦横を計算
	double west, south, east, north;
	result->getDataAreaSize2(&west, &south, &east, &north);
	const double right = east - west;
	const double top = north - south;

	// 長辺を1とした縦横の倍率を求める
	double ratioVertical = 1, ratioHorizontal = 1;
	if (top < right) {
		// 横長の場合は縦の倍率が1未満になる
		ratioVertical = top / right;
	} else {
		// 縦長の場合は横の倍率が1未満になる
		ratioHorizontal = right / top;
	}

	// 緯度を計算
	*gpsX = x * right / (width * ratioHorizontal) + west;

	// 経度を計算
	*gpsY = (height - y) * top / (height * ratioVertical) + south;
}

int getNearestId(double x, double y) {
	const int count = result->getDataCount();
	if (count <= 0) {
		// データが一つもない場合
		return -1;
	}
	// とりあえず最初の点をもっとも近いに設定する。
	double px, py;
	result->getXY(0, &px, &py);
	double minDist = pow(px - x, 2) + pow(py - y, 2);
	int minId = 0;

	//　２つ目の点から一つずつ近い点を調べる。
	for (int i = 1; i < count; i++) {
		result->getXY(i, &px, &py);
		double dist = pow(px - x, 2) + pow(py - y, 2);
		if (dist < minDist) {
			minDist = dist;
			minId = i;
		}
	}
	return minId;
}

//------------------------------------------------------------------------------
// MiniMap Canvas Functions
//------------------------------------------------------------------------------

void MainWindow::canvas_MiniMapOnClicked(void) {
	// フォルダを指定する前は何もしない
	if (result == NULL) {
		return;
	}

	// コントロールパネルの値を保存
	resultDataSave();

	// キャンバスサイズを取得する
	RECT rect;
	canvas_MiniMap->getClientRect(&rect);

	const int miniMapWidth = 280;
	const int miniMapHeight = 280;

	const int leftMargin = (rect.right - rect.left - miniMapWidth) / 2;
	const int topMargin = (rect.bottom - rect.top - miniMapHeight) / 2;

	// 点をクリックしたかどうか確認する
	int x, y;
	for (int id = 0; id < result->getDataCount(); id++) {
		gpsPosToMiniMapPixel(id, miniMapWidth, miniMapHeight, &x, &y);
		int dx = (x + leftMargin) - minimapX;
		int dy = (y + topMargin) - minimapY;
		if (dx * dx + dy * dy < 25) {
			// 点をクリックしていた場合、その画像を最前面に出す
			selectedPictureId = id;
			if (selectedPictureId != -1) {
				controlPanel->enableButton();
				controlPanel->updateMode();
			} else {
				controlPanel->disableButton();
			}
			resultDataWrite();
			// 表示を更新する
			canvasUpdate();
			break;
		}
	}
}

void MainWindow::canvas_MiniMapOnDblClicked(void) {
	// フォルダを指定する前は何もしない
	if (result == NULL) {
		return;
	}

	// データがない場合も何もしない
	if (result->getDataCount() == 0) {
		return;
	}

	// キャンバスサイズを取得する
	RECT rect;
	canvas_MiniMap->getClientRect(&rect);

	const int miniMapWidth = 280;
	const int miniMapHeight = 280;

	const int leftMargin = (rect.right - rect.left - miniMapWidth) / 2;
	const int topMargin = (rect.bottom - rect.top - miniMapHeight) / 2;

	double px, py;
	miniMapPixelToGPSPos(minimapX - leftMargin, minimapY - topMargin,
			miniMapWidth, miniMapHeight, &px, &py);

	int nearestId = getNearestId(px, py);
	if (nearestId != selectedPictureId) {
		// 最も近い画像を最前面に出す
		selectedPictureId = nearestId;
	}
	// ボタンの有効化/無効化
	if (selectedPictureId != -1) {
		controlPanel->enableButton();
		controlPanel->updateMode();
	} else {
		controlPanel->disableButton();
	}
	// 表示を更新する
	canvasUpdate();
}

void MainWindow::canvas_MiniMapUpdate(void) {
	// ミニマップの描画
	canvas_MiniMap->clear((HBRUSH) GetStockObject(WHITE_BRUSH));

	// フォルダを指定する前は何もしない
	if (result == NULL) {
		return;
	}

	// データがない場合も何もしない
	if (result->getDataCount() == 0) {
		return;
	}

	bool *linked;
	if (controlPanel->getMode() == ControlPanel::CONTROL_PANEL_MODE_HOTSPOT) {
		linked = hotspotLinker->linkedPictures(0);
	} else {
		linked = new bool[result->getDataCount()];
		ZeroMemory(linked, sizeof(bool) * result->getDataCount());
	}

	// キャンバスサイズを取得する
	RECT rect;
	canvas_MiniMap->getClientRect(&rect);

	const int miniMapWidth = 280;
	const int miniMapHeight = 280;

	const int leftMargin = (rect.right - rect.left - miniMapWidth) / 2;
	const int topMargin = (rect.bottom - rect.top - miniMapHeight) / 2;

	// 縦座標軸を描画
	canvas_MiniMap->drawLine(rect.left + leftMargin, rect.bottom - topMargin,
			rect.left + leftMargin, rect.top + topMargin);

	// 横座標軸を描画
	canvas_MiniMap->drawLine(rect.left + leftMargin, rect.bottom - topMargin,
			rect.right - leftMargin, rect.bottom - topMargin);

	// 画像の位置を点で表示する
	HBRUSH oldBrush = (HBRUSH) canvas_MiniMap->selectGdiObject(
			CreateSolidBrush(RGB(0, 0, 255)));
	for (int i = 0; i < result->getDataCount(); i++) {
		int x, y, r = 5;
		gpsPosToMiniMapPixel(i, miniMapWidth, miniMapHeight, &x, &y);
		x += leftMargin;
		y += topMargin;
		if (i == selectedPictureId) {
			// 選択中の画像はRGB(255, 255, 0)で表示
			HBRUSH oldBrush2 = (HBRUSH) canvas_MiniMap->selectGdiObject(
					CreateSolidBrush(RGB(255, 255, 0)));
			canvas_MiniMap->drawCircle(x, y, r);
			DeleteObject(canvas_MiniMap->selectGdiObject(oldBrush2));
		} else if ((controlMode == CONTROL_PANEL_MODE_DEFAULT)
				&& (canvas_SerchDetailhotspot(i))) {
			// (2020/02/07LEE) 選択した画像をMinimapに表示
			HBRUSH orangePen = (HBRUSH) canvas_MiniMap->selectGdiObject(
					CreateSolidBrush(RGB(250, 210, 0)));
			canvas_MiniMap->drawCircle(x, y, r);
			DeleteObject(canvas_MiniMap->selectGdiObject(orangePen));
		} else if ((controlMode == CONTROL_PANEL_MODE_HOTSPOT)
				&& (result->isAroundPicture(selectedPictureId, i))) {
			// ホットスポット補正画面で選択中の画像の周囲にある画像はRGB(0, 192, 0)で表示
			HBRUSH oldBrush2 = (HBRUSH) canvas_MiniMap->selectGdiObject(
					CreateSolidBrush(RGB(0, 192, 0)));
			canvas_MiniMap->drawCircle(x, y, r);
			DeleteObject(canvas_MiniMap->selectGdiObject(oldBrush2));
		} else if (linked[i] == true) {
			// 最初の画像と繋がっている画像はRGB(0, 192, 255)で表示
			HBRUSH oldBrush2 = (HBRUSH) canvas_MiniMap->selectGdiObject(
					CreateSolidBrush(RGB(0, 192, 255)));
			canvas_MiniMap->drawCircle(x, y, r);
			DeleteObject(canvas_MiniMap->selectGdiObject(oldBrush2));
		} else if (result->existHotspot(i)) {
			// ホットスポットがある画像はRGB(255, 0, 0)で表示
			HBRUSH oldBrush2 = (HBRUSH) canvas_MiniMap->selectGdiObject(
					CreateSolidBrush(RGB(255, 0, 0)));
			canvas_MiniMap->drawCircle(x, y, r);
			DeleteObject(canvas_MiniMap->selectGdiObject(oldBrush2));
		} else {
			canvas_MiniMap->drawCircle(x, y, r);
		}

#ifdef MINIMAP_SHOW_DIRECTION
		const double angle = result->getGPSCardinalDirection(i);
		const int x2 = x + sin(angle) * r * 2;
		const int y2 = y - cos(angle) * r * 2;
		canvas_MiniMap->drawLine(x, y, x2, y2);
#endif
	}
	DeleteObject(canvas_MiniMap->selectGdiObject(oldBrush));
}

bool MainWindow::canvas_SerchDetailhotspot(int Picnum) {
	std::vector<HotspotIdList> hotspotIdList =
			MainWindow::getInstance()->getHotspotIdList();
	for (std::vector<HotspotIdList>::iterator item = hotspotIdList.begin();
			item != hotspotIdList.end(); item++) {
		if (item->bestpicnum == Picnum) {
			return true;
		}
	}
	return false;
}

