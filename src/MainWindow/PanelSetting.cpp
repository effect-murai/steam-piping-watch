/*
 * PanelSetting.cpp
 *
 *  Created on: 2016/01/28
 *      Author: PC-EFFECT-002
 */

// Windows APIのヘッダをインクルード
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shlobj.h>

#include "app.h"
#include "MainWindow.h"
#include "ResultData.h"
#include "Graphics.h"
#include "PanelInfo.h"
#include "resource.h"
#include "controlPanel.h"
#include "SubWindows/PanelSetting.h"
#include "VectorOp.h"
#include "StringUtils.h"
#include "FileUtils.h"

//------------------------------------------------------------------------------
// External Global Variables
//------------------------------------------------------------------------------

extern ResultData *result;
extern PanelData *panelData;
extern void openReport(TCHAR *pFilePath);

// 暫定
#ifdef UNICODE
typedef std::wstring String;
typedef std::wstringstream StringStream;
#else
typedef std::string String;
typedef std::stringstream StringStream;
#endif
#define isKeyDown(keycode) ((GetKeyState(keycode) & 0x8000) != 0)

//------------------------------------------------------------------------------
// Class Functions
//------------------------------------------------------------------------------

bool MainWindow::canvas_Main_PanelSettingOnClick(void) {
	int id = panelFind(mainX, mainY);
	int prevId = panelData->getSelectedPanelId();
	panelData->setSelectedPanelId(id);
	bool refresh = (prevId != id);

	// コントロールパネルに読み込んだ値を表示する
	if (id != -1) {
		// パネル管理情報の表示更新
		this->updatePanelSettingInfo();
		controlPanel->enableControl();
	} else {
		// ボタンの無効化
		controlPanel->disableControl();
	}
	if (refresh) {
		canvasUpdate();
	}
	return true;
}

bool MainWindow::canvas_Main_PanelSettingOnDblClick(void) {
	return true;
}

bool MainWindow::canvas_Main_PanelSettingUpdate(void) {
#ifdef MAINWINDOW_PANELSETTING_DEBUG
	clock_t tm1 = clock();
#endif

	// 画像種別
	int pictureType = controlPanel->getPictureType();
	int size = this->panelSettingData->getCopyIdSize();
	// 画像表示
	this->canvas_Main->setNumberdrawPictureAll(pictureType, size);

	// ホットスポットの半径
	const int radius = (getViewRatio() < 1 ? 2 : getViewRatio() * 2);

	// 熱画像のときのみホットスポットを表示する
	if ((pictureType == ResultData::INFRARED_IMAGE)
			|| (pictureType == ResultData::PANEL_IMAGE)) {
		//　(2017/5/31YM)輪郭画像を追加
		this->canvas_Main->drawHotspotAll(radius);
	}

#ifdef MAINWINDOW_PANELSETTING_DEBUG
	clock_t tm2 = clock();
#endif

	// 全パネルを描画
	this->canvas_Main->drawPanelSetting();

	int mode = this->panelSettingData->getMode();
	size = this->panelSettingData->getCopyIdSize();
	if ((size != 0)
			&& (mode == PanelSettingData::PANEL_SETTING_MODE_COPY
					|| mode == PanelSettingData::PANEL_SETTING_MODE_MOVE)) {
		int data[size];
		for (int i = 0; i < size; i++) {
			data[i] = this->panelSettingData->getCopyId(i);
		}
		this->canvas_Main->drawMultiSelectedPanels(data, size);
	}

#ifdef MAINWINDOW_PANELSETTING_DEBUG
	clock_t tm3 = clock();
	wprintf(TEXT("canvas_Main_PanelSettingUpdate:%d/%d ms\n"), tm3 - tm2, tm3 - tm1);
#endif
	return true;
}

LRESULT MainWindow::canvas_Main_PanelSettingProc(HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam) {
	if (panelData == NULL) {
		return 1;
	}

	HDC hdc;
	PAINTSTRUCT ps;
	int x, y;
	POINT pos = { mainX, mainY };

	this->canvas_Main->clipPos(pos);
	x = pos.x;
	y = pos.y;

	int mode = this->panelSettingData->getMode();

	switch (uMsg) {
	case WM_LBUTTONDOWN:
		if (mode == PanelSettingData::PANEL_SETTING_MODE_2SET
				|| mode == PanelSettingData::PANEL_SETTING_MODE_4SET) {
			if (panelFind(x, y) == -1) {
				if (mode
						== PanelSettingData::PANEL_SETTING_MODE_2SET&& blMouse == FALSE) {
					blMouse = TRUE;
					this->panelSettingData->setRectLeft(x);
					this->panelSettingData->setRectTop(y);
					SetCapture(hWnd);
				} else if (mode == PanelSettingData::PANEL_SETTING_MODE_4SET) {
					int count = this->panelSettingData->getPointCount() + 1;
					if (count >= 4) {
						if (createPanel4Points()) {
							canvasUpdate();
						}
						this->panelSettingData->clearPoints();
					} else {
						this->panelSettingData->setPointCount(count);
					}
				}
			}
		} else if (mode == PanelSettingData::PANEL_SETTING_MODE_COPY
				&& blMouse == false) {
			blMouse = TRUE;
			this->panelSettingData->setRectLeft(x);
			this->panelSettingData->setRectTop(y);
			this->panelSettingData->setRectRight(x);
			this->panelSettingData->setRectBottom(y);
			SetCapture(hWnd);
		} else if (mode == PanelSettingData::PANEL_SETTING_MODE_MOVE
				&& blMouse == false) {
			blMouse = TRUE;
			this->panelSettingData->setRectLeft(x);
			this->panelSettingData->setRectTop(y);
			this->panelSettingData->setRectRight(x);
			this->panelSettingData->setRectBottom(y);
			SetCapture(hWnd);
		}
		break;

	case WM_RBUTTONDOWN:
		prevMainPos.x = mainX;
		prevMainPos.y = mainY;
		this->panelSettingData->clearPointCount();
		break;

	case WM_LBUTTONUP:
		canvas_Main_PanelSettingOnLButtonUp(x, y);
		break;

	case WM_MOUSEMOVE:
		if (blMouse != true) {
			showHotspotDetail();
			if (isKeyDown(VK_RBUTTON)) {
				scroll();
			}
		}

		if (mode == PanelSettingData::PANEL_SETTING_MODE_2SET
				&& blMouse == true) {
			this->panelSettingData->setRectRight(x);
			this->panelSettingData->setRectBottom(y);
			this->panelSettingData->setPoint(0,
					this->panelSettingData->getRectLeft(),
					this->panelSettingData->getRectTop());
			this->panelSettingData->setPoint(1,
					this->panelSettingData->getRectLeft(),
					this->panelSettingData->getRectBottom());
			this->panelSettingData->setPoint(2,
					this->panelSettingData->getRectRight(),
					this->panelSettingData->getRectBottom());
			this->panelSettingData->setPoint(3,
					this->panelSettingData->getRectRight(),
					this->panelSettingData->getRectTop());
			this->panelSettingData->setPointCount(3);
		} else if (mode == PanelSettingData::PANEL_SETTING_MODE_4SET) {
			this->panelSettingData->setPoint(
					this->panelSettingData->getPointCount(), x, y);
		} else if (mode == PanelSettingData::PANEL_SETTING_MODE_COPY
				&& blMouse == true) {
			this->panelSettingData->setRectRight(x);
			this->panelSettingData->setRectBottom(y);
			this->panelSettingData->setPoint(0,
					this->panelSettingData->getRectLeft(),
					this->panelSettingData->getRectTop());
			this->panelSettingData->setPoint(1,
					this->panelSettingData->getRectLeft(),
					this->panelSettingData->getRectBottom());
			this->panelSettingData->setPoint(2,
					this->panelSettingData->getRectRight(),
					this->panelSettingData->getRectBottom());
			this->panelSettingData->setPoint(3,
					this->panelSettingData->getRectRight(),
					this->panelSettingData->getRectTop());
			this->panelSettingData->setPointCount(3);
		} else if (mode == PanelSettingData::PANEL_SETTING_MODE_MOVE
				&& blMouse == true) {
			if (this->panelSettingData->getCopyIdSize() == 0) {
				this->panelSettingData->setRectRight(x);
				this->panelSettingData->setRectBottom(y);
				this->panelSettingData->setPoint(0,
						this->panelSettingData->getRectLeft(),
						this->panelSettingData->getRectTop());
				this->panelSettingData->setPoint(1,
						this->panelSettingData->getRectLeft(),
						this->panelSettingData->getRectBottom());
				this->panelSettingData->setPoint(2,
						this->panelSettingData->getRectRight(),
						this->panelSettingData->getRectBottom());
				this->panelSettingData->setPoint(3,
						this->panelSettingData->getRectRight(),
						this->panelSettingData->getRectTop());
				this->panelSettingData->setPointCount(3);
			} else {
				this->panelSettingData->setRectRight(x);
				this->panelSettingData->setRectBottom(y);
				this->panelSettingData->setPoint(0,
						this->panelSettingData->getRectLeft(),
						this->panelSettingData->getRectTop());
				this->panelSettingData->setPoint(1,
						this->panelSettingData->getRectRight(),
						this->panelSettingData->getRectBottom());
				this->panelSettingData->setPointCount(1);
			}
		}
		canvas_Main->refresh();
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		canvas_Main->transfer(hdc);
		if (this->panelSettingData->getPointCount() > 0) {
			HPEN oldPen = (HPEN) SelectObject(hdc,
					CreatePen(PS_DASH, -1, RGB(255, 255, 255)));
			HBRUSH oldBrush = (HBRUSH) SelectObject(hdc,
					GetStockObject(NULL_BRUSH));
			Polygon(hdc, this->panelSettingData->getPoint(0),
					this->panelSettingData->getPointCount() + 1);
			SelectObject(hdc, oldBrush);
			DeleteObject(SelectObject(hdc, oldPen));
		}
		EndPaint(hWnd, &ps);
		break;

	default:
		break;
	}

	return 0;
}

void MainWindow::canvas_Main_PanelSettingOnLButtonUp(int x, int y) {
	if (blMouse == true) {
		int mode = this->panelSettingData->getMode();
		if (mode == PanelSettingData::PANEL_SETTING_MODE_2SET) {
			this->panelSettingData->setRectRight(x);
			this->panelSettingData->setRectBottom(y);
			createPanel2Points();

			blMouse = FALSE;
			this->panelSettingData->clearPointCount();
			ReleaseCapture();
			// キャンバスの再描画
			canvas_MainUpdate();
			canvas_Main->update();
		} else if (mode == PanelSettingData::PANEL_SETTING_MODE_COPY) {
			blMouse = FALSE;
			panelFind();
			ReleaseCapture();
			this->panelSettingData->clearPointCount();
			canvasUpdate();
		} else if (mode == PanelSettingData::PANEL_SETTING_MODE_MOVE) {
			blMouse = FALSE;
			if (this->panelSettingData->getCopyIdSize() == 0) {
				panelFind();
			} else {
				this->panelSettingData->setRectRight(x);
				this->panelSettingData->setRectBottom(y);
				if (this->panelPosCheck()) {
					panelMove();
				}
			}

			ReleaseCapture();
			this->panelSettingData->clearPointCount();
			canvasUpdate();
		}
	}
}

/** 4点化 */
int MainWindow::panelFind(int mainX, int mainY) {
	double realX, realY;
	this->pixelToGPSPos(mainX, mainY, &realX, &realY);
	return panelData->findPanel(realX, realY);
}

bool MainWindow::panelFind() {
	int mainX = this->panelSettingData->getRectLeft();
	int mainY = this->panelSettingData->getRectTop();
	int mainX1 = this->panelSettingData->getRectRight();
	int mainY1 = this->panelSettingData->getRectBottom();
	POINT mousePos[4];
	this->panelSettingData->ClearCopyIdSize();
	mousePos[0].x = mainX;
	mousePos[0].y = mainY;
	mousePos[1].x = mainX;
	mousePos[1].y = mainY1;
	mousePos[2].x = mainX1;
	mousePos[2].y = mainY1;
	mousePos[3].x = mainX1;
	mousePos[3].y = mainY;
	for (int i = 0; i < panelData->getPanelNameCountMax(); i++) {
		POINT pt[4];
		int x, y;
		Vector2D pt1 = panelData->getPoint(i, 0);
		gpsPosToPixel(pt1.x, pt1.y, &x, &y);
		pt[0].x = x;
		pt[0].y = y;
		Vector2D pt2 = panelData->getPoint(i, 1);
		gpsPosToPixel(pt2.x, pt2.y, &x, &y);
		pt[1].x = x;
		pt[1].y = y;
		Vector2D pt3 = panelData->getPoint(i, 2);
		gpsPosToPixel(pt3.x, pt3.y, &x, &y);
		pt[2].x = x;
		pt[2].y = y;
		Vector2D pt4 = panelData->getPoint(i, 3);
		gpsPosToPixel(pt4.x, pt4.y, &x, &y);
		pt[3].x = x;
		pt[3].y = y;
		if (isPointInPolygon(pt[0], 4, mousePos)
				&& isPointInPolygon(pt[1], 4, mousePos)
				&& isPointInPolygon(pt[2], 4, mousePos)
				&& isPointInPolygon(pt[3], 4, mousePos)) {
			this->panelSettingData->addCopyId(i);
		}
	}

	if (this->panelSettingData->getCopyIdSize() > 0) {
		return true;
	} else {
		return false;
	}
}

bool MainWindow::panelMove() {
	int mainX = this->panelSettingData->getRectLeft();
	int mainY = this->panelSettingData->getRectTop();
	int mainX1 = this->panelSettingData->getRectRight();
	int mainY1 = this->panelSettingData->getRectBottom();
	double realMainX, realMainY, realMainX1, realMainY1;
	MainWindow::pixelToGPSPos(mainX, mainY, &realMainX, &realMainY);
	MainWindow::pixelToGPSPos(mainX1, mainY1, &realMainX1, &realMainY1);
	double realMoveX = realMainX1 - realMainX;
	double realMoveY = realMainY1 - realMainY;
	int size = this->panelSettingData->getCopyIdSize();
	for (int i = 0; i < size; i++) {
		int id = this->panelSettingData->getCopyId(i);
		panelData->movePanel(id, realMoveX, realMoveY);
		// ホットスポット数の更新(内部データ)
		this->updateHotspotCount(id);
	}
	// パネル管理情報の表示更新
	this->updatePanelSettingInfo();
	this->panelSettingData->ClearCopyIdSize();
	return true;
}

/**
 * パネル管理領域をコピーする
 */
void MainWindow::PanelSettingPanelCopy() {
	int size = this->panelSettingData->getCopyIdSize();

	// 4点の実世界座標をキャンバス座標に変換
	POINT pt[4];
	Vector2D realPt[4];
	double minX = 100000;
	double minY = 0;
	for (int j = 0; j < size; j++) {
		for (int i = 0; i < 4; i++) {
			realPt[i] = panelData->getPoint(
					this->panelSettingData->getCopyId(j), i);
			if (minX > realPt[i].x) {
				minX = realPt[i].x;
			}
			if (minY < realPt[i].y) {
				minY = realPt[i].y;
			}
		}
	}

	double realMainX;
	double realMainY;
	pixelToGPSPos(mainX, mainY, &realMainX, &realMainY);
	int panelId = PanelData::NOT_SELECTED;
	for (int j = 0; j < size; j++) {
		int x, y;
		int copyId = this->panelSettingData->getCopyId(j);
		for (int i = 0; i < 4; i++) {
			realPt[i] = panelData->getPoint(copyId, i);
			realPt[i].x = realMainX + realPt[i].x - minX;
			realPt[i].y = realMainY + realPt[i].y - minY;
		}
		for (int i = 0; i < 4; i++) {
			gpsPosToPixel(realPt[i].x, realPt[i].y, &x, &y);
			pt[i].x = x;
			pt[i].y = y;
		}
		canvas_Main->drawPolygon(pt, 4);
		panelId = panelData->panelSettingAdd(realPt);

		// 内部分割数をコピー
		SIZE splitCount;
		panelData->getInternalSplitCount(copyId, &splitCount);
		panelData->setInternalSplitCount(panelId, splitCount.cx, splitCount.cy);

		// ホットスポット数の更新(内部データ)
		this->updateHotspotCount(panelId);
		this->panelSettingData->setCopyId(j, -1);
	}
	// 最終パネルを選択
	panelData->setSelectedPanelId(panelId);
	// パネル管理情報の表示更新
	this->updatePanelSettingInfo();
	this->panelSettingData->ClearCopyIdSize();
	panelData->setSelectedPanelId(PanelData::NOT_SELECTED);
}

/**
 * パネル管理領域を削除する
 */
void MainWindow::PanelSettingPanelDelete() {
	int mode = this->panelSettingData->getMode();
	int size = this->panelSettingData->getCopyIdSize();
	if ((mode == PanelSettingData::PANEL_SETTING_MODE_COPY
			|| mode == PanelSettingData::PANEL_SETTING_MODE_MOVE) && 0 < size) {
		for (int j = 0; j < size; j++) {
			panelData->panelSettingDelete(this->panelSettingData->getCopyId(j));
			for (int i = j + 1; i < size; i++) {
				int data = this->panelSettingData->getCopyId(i);
				data--;
				this->panelSettingData->setCopyId(i, data);
			}
			this->panelSettingData->setCopyId(j, -1);
		}
		this->panelSettingData->ClearCopyIdSize();
		panelData->setSelectedPanelId(-1);
	}
}

bool MainWindow::createPanel4Points(void) {
	// 作成判定
	bool bRet = this->squareJudgment(*this->panelSettingData->getPoint(0),
			*this->panelSettingData->getPoint(1),
			*this->panelSettingData->getPoint(2),
			*this->panelSettingData->getPoint(3));
	if (bRet != true) {
		// 作成できなかった場合は作成失敗を返す
		return false;
	}

	// ダイアログ表示
	int ret = PanelSettingDialog::create(getHandle());
	if (ret == IDOK) {
		// 分割処理
		int start1, start2;	// rows, columns; // 分割数　行、列 // (2019/09/26LEE)　注釈処理。
		TCHAR *line1 = this->panelSettingData->getLine1();
		start1 = this->panelSettingData->getStart1();
		start2 = this->panelSettingData->getStart2();

		POINT pt[4];
		for (int i = 0; i < 4; i++) {
			pt[i] = *this->panelSettingData->getPoint(i);
		}

		int panelId = PanelData::NOT_SELECTED;
		//　ブロックごとの四角形を登録する

		int blockCount = start2 - start1 + 1;
		for (int i = 0; i < blockCount; i++) {
			POINT p1, p2, p3, p4;
			p1.x = pt[0].x + (pt[3].x - pt[0].x) * i / blockCount;
			p1.y = pt[0].y + (pt[3].y - pt[0].y) * i / blockCount;
			p2.x = pt[1].x + (pt[2].x - pt[1].x) * i / blockCount;
			p2.y = pt[1].y + (pt[2].y - pt[1].y) * i / blockCount;
			p3.x = pt[0].x + (pt[3].x - pt[0].x) * (i + 1) / blockCount;
			p3.y = pt[0].y + (pt[3].y - pt[0].y) * (i + 1) / blockCount;
			p4.x = pt[1].x + (pt[2].x - pt[1].x) * (i + 1) / blockCount;
			p4.y = pt[1].y + (pt[2].y - pt[1].y) * (i + 1) / blockCount;
			panelId = this->squareAdd(p1, p2, p4, p3);

			StringStream panelName;
			panelName << line1 << TEXT("-") << i + start1;
			panelData->setPanelName(panelId, (LPTSTR) panelName.str().data());

			// ホットスポット数の更新(内部データ)
			this->updateHotspotCount(panelId);
		}
		// パネル管理情報の表示更新
		this->updatePanelSettingInfo();
	}
	return true;
}

bool MainWindow::createPanel2Points(void) {
	// @todo 仮
	double x1, y1, x2, y2;
	pixelToGPSPos(this->panelSettingData->getRectLeft(),
			this->panelSettingData->getRectTop(), &x1, &y1);
	pixelToGPSPos(this->panelSettingData->getRectRight(),
			this->panelSettingData->getRectBottom(), &x2, &y2);

	if ((x1 == x2) || (y1 == y2)) {
		return false;
	}

	if (x2 < x1) {
		std::swap(x1, x2);
	}
	if (y2 < y1) {
		std::swap(y1, y2);
	}
	int ret = PanelSettingDialog::create(getHandle());
	if (ret == IDOK) {
		TCHAR *line1 = this->panelSettingData->getLine1();
		int start1 = this->panelSettingData->getStart1();
		int start2 = this->panelSettingData->getStart2();
		int split = start2 - start1 + 1;
		double splitValue = (x2 - x1) / split;
		int panelId = PanelData::NOT_SELECTED;

		for (int i = 0; i < split; i++) {
			this->panelSettingData->setRectLeft(x1 + splitValue * i);
			this->panelSettingData->setRectTop(y1);
			this->panelSettingData->setRectRight(x1 + splitValue * (i + 1));
			this->panelSettingData->setRectBottom(y2);

			Vector2D square[4];
			square[0].x = x1 + splitValue * i;
			square[0].y = y1;
			square[1].x = x1 + splitValue * i;
			square[1].y = y2;
			square[2].x = x1 + splitValue * (i + 1);
			square[2].y = y2;
			square[3].x = x1 + splitValue * (i + 1);
			square[3].y = y1;

			panelId = panelData->panelSettingAdd(square);

			StringStream panelName;
			panelName << line1 << TEXT("-") << i + start1;
			panelData->setPanelName(panelId, (LPTSTR) panelName.str().data());

			// ホットスポット数の更新(内部データ)
			this->updateHotspotCount(panelId);
		}

		// パネル管理情報の表示更新
		this->updatePanelSettingInfo();

		// ボタンの有効化
		controlPanel->enableControl();
		controlPanel->setFocus();
	}
	return true;
}

bool MainWindow::mouseCursorLocationCheck(RECT *rect) {
	if ((mainX > rect->left && mainX < rect->right)
			&& (mainY > rect->top && mainY < rect->bottom)) {
		return true;
	} else {
		return false;
	}
}

bool MainWindow::panelPosCheck() {
	bool ret = false;

	// 移動元座標(最大値)の取得(GPS座標)
	int size = this->panelSettingData->getCopyIdSize();
	Vector2D realPt[4];
	double minRealX = 100000;
	double maxRealX = 0;
	double minRealY = 100000;
	double maxRealY = 0;
	for (int j = 0; j < size; j++) {
		for (int i = 0; i < 4; i++) {
			realPt[i] = panelData->getPoint(
					this->panelSettingData->getCopyId(j), i);
			if (minRealX > realPt[i].x) {
				minRealX = realPt[i].x;
			}
			if (maxRealX < realPt[i].x) {
				maxRealX = realPt[i].x;
			}
			if (minRealY > realPt[i].y) {
				minRealY = realPt[i].y;
			}
			if (maxRealY < realPt[i].y) {
				maxRealY = realPt[i].y;
			}
		}
	}
	// GPS座標をキャンバス座標に変換
	int posLeft;
	int posRight;
	int posTop;
	int posBottom;
	this->gpsPosToPixel(minRealX, minRealY, &posLeft, &posBottom);
	this->gpsPosToPixel(maxRealX, maxRealY, &posRight, &posTop);

	// 移動サイズの取得(キャンバス座標)
	int mode = this->panelSettingData->getMode();
	int moveX = 0;
	int moveY = 0;
	switch (mode) {
	case PanelSettingData::PANEL_SETTING_MODE_COPY:
		moveX = this->mainX - posLeft;
		moveY = this->mainY - posTop;
		break;

	case PanelSettingData::PANEL_SETTING_MODE_MOVE:
		moveX = this->panelSettingData->getRectRight()
				- this->panelSettingData->getRectLeft();
		moveY = this->panelSettingData->getRectBottom()
				- this->panelSettingData->getRectTop();
		break;

	default:
		break;
	}

	// 移動先座標の計算(キャンバス座標)
	posLeft += moveX;
	posRight += moveX;
	posTop += moveY;
	posBottom += moveY;

	// 移動先がキャンパス内か確認
	RECT rect;
	this->canvas_Main->getClientRect(&rect);
	if ((posLeft > rect.left) && (posRight < rect.right) && (posTop > rect.top)
			&& (posBottom < rect.bottom)) {
		ret = true;
	}

	return ret;
}

/**
 * ホットスポット数一覧を保存する
 */
void MainWindow::saveHotspotNumInfo() {
	// パネル情報がない場合
	if (panelData == NULL) {
		return;
	}

	// ダイアログ表示
	TCHAR *fileName;
	fileName = this->showSaveCSVXLSMFileDialog();
	if (fileName != NULL) {
		// 拡張子をチェックする
		TCHAR *extName = getExt(fileName);
		if (lstrcmp(extName, TEXT("csv")) == 0) {
			// CSV形式でデータ保存
			bool ret = panelData->saveHotspotNumInfo(fileName);
			if (ret != true) {
				// エラー
				this->showMessageBox(IDS_ERR_SAVE_PANEL_INFO, IDS_SET_NO_TITLE,
				MB_OK);
			}
		} else if (lstrcmp(extName, TEXT("xlsm")) == 0) {
			// XLSM形式でデータ保存
			TCHAR *baseName = toBaseName(clone(fileName));
			int baseNameLen = lstrlen(baseName);
			TCHAR *hotspotFileName = new TCHAR[L_tmpnam + baseNameLen + 4];
			TCHAR *hotspotFileNameXlsm = clone(fileName);
			makeWholePictureReport(hotspotFileNameXlsm);

			// 一時ファイル名を生成する
			lstrcpy(hotspotFileName, baseName);
			_wtmpnam(&hotspotFileName[baseNameLen]);
			lstrcpy(&hotspotFileName[lstrlen(hotspotFileName)], TEXT("txt"));

			// ホットスポットファイルを作成する
			bool ret = panelData->saveHotspotNumInfo(hotspotFileName);
			if (ret != true) {
				// エラー
				this->showMessageBox(IDS_ERR_SAVE_PANEL_INFO, IDS_SET_NO_TITLE,
				MB_OK);
			}

			// 俯瞰画像を保存する
			TCHAR *fileName2 = new TCHAR[MAX_PATH];
			TCHAR *path = result->getCachePath();
			stprintf(fileName2, TEXT("%s\\infrared2.png"), path);
			saveWholePictureWithPanelData(fileName2,
					ResultData::INFRARED_IMAGE);

			stprintf(fileName2, TEXT("%s\\visible2.png"), path);
			saveWholePictureWithPanelData(fileName2,
					ResultData::VISIBLE_LIGHT_IMAGE);

			stprintf(fileName2, TEXT("%s\\filePath2.txt"), baseName);
			saveFilePath2List(fileName2, hotspotFileName);

			openReport(hotspotFileNameXlsm);

			delete baseName;
			delete hotspotFileNameXlsm;
			delete fileName2;
			delete hotspotFileName;

		} else {
			// エラー
			this->showMessageBox(IDS_EXT_IS_NOT_XLSM, IDS_SET_NO_TITLE,
			MB_OK | MB_ICONERROR);
		}
		free(fileName);
	}
}

bool MainWindow::makeWholePictureReport(TCHAR *path) {
	FILE *file = fileOpen(path, TEXT("wb"));
	if (file == NULL) {
		return false;
	}

	// リソースを開く
	HRSRC res;
	if (getActivationResult()) {
		res = Resource::getObject(IDR_WHOLE_PIC_WITH_PANEL, (int) RT_RCDATA);
	} else {
		res = Resource::getObject(IDR_WHOLE_PIC_WITH_PANEL_TRIAL,
				(int) RT_RCDATA);
	}
	HGLOBAL mem = LoadResource(GetModuleHandle(NULL), res);
	size_t size = SizeofResource(GetModuleHandle(NULL), res);
	char *data = (char*) LockResource(mem);

	// ファイルに書き込む
	for (size_t i = 0; i < size; i++) {
		fputc(data[i], file);
	}

	// 後片付け
	FreeResource(mem);

	fclose(file);
	return true;
}
/**
 * 管理番号を保存する
 */
void MainWindow::savePanelInfo() {
	// パネル情報がない場合
	if (panelData == NULL) {
		return;
	}

	// ダイアログ表示
	TCHAR *fileName;
	fileName = this->showSaveCSVFileDialog();
	if (fileName != NULL) {
		// データ保存
		bool ret = panelData->savePanelInfo(fileName);
		if (ret != true) {
			// エラー
			this->showMessageBox(IDS_ERR_SAVE_PANEL_INFO, IDS_SET_NO_TITLE,
			MB_OK);
		}
		free(fileName);
	}
}

/**
 * 管理番号を読み込む
 */
void MainWindow::loadPanelInfo() {
	// ダイアログ表示
	TCHAR *fileName;
	fileName = this->showLoadCSVFileDialog();
	if (fileName != NULL) {
		if (panelData != NULL) {
			// パネル情報を削除
			delete panelData;
		}

		// 新規生成
		panelData = new PanelData();

		// データ読み込み
		bool ret = panelData->loadPanelInfo(fileName);
		if (ret != true) {
			// エラー
			this->showMessageBox(IDS_ERR_LOAD_PANEL_INFO, IDS_SET_NO_TITLE,
			MB_OK);
		}
		free(fileName);
	}
}

/**
 * パネル管理情報の表示更新
 */
void MainWindow::updatePanelSettingInfo() {
	int panelId = panelData->getSelectedPanelId();

	if (panelId != PanelData::NOT_SELECTED) {
		// パネル名
		controlPanel->setEditName(panelData->getPanelName(panelId));

		// ホットスポット数
		StringStream hotspotCountText;
		const int COUNT = 4;
		Vector2D panel[COUNT];
		for (int j = 0; j < COUNT; j++) {
			panel[j] = panelData->getPoint(panelId, j);
		}

		int hotspotCount = result->getHotspotCountInArea(panel, COUNT);
		hotspotCountText << hotspotCount;
		controlPanel->setHotspotCount((LPTSTR) hotspotCountText.str().data());
	}
}

/**
 * ホットスポット数の更新(内部データ)
 */
void MainWindow::updateHotspotCount(int panelId) {
	const int COUNT = 4;
	Vector2D panel[COUNT];
	for (int j = 0; j < COUNT; j++) {
		panel[j] = panelData->getPoint(panelId, j);
	}

	int hotspotCount = result->getHotspotCountInArea(panel, COUNT);
	panelData->setHotspotCount(panelId, hotspotCount);
}
