/*
 * AreaSetting.cpp
 *
 *  Created on: 2016/03/01
 *      Author: PC-EFFECT-011
 */

#include "MainWindow.h"
#include "Graphics.h"
#include "ResultData.h"
#include "AreaSettingData.h"

//------------------------------------------------------------------------------
// External Global Variables
//------------------------------------------------------------------------------

extern ResultData *result;

int checkcondition;		//(2020/01/08LEE) モードのデータを保存して区分
int prevcondition; //(2020/01/08LEE) モードのデータを保存して区分
int modedata[1000] = { };	//(2020/01/08LEE) モードのデータを保存して区分
int modecount = 0;		//(2020/01/08LEE) モードのデータを保存して区分
//------------------------------------------------------------------------------
// MainWindow Class
//------------------------------------------------------------------------------

bool MainWindow::canvas_Main_AreaSettingOnClick() {
	Vector2D pos;
	POINT mousePos = { this->mainX, this->mainY };
	Vector2D PosPrev;
	Vector2D PosNow;
	BOOL isEnable = true;

	// 既に処理が完了している場合は、処理なし。
	if (this->isAreaComplete) {
		return true;
	}

	// (2019/09/25LEE)　直線と対角線をために追加。
	if ((GetKeyState(VK_SHIFT) & 0x8000)) {
		checkcondition = 1;	// 対角線モード
	} else {
		checkcondition = 2;	// 直線モード
	}

	// ペンを変更
	HPEN whitePen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
	HPEN oldPen = (HPEN) canvas_Main->selectGdiObject(whitePen);

	// 座標のクリップ
	this->canvas_Main->clipPos(mousePos);

	pixelToGPSPos(mousePos.x, mousePos.y, &pos.x, &pos.y);

	// 座標修正
	if (this->HotSpotAreas.size() < 1) {
		// 1回目
		PosNow.x = pos.x;
		PosNow.y = pos.y;
	} else {
		HotSpotArea::reverse_iterator it = this->HotSpotAreas.rbegin();
		// 2回目以降
		PosPrev = (*it++);
		if (abs(PosPrev.x - pos.x) > abs(PosPrev.y - pos.y)) {
			PosNow.x = pos.x;
			//PosNow.y = PosPrev.y; //(2019/09/19LEE) 変更
			if (checkcondition == 1)
				PosNow.y = pos.y;
			else if (checkcondition == 2)
				PosNow.y = PosPrev.y;
			else
				PosNow.y = pos.y;
		} else {
			//PosNow.x = PosPrev.x; //(2019/09/19LEE) 変更
			PosNow.y = pos.y;
			if (checkcondition == 1) {
				PosNow.x = pos.x;
			} else if (checkcondition == 2) {
				PosNow.x = PosPrev.x;
			} else {
				PosNow.x = pos.x;
			}
		}

		// 3回目以降、逆行する位置は無効とする
		if (checkcondition == 2)  // (2019/09/25LEE)　直線と対角線を区分するために追加。
				{
			if (this->HotSpotAreas.size() >= 2) {
				// (2020/01/07LEE) 
				if (modedata[modecount - 1] == 1) {
					if (abs(PosPrev.x - pos.x) > abs(PosPrev.y - pos.y)) {
						PosNow.x = pos.x;
						PosNow.y = PosPrev.y;
					} else {
						PosNow.y = pos.y;
						PosNow.x = PosPrev.x;
					}
				}

				else {
					Vector2D PosPrePrev;
					double vectorX;
					double vectorY;
					PosPrePrev = (*it);
					vectorX = PosPrev.x - PosPrePrev.x;
					vectorY = PosPrev.y - PosPrePrev.y;

					if ((vectorX > 0) && ((PosNow.x - PosPrev.x) < 0)) {
						isEnable = false;
					}
					if ((vectorX < 0) && ((PosNow.x - PosPrev.x) > 0)) {
						isEnable = false;
					}
					if ((vectorY > 0) && ((PosNow.y - PosPrev.y) < 0)) {
						isEnable = false;
					}
					if ((vectorY < 0) && ((PosNow.y - PosPrev.y) > 0)) {
						isEnable = false;
					}
				}
			}
		}

		//(2020/01/06LEE) 
		if (isEnable) {
			// 点を結ぶ
			POINT canvasPos1, canvasPos2;
			gpsPosToPixel(&PosPrev, &canvasPos1);
			gpsPosToPixel(&PosNow, &canvasPos2);
			canvas_Main->drawLine(canvasPos1.x, canvasPos1.y, canvasPos2.x,
					canvasPos2.y);
			//(2020/01/08LEE) モードのデータを保存して区分
			modedata[modecount] = checkcondition;
			modecount++;
		}
	}

	if (isEnable) {
		// 点を表示する
		const int radius = 2;
		POINT canvasPos;
		gpsPosToPixel(&PosNow, &canvasPos);
		canvas_Main->drawCircle(canvasPos.x, canvasPos.y, radius);
		// 位置保存
		this->HotSpotAreas.push_back(PosNow);
	}

	// ペンを元に戻す
	canvas_Main->selectGdiObject(oldPen);
	DeleteObject(whitePen);

	// バッファ更新
	this->immediatelyRedraw();

	this->isAreaMove = true;

	return true;
}

bool MainWindow::canvas_Main_AreaSettingOnDblClick() {
	// 既に処理が完了している場合は、処理なし。
	if (this->isAreaComplete) {
		return true;
	}

	modecount = 0; //(2020/01/08LEE) モードのデータを保存して区分
	memset(modedata, 0, 1000 * sizeof(int)); //(2020/01/08LEE) モードのデータを保存して区分

	// 最終確定
	this->AreaSettingComplete();

	// ボタン有効化
	this->controlPanel->enableControl();

	return true;
}

bool MainWindow::canvas_Main_AreaSettingUpdate() {
	// フォルダを指定する前は何もしない
	if (result == NULL) {
		return false;
	}

	// データがない場合も何もしない
	if (result->getDataCount() == 0) {
		return false;
	}

	// 画像種別
	const int pictureType = ResultData::INFRARED_IMAGE;
	this->canvas_Main->drawPictureAll(pictureType);

	// ホットスポットを表示
	const int radius = 2;
	this->canvas_Main->drawHotspotArea(radius);

	// 解析範囲を表示
	this->canvas_Main->drawAreaSetting();

	return true;
}

LRESULT MainWindow::canvas_Main_AreaSettingProc(HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam) {
	HDC hDC;
	PAINTSTRUCT ps;

	switch (uMsg) {
	case WM_MOUSEMOVE:
		if ((this->HotSpotAreas.size() > 0) && this->isAreaMove) {
			this->canvas_Main->refresh();
		}
		break;

	case WM_PAINT:
		if ((this->HotSpotAreas.size() > 0) && this->isAreaMove) {
			hDC = BeginPaint(hWnd, &ps);
			this->canvas_Main->transfer(hDC);

			// 軌跡を表示
			POINT pos = { this->mainX, this->mainY };
			POINT posStart;
			POINT posEnd;
			Vector2D realPos = this->HotSpotAreas.back();
			gpsPosToPixel(&realPos, &posStart);
			// 座標のクリップ
			this->canvas_Main->clipPos(pos);
			// 直交座標へ変更
			posEnd.x = pos.x; // (2019/09/19LEE) 変更。
			posEnd.y = pos.y; // (2019/09/19LEE) 変更。

			// ペンを変更
			HPEN whitePen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
			HPEN oldPen = (HPEN) SelectObject(hDC, whitePen);
			// ライン描画
			MoveToEx(hDC, posStart.x, posStart.y, NULL);
			LineTo(hDC, posEnd.x, posEnd.y);
			// ペンを元に戻す
			SelectObject(hDC, oldPen);
			DeleteObject(whitePen);
			EndPaint(hWnd, &ps);
		}
		break;

	case WM_RBUTTONDOWN:
		if (this->isAreaComplete) {
			// 既に処理が完了している場合は、処理なし。
		} else if (this->HotSpotAreas.size() > 0) {
			// キャンセル(最終位置を削除)
			this->AreaSettingCancel();

			modedata[modecount] = 0;
			modecount--;         //(2020/01/08LEE)　モードのデータを保存して区分

			// (2020/01/07LEE) キャンセルをする時、対角線モードと直線モードを一緒に使用するために追加
			if (modedata[modecount] == 1) {
				prevcondition = 1;
			} else {
				prevcondition = 0;
			}
		}
		break;

	default:
		break;
	}
	return 0;
}

/**
 * 解析範囲設定を初期化する。
 */
void MainWindow::AreaSettingInit() {
	this->HotSpotAreas.clear();
	this->isAreaMove = false;
	this->isAreaComplete = false;
	if (this->HotSpotAreaRgn != NULL) {
		DeleteObject(this->HotSpotAreaRgn);
		this->HotSpotAreaRgn = NULL;
	}
}

/**
 * 解析範囲設定を完了する。
 */
void MainWindow::AreaSettingComplete() {
	Vector2D PosStart;		// 始点
	Vector2D PosNext;		// 始点の次
	Vector2D PosPrev;		// 終点の前
	Vector2D PosEnd;		// 終点

	// 始点、始点の次を取得
	HotSpotArea::iterator it = this->HotSpotAreas.begin();
	PosStart = (*it++);
	PosNext = (*it--);
	// 終点、終点一つ前を取得
	HotSpotArea::reverse_iterator rit = this->HotSpotAreas.rbegin();
	PosEnd = (*rit++);
	PosPrev = (*rit--);

	// 終点を修正
	if (PosPrev.x == PosEnd.x) {
		rit->y = PosStart.y;
	} else {
		rit->x = PosStart.x;
	}

	// 終点が重なる場合、始点を削除
	if (((PosStart.x == PosNext.x) && (PosStart.x == rit->x))
			|| ((PosStart.y == PosNext.y) && (PosStart.y == rit->y))) {
		this->HotSpotAreas.erase(it);
	}

	// キャンバス上の座標に変換
	std::vector<POINT> HotSpotAreasOnCanvas;
	HotSpotAreasOnCanvas.clear();
	for (HotSpotArea::iterator item = this->HotSpotAreas.begin();
			item != this->HotSpotAreas.end(); item++) {
		int x, y;
		gpsPosToPixel(item->x, item->y, &x, &y);
		POINT posCanvas = { x, y };
		HotSpotAreasOnCanvas.push_back(posCanvas);
	}

	// リージョン作成(vectorは、整列されている)
	this->HotSpotAreaRgn = CreatePolygonRgn(&HotSpotAreasOnCanvas.front(),
			HotSpotAreasOnCanvas.size(), WINDING);

	if (this->HotSpotAreaRgn == NULL) {
		// 完了としない
	} else {
		// 移動完了
		this->isAreaMove = false;
		// 完了フラグ設定
		this->isAreaComplete = true;
	}

	// キャンバス更新
	this->canvas_MainUpdate();

	// バッファ更新
	this->immediatelyRedraw();
}

/**
 * 解析範囲設定を取り消す。
 */
void MainWindow::AreaSettingCancel() {
	// 終点を削除
	this->HotSpotAreas.pop_back();

	// キャンバス更新
	this->canvas_MainUpdate();

	// バッファ更新
	this->immediatelyRedraw();
}

/**
 * 解析範囲設定を消去する。
 */
void MainWindow::AreaSettingClear() {
	// 解析範囲設定 初期化
	this->AreaSettingInit();

	// コントロール無効化
	this->controlPanel->disableControl();

	// キャンバス更新
	this->canvas_MainUpdate();

	// バッファ更新
	this->immediatelyRedraw();
}
