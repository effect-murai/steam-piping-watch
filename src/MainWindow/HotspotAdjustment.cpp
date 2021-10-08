/*
 * HotspotAdjustment.cpp
 *
 *  Created on: 2016/01/08
 *      Author: PC-EFFECT-012
 */

// 3. ホットスポット修正
#include "StatusPanel.h"
#include "MainWindow.h"
#include "ResultData.h"
#include "HotspotLinker.h"
#include "Graphics.h"
#include <stdio.h>
#include "resource.h"

#include "SubWindows/PictureWindow.h"

#include "Functions.h"
#include "VectorOp.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------

enum {
	PICTURE_POS_NORTH = 0,
	PICTURE_POS_EAST,
	PICTURE_POS_SOUTH,
	PICTURE_POS_WEST,
	PICTURE_POS_CENTER,
	PICTURE_POS_COUNT
};

#define HOTSPOT_CIRCLE_RADIUS 10

//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------

// foreach構文
#define __foreach(item_type, value, list) \
	for (\
		std::vector<item_type>::iterator \
		value = list.begin(); \
		value != list.end(); \
		value++\
	)

#define isKeyDown(keycode) ((GetKeyState(keycode) & 0x8000) != 0)

// @todo 暫定マクロ定義
#define unselectHotspot() (selectHotspot(NOT_SELECTED, NOT_SELECTED))
#define isNotHotspotSelected(number) (\
		(number.pictureNo == NOT_SELECTED) ||\
		(number.pointNo == NOT_SELECTED)\
		)

#define isHotspotEnabled(pic, pt) (\
	(result->isEnabledHotspot((pic), (pt)) && this->getEnabledHotspotShowMode()) ||\
	(result->isHotspotCandidate((pic), (pt)) && this->getHotspotCandidateShowMode()) ||\
	(result->isDisabledHotspot((pic), (pt)) && this->getDisabledHotspotShowMode()) ||\
	(result->isKeypoint((pic), (pt)) && this->getKeypointShowMode())\
	)

//------------------------------------------------------------------------------
// External Global Variables
//------------------------------------------------------------------------------

extern ResultData *result;
extern HotspotLinker *hotspotLinker;
extern MainWindow *mainWindow;		// (2017/10/26YM)mainWindowを

//------------------------------------------------------------------------------
// Inline Functions
//------------------------------------------------------------------------------

inline void affineTransformation2(POINT *pt, int baseX, int baseY, double angle,
		double ratio, int shiftX, int shiftY) {
	int px, py;
	px = pt->x - baseX;
	py = pt->y - baseY;
	pt->x = (px * cos(angle) - py * sin(angle)) * ratio + shiftX;
	pt->y = (px * sin(angle) + py * cos(angle)) * ratio + shiftY;
}

inline void rectToPointArray(int width, int height, POINT *points) {
	points[0].x = 0;
	points[0].y = 0;
	points[1].x = 0;
	points[1].y = height;
	points[2].x = width;
	points[2].y = height;
	points[3].x = width;
	points[3].y = 0;
}

inline void getPicturePos(int directionId, int infraredWidth,
		int infraredHeight, int canvasWidth, int canvasHeight, double direction,
		int *x, int *y, double *ratio) {

	// 各画像の方位から縮尺を計算する
	const int dstWidth = (int) (fabs(infraredWidth * cos(-direction))
			+ fabs(infraredHeight * sin(-direction)) + 0.5);
	const int dstHeight = (int) (fabs(infraredWidth * sin(-direction))
			+ fabs(infraredHeight * cos(-direction)) + 0.5);

	const double ratio1 = (double) infraredWidth / (double) dstWidth;
	const double ratio2 = (double) infraredHeight / (double) dstHeight;
	const double ratio3 = (double) canvasWidth / 3.0 / (double) infraredWidth;

	// @todo 高度による補正
	*ratio = ((ratio1 < ratio2) ? ratio1 : ratio2) * ratio3;

	// 画像を読み込んだBitmapを描画
	if (directionId < 4) {
		*x = canvasWidth / 2
				+ sin(M_PI / 2 * directionId) * (canvasWidth / 3.0);
		*y = canvasHeight / 2
				- cos(M_PI / 2 * directionId) * (canvasWidth / 3.0);
	} else {
		*x = canvasWidth / 2;
		*y = canvasHeight / 2;
	}
}

inline void getPictureRect(int infraredWidth, int infraredHeight, double ratio,
		double direction, int baseX, int baseY, POINT *p) {
	rectToPointArray(infraredWidth, infraredHeight, p);
	for (int i = 0; i < 4; i++) {
		affineTransformation2(&p[i], infraredWidth / 2, infraredHeight / 2,
				direction, ratio, baseX, baseY);
	}
}

inline void getHotspotPos_2(int picNo, int ptNo, int infraredWidth,
		int infraredHeight, double direction, double ratio, int baseX,
		int baseY, int *x, int *y) {
	POINT p;
	result->getHotspot(picNo, ptNo, &p);
	affineTransformation2(&p, infraredWidth / 2, infraredHeight / 2, direction,
			ratio, baseX, baseY);
	*x = p.x;
	*y = p.y;
}

inline void getHotspotPos(int directionId, int infraredWidth,
		int infraredHeight, int canvasWidth, int canvasHeight, double direction,
		int picNo, int ptNo, int *x, int *y) {
	int cx, cy;
	double ratio;
	getPicturePos(directionId, infraredWidth, infraredHeight, canvasWidth,
			canvasHeight, direction, &cx, &cy, &ratio);

	getHotspotPos_2(picNo, ptNo, infraredWidth, infraredHeight, direction,
			ratio, cx, cy, x, y);
}

inline int getAroundPictureId(int id, int directionId) {
	if (directionId < PICTURE_POS_CENTER) {
		return result->getAroundPictureId(id, directionId);
	} else {
		return id;
	}
}

inline double getPictureRatio(int picNo1, int picNo2) {
	const double height1 = result->getHeight(picNo1);
	const double height2 = result->getHeight(picNo2);
	if (height1 * height2 == 0) {
		return 1.0;
	}

	const double size = result->getInfraredWidth();
	const double viewAngle = result->getViewAngle();

	double mpp1 = ResultData::getMeterPerPixel(size, viewAngle, height1);
	double mpp2 = ResultData::getMeterPerPixel(size, viewAngle, height2);

	return mpp2 / mpp1;
}

inline bool isHotspotInPicture(HotspotNumber &hotspot1, HotspotNumber &hotspot2,
		int hotspot3) {
	Vector2D pt, pt1, pt2, pt3;
	Vector2DPolar polar;
	result->getPolarCoordinates(hotspot1.pictureNo, hotspot1.pointNo, &polar);
	polar.arg += result->getDirection(hotspot1.pictureNo)
			+ result->getBaseDirection();
	pt1.x = polar.r * cos(polar.arg);
	pt1.y = polar.r * sin(polar.arg);

	double ratio = getPictureRatio(hotspot1.pictureNo, hotspot2.pictureNo);
	result->getPolarCoordinates(hotspot2.pictureNo, hotspot2.pointNo, &polar);
	polar.arg += result->getDirection(hotspot2.pictureNo)
			+ result->getBaseDirection();
	pt2.x = ratio * polar.r * cos(polar.arg);
	pt2.y = ratio * polar.r * sin(polar.arg);

	result->getPolarCoordinates(hotspot2.pictureNo, hotspot3, &polar);
	polar.arg += result->getDirection(hotspot2.pictureNo)
			+ result->getBaseDirection();
	pt3.x = ratio * polar.r * cos(polar.arg);
	pt3.y = ratio * polar.r * sin(polar.arg);

	pt = pt1 - pt2 + pt3;

	polar.r = sqrt(pt.x * pt.x + pt.y * pt.y);
	polar.arg = atan2(pt.y, pt.x)
			- (result->getDirection(hotspot1.pictureNo)
					+ result->getBaseDirection());

	pt.x = polar.r * cos(polar.arg) + result->getInfraredWidth() / 2;
	pt.y = polar.r * sin(polar.arg) + result->getInfraredHeight() / 2;

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = result->getInfraredWidth();
	rect.bottom = result->getInfraredHeight();
	POINT point;
	point = toPOINT(pt);
	if (PtInRect(&rect, point)) {
		return true;
	}
	return false;
}

inline bool fixHotspotCandidates(HotspotNumber linkBase,
		HotspotNumber linkTarget) {
	int picNo = linkTarget.pictureNo;
	bool changed = false;
	for (int ptNo = 0; ptNo < result->getHotspotCount(picNo); ptNo++) {
		if (result->isHotspotCandidate(picNo, ptNo)) {
			int linkId = result->getHotspotId(picNo, ptNo);
			if (linkId == ResultData::NOT_ASSIGNED) {
				// リンク元側の画像の範囲内の場合のみ処理を実施
				bool ret = isHotspotInPicture(linkBase, linkTarget, ptNo);
				if (ret == true) {
					// リンクがない候補は無効化する
					result->disableHotspot(picNo, ptNo);
					changed = true;
				}
			} else {
				// リンクがある場合は確定させる
				result->enableHotspot(picNo, ptNo);
				changed = true;
			}
		}
	}
	return changed;
}

// 指定したホットスポットにリンクされているホットスポットする
inline void changeDisabledHotspots(int picNo, int ptNo, int type) {
	if (result->isDisabledHotspot(picNo, ptNo)) {
		if (result->isUniqueHotspot(picNo, ptNo)) {
			result->setHotspotType(picNo, ptNo, type);
		} else {
			int linkId = result->getHotspotId(picNo, ptNo);
			HotspotLink link = (*hotspotLinker)[linkId];
			__foreach (HotspotNumber, hotspot, link)
			{
				result->enableHotspot(hotspot->pictureNo, hotspot->pointNo);
			}
		}
	}
}

//------------------------------------------------------------------------------
// MainWindow Class
//------------------------------------------------------------------------------

bool MainWindow::canvas_Main_HotspotAdjustmentOnClick(void) {
	// フォルダを指定する前は何もしない
	if (result == NULL) {
		return false;
	}

	// データがない場合も何もしない
	if (result->getDataCount() == 0) {
		return false;
	}

	// 画面の更新を指示
	this->canvas_Main->refresh();

	// 全非表示モードの場合は何もしない
	if (getHotspotShowMode() == 0) {
		return true;
	}

	// 画像上をクリックしている場合は座標を記録する
	if (this->blMouse == false) {
		// 画像上をクリックしているか確認
		int clickedId = getClickedPictureId(mainX, mainY);
		if (clickedId < 0 || clickedId >= result->getDataCount()) {
			this->blMouse = false;
			return true;
		}

		this->prevMainPos.x = mainX;
		this->prevMainPos.y = mainY;
		this->blMouse = true;
	} else {
		addLink();
		// 画面の更新を指示
		this->canvas_Main->refresh();
	}

	return true;
}

bool MainWindow::canvas_Main_HotspotAdjustmentOnDblClick(void) {
	// コントロールキーを押した場合
	if (isKeyDown(VK_CONTROL)) {
		int id = getClickedPictureId(mainX, mainY);
		if (id < 0 || id >= result->getDataCount()) {
			return true;
		}
		// (2019/11/19LEE) 変更。
		PictureWindow::create(this, ResultData::INFRARED_IMAGE, id);
		canvasUpdate();

		// 選択を解除する
		blMouse = false;
		return true;
	}

	// (2016/12/20YM)隣の画像の拡大処理を追加
	//　シフトキーを押したとき
	if (isKeyDown(VK_SHIFT)) {
		int id = getClickedPictureId(mainX, mainY);
		if (id < 0 || id >= result->getDataCount()) {
			return true;
		}
		if (id != selectedPictureId) {
			selectedPictureId = id;
			canvasUpdate();
		}
		// 選択を解除する
		blMouse = false;
		return true;
	}
	//　↑ここまで追加

	// 全非表示モードの場合は何もしない
	if (getHotspotShowMode() == 0) {
		return true;
	}

	// 選択を解除する
	blMouse = false;

	// ホットスポット以外をクリックした場合
	if (checkHotspot(mainX, mainY) == false) {
		// デバッグ用
		if (isKeyDown(VK_F1) && isKeyDown(VK_SHIFT)) {
			addHotspot(mainX, mainY, HOTSPOT_TYPE_DISABLED);
			return true;
		} else if (isKeyDown(VK_F2) && isKeyDown(VK_SHIFT)) {
			addHotspot(mainX, mainY, HOTSPOT_TYPE_CANDIDATE);
			return true;
		}

		if (deleteLinkLineSegment(mainX, mainY, HOTSPOT_CIRCLE_RADIUS)) {
			canvasUpdate();
			return true;
		}

		if (isKeyDown(VK_MENU)) {
			addHotspot(mainX, mainY, HOTSPOT_TYPE_KEYPOINT);
		} else {
			// ホットスポットを追加する
			addHotspot(mainX, mainY, HOTSPOT_TYPE_ENABLED);
		}
	}

	else {
		HotspotNumber hotspot = this->getHotspotOnAdjustmentMode(mainX, mainY);
		if (result->isDisabledHotspot(hotspot.pictureNo, hotspot.pointNo)) {
			changeDisabledHotspots(hotspot.pictureNo, hotspot.pointNo,
					HOTSPOT_TYPE_ENABLED);
			// ホットスポットを保存する
			result->saveHotspot();
			// 表示更新
			int id = getClickedPictureId(mainX, mainY);	// (2019/12/04LEE) 追加
			HWND window = canvas_Main->getHandle();
			PictureWindow::Windowupdate(window, id); // (2019/12/04LEE) 追加
			canvasUpdate();
		} else {
			// ホットスポットを削除する
			addHotspot(mainX, mainY, HOTSPOT_TYPE_ENABLED);
		}
	}
	return true;
}

bool MainWindow::canvas_Main_HotspotAdjustmentUpdate(void) {
	// フォルダを指定する前は何もしない
	if (result == NULL) {
		return false;
	}

	// データがない場合も何もしない
	if (result->getDataCount() == 0) {
		return false;
	}
	// キャンバスサイズを取得する
	const int canvasWidth = canvas_Main->clientWidth();
	const int canvasHeight = canvas_Main->clientHeight();

	int checkNorth, checkEast, checkSouth, checkWest;
	int checkposition[5] = { 0 };
	// GDI+のGraphicsオブジェクトを作成
	Gdiplus::Graphics *graphics = new Gdiplus::Graphics(
			canvas_Main->getBackBuffer());

	// 熱画像サイズを取得する
	const int infraredWidth = result->getInfraredWidth();
	const int infraredHeight = result->getInfraredHeight();

	// ペンを追加
	HPEN whitePen = CreatePen(PS_SOLID, 4, RGB(255, 255, 255));
	HPEN redPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
	HPEN greenPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
	HPEN bluePen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
	HPEN grayPen = CreatePen(PS_SOLID, 2, RGB(160, 160, 160));
	// (2017/7/31YM)ペンを追加
	HPEN redPen2 = CreatePen(PS_SOLID, 2, RGB(96, 0, 0));

	int pictureId, hotspotId;
	pictureId = selectedHotspotPictureId;
	hotspotId = selectedHotspotPointId;

	// (2020/01/08LEE) 画面を右、左に順番で対応するために追加
	for (int i = 0; i < PICTURE_POS_COUNT; i++) {
		checkposition[i] = getAroundPictureId(getSelectedId(), i);
	}
	checkNorth = checkposition[0];
	checkEast = checkposition[1];
	checkSouth = checkposition[2];
	checkWest = checkposition[3];

	for (int i = 0; i < PICTURE_POS_COUNT; i++) {
		// 画像番号を取得する

		int id = getAroundPictureId(getSelectedId(), i);
		/* */
		// (2020/01/08LEE) 画面を右、左に順番で対応するために追加
		if (i == PICTURE_POS_EAST) {
			if (id == checkNorth || id == checkSouth) {
				id = -1;
			}
		}

		if (i == PICTURE_POS_WEST) {
			if (id == checkNorth || id == checkSouth) {
				id = -1;
			}
		}
		// (2020/01/08LEE) 画面を右、左に順番で対応するために追加
		// その方位に画像がない場合
		if (id < 0) {
			continue;
		}
		// ファイル名
		TCHAR *fileName = result->getInfraredFilePath(id);
		// 撮影方位を取得する
		const double direction = getDirection(id);
		// 画像の表示位置と画像サイズ(倍率)を取得する
		int pictureposion = i;

		if (i == PICTURE_POS_EAST) {
			if (id == checkNorth) {
				pictureposion = 0;
			}
			if (id == checkSouth) {
				pictureposion = 2;
			}
		}

		int cx, cy;
		double ratio;
		getPicturePos(pictureposion, infraredWidth, infraredHeight, canvasWidth,
				canvasHeight, direction, &cx, &cy, &ratio);

		// Bitmapイメージを読み込む 、(2019/11/25LEE) pictureIdを追加。
		int pictureType = 1;
		Gdiplus::Bitmap *img = Graphics::loadBitmap(fileName, pictureType,
				ratio, -direction);

		graphics->DrawImage(img, cx - (int) img->GetWidth() / 2,
				cy - (int) img->GetHeight() / 2);
		// Bitmapオブジェクトを破棄
		delete img;
		// ホットスポットを表示する
		HBRUSH oldBrush = (HBRUSH) canvas_Main->selectGdiObject(
				GetStockObject(NULL_BRUSH));
		// 点の表示位置を計算し、点を表示する
		for (int i = 0; i < result->getHotspotCount(id); i++) {
			if (!isHotspotEnabled(id, i)) {
				// 指定したホットスポットが非表示にされている場合
				continue;
			}
			int x, y;
			getHotspotPos_2(id, i, infraredWidth, infraredHeight, direction,
					ratio, cx, cy, &x, &y);
			HPEN oldPen = (HPEN) canvas_Main->selectGdiObject(whitePen);
			canvas_Main->drawCircle(x, y, HOTSPOT_CIRCLE_RADIUS);
			if (id == pictureId && i == hotspotId) {
				canvas_Main->selectGdiObject(greenPen);
			} else {
				switch (result->getHotspotType(id, i)) {
				case HOTSPOT_TYPE_KEYPOINT:
					canvas_Main->selectGdiObject(bluePen);
					break;

				case HOTSPOT_TYPE_DISABLED:
					canvas_Main->selectGdiObject(grayPen);
					break;

				case HOTSPOT_TYPE_CANDIDATE:
				default:
					// (2017/7/31YM)ホットスポットの温度別に色を変更する処理を追加
					int hsid = result->getHotspotID(id, i);
					if (hsid > -1) {
						float temp = result->getHotspotTemp(hsid);
						float panelavetemp = result->getPanelAveTemp(hsid);
						double prevthre = getThresholdTemp() + panelavetemp;
						if (prevthre < 0) {
							prevthre = 0;
						}
						if (temp >= (prevthre + 20)) {
							canvas_Main->selectGdiObject(redPen2);
						} else {
							canvas_Main->selectGdiObject(redPen);
						}
					} else {
						canvas_Main->selectGdiObject(redPen);
					}
					// (2017/7/31YM)元のコード
					break;
				}
			}
			canvas_Main->drawCircle(x, y, HOTSPOT_CIRCLE_RADIUS);
			canvas_Main->selectGdiObject(oldPen);
		}
		canvas_Main->selectGdiObject(oldBrush);
	}

	int selected = getSelectedId();

	if (selected >= 0) {
		int count = result->getHotspotCount(selected);
		for (int i = 0; i < count; i++) {
			if (!isHotspotEnabled(selected, i)) {
				// 指定したホットスポットが非表示にされている場合
				continue;
			}

			int id = result->getHotspotId(selected, i);
			if (id == -1) {
				// ID未割り当ての場合
				continue;
			}

			// 中心画像のホットスポットの位置を取得する
			int x0, y0;
			getHotspotPos(PICTURE_POS_CENTER, infraredWidth, infraredHeight,
					canvasWidth, canvasHeight, getDirection(selected), selected,
					i, &x0, &y0);

			HotspotLink link = (*hotspotLinker)[id];
			__foreach (HotspotNumber, hotspot, link)
			{
				if (selected == hotspot->pictureNo) {
					continue;
				}
				for (int k = 0; k < PICTURE_POS_CENTER; k++) {
					int picNo = result->getAroundPictureId(selected, k);
					// (2020/01/08LEE) 画面を右、左に順番で対応するために追加
					if (k == PICTURE_POS_EAST) {
						if (picNo == checkNorth || picNo == checkSouth) {
							picNo = -1;
						}
					}
					if (k == PICTURE_POS_WEST) {
						if (picNo == checkNorth || picNo == checkSouth) {
							picNo = -1;
						}
					}

					if (hotspot->pictureNo == picNo) {
						// 撮影方位を取得する
						const double direction = getDirection(picNo);
						// 周囲画像のホットスポットの位置を取得する
						int x, y;
						getHotspotPos(k, infraredWidth, infraredHeight,
								canvasWidth, canvasHeight, direction, picNo,
								hotspot->pointNo, &x, &y);
						double dl = atan2(-y + y0, x - x0);
						int dx = cos(dl) * HOTSPOT_CIRCLE_RADIUS;
						int dy = sin(dl) * HOTSPOT_CIRCLE_RADIUS;

						// リンク線を描画する
						HPEN oldPen;
						switch (result->getHotspotType(hotspot->pictureNo,
								hotspot->pointNo)) {
						case HOTSPOT_TYPE_KEYPOINT:
							oldPen = (HPEN) canvas_Main->selectGdiObject(
									bluePen);
							break;
						case HOTSPOT_TYPE_DISABLED:
							oldPen = (HPEN) canvas_Main->selectGdiObject(
									grayPen);
							break;
						case HOTSPOT_TYPE_CANDIDATE:
						default:
							oldPen = (HPEN) canvas_Main->selectGdiObject(
									redPen);
							break;
						}
						canvas_Main->drawLine(x - dx, y + dy, x0 + dx, y0 - dy);
						canvas_Main->selectGdiObject(oldPen);
						break;
					}
				}
			}
		}
		// ID表示
		this->canvas_Main->drawId(selected, result->getDataCount());
	}

	// 作成したペンを削除
	DeleteObject(whitePen);
	DeleteObject(redPen);
	DeleteObject(greenPen);
	DeleteObject(bluePen);
	DeleteObject(grayPen);
	DeleteObject(bluePen);
	// (2017/7/31YM)ペンの削除を追加
	DeleteObject(redPen2);

	// Graphicsオブジェクトを破棄
	delete graphics;

	// ホットスポット表示状態を表示する
	this->canvas_Main->drawHotspotShowMode();

	return true;
}

// (2020/01/09LEE)
bool MainWindow::canvas_Main_HotspotAdjustmentOnPaint(void) {
	int x1 = this->prevMainPos.x;
	int y1 = this->prevMainPos.y;
	int x2 = this->mainX;
	int y2 = this->mainY;
	int type = 0;
	bool showLine = false;
	if ((blMouse == true) && (getHotspotShowMode() != 0)) {
		// キャンバスサイズを取得する
		const int canvasWidth = canvas_Main->clientWidth();
		const int canvasHeight = canvas_Main->clientHeight();

		// 熱画像サイズを取得する
		const int infraredWidth = result->getInfraredWidth();
		const int infraredHeight = result->getInfraredHeight();

		// (2020/01/08LEE) 画面を右、左に順番で対応するために追加
		int pictureposion;
		int checkNorth, checkEast, checkSouth, checkWest;
		int checkposition[5] = { 0 };

		for (int i = 0; i < PICTURE_POS_COUNT; i++) {
			checkposition[i] = getAroundPictureId(getSelectedId(), i);
		}
		checkNorth = checkposition[0];
		checkEast = checkposition[1];
		checkSouth = checkposition[2];
		checkWest = checkposition[3];

		// (2020/01/08LEE) 画面を右、左に順番で対応するために追加

		HotspotNumber selected1 = this->getHotspotOnAdjustmentMode(x1, y1);
		if (!isNotHotspotSelected(selected1)) {
			if (!isHotspotEnabled(selected1.pictureNo, selected1.pointNo)) {
				// 表示されていないホットスポットはそのままにする
				selected1.pictureNo = ResultData::NOT_ASSIGNED;
			} else {
				// 表示されているホットスポットは円に接続される
				int i = 0;
				for (i = 0; i < PICTURE_POS_COUNT; i++) {
					if (result->getAroundPictureId(this->getSelectedId(), i)
							== selected1.pictureNo) {
						break;
					}
				}
				pictureposion = i;
				double direction = this->getDirection(selected1.pictureNo);
				// (2020/01/08LEE) 画面を右、左に順番で対応するために追加
				if (i == PICTURE_POS_EAST) {
					if (checkEast == checkNorth) {
						pictureposion = 0;
					}
					if (checkEast == checkSouth) {
						pictureposion = 2;
					}
				}

				if (i == PICTURE_POS_WEST) {
					if (checkWest == checkNorth) {
						pictureposion = 0;
					}
					if (checkWest == checkSouth) {
						pictureposion = 2;
					}
				}
				// (2020/01/08LEE) 画面を右、左に順番で対応するために追加
				getHotspotPos(pictureposion, infraredWidth, infraredHeight,
						canvasWidth, canvasHeight, direction,
						selected1.pictureNo, selected1.pointNo, &x1, &y1);
			}
		}

		HotspotNumber selected2 = this->getHotspotOnAdjustmentMode(x2, y2);
		if (!isNotHotspotSelected(selected2)) {
			if (!isHotspotEnabled(selected2.pictureNo, selected2.pointNo)) {
				// 表示されていないホットスポットはそのままにする
				selected2.pictureNo = ResultData::NOT_ASSIGNED;
			} else if ((selected1.pictureNo == selected2.pictureNo)
					&& (selected1.pointNo != selected2.pointNo)) {
				// 同一画像上のホットスポットもそのままにする
				selected2.pictureNo = ResultData::NOT_ASSIGNED;
			} else {
				// ホットスポット表示円に接続する
				int i = 0;
				for (i = 0; i < PICTURE_POS_COUNT; i++) {
					if (result->getAroundPictureId(this->getSelectedId(), i)
							== selected2.pictureNo) {
						break;
					}
				}
				double direction = this->getDirection(selected2.pictureNo);
				pictureposion = i;
				if (i == PICTURE_POS_EAST) {
					if (checkEast == checkNorth) {
						pictureposion = 0;
					}
					if (checkEast == checkSouth) {
						pictureposion = 2;
					}
				}

				if (i == PICTURE_POS_WEST) {
					if (checkWest == checkNorth) {
						pictureposion = 0;
					}
					if (checkWest == checkSouth) {
						pictureposion = 2;
					}
				}

				getHotspotPos(pictureposion, infraredWidth, infraredHeight,
						canvasWidth, canvasHeight, direction,
						selected2.pictureNo, selected2.pointNo, &x2, &y2);
			}
		}

		if (!isNotHotspotSelected(selected1)) {
			double dl = atan2(-y2 + y1, x2 - x1);
			int dx = cos(dl) * HOTSPOT_CIRCLE_RADIUS;
			int dy = sin(dl) * HOTSPOT_CIRCLE_RADIUS;
			x1 += dx;
			y1 -= dy;
			type = 1;
		}

		if (!isNotHotspotSelected(selected2)) {
			double dl = atan2(-y2 + y1, x2 - x1);
			int dx = cos(dl) * HOTSPOT_CIRCLE_RADIUS;
			int dy = sin(dl) * HOTSPOT_CIRCLE_RADIUS;
			x2 -= dx;
			y2 += dy;
			type = 1;
		}

		showLine = true;
	}

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(canvas_Main->getHandle(), &ps);
	canvas_Main->transfer(hdc);
	if (showLine) {
		HPEN pen;
		if (type == 0) {
			pen = CreatePen(PS_SOLID, 2, RGB(128, 255, 128));
		} else if (type == 1) {
			pen = CreatePen(PS_SOLID, 2, RGB(255, 128, 128));
		} else {
			pen = CreatePen(PS_SOLID, 2, RGB(128, 128, 255));
		}
		HPEN oldPen = (HPEN) SelectObject(hdc, pen);
		MoveToEx(hdc, x1, y1, NULL);
		LineTo(hdc, x2, y2);
		DeleteObject(SelectObject(hdc, oldPen));
	}
	EndPaint(canvas_Main->getHandle(), &ps);

	return true;
}

LRESULT MainWindow::canvas_Main_HotspotAdjustmentProc(HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_LBUTTONDOWN:
		break;
	case WM_RBUTTONDOWN:
		// 全非表示モードの場合は何もしない
		if (getHotspotShowMode() != 0) {
			if (blMouse == true) {
				blMouse = false;
				canvasUpdate();
			}
		}
		break;

	case WM_LBUTTONUP:
		break;

	case WM_MOUSEMOVE:
		this->canvas_Main->update();
		break;

	case WM_PAINT:
		this->canvas_Main_HotspotAdjustmentOnPaint();
		break;

	default:
		break;
	}
	return 0;
}

int MainWindow::getClickedPictureId(int x, int y) {
	// キャンバスサイズを取得する
	const int canvasWidth = canvas_Main->clientWidth();
	const int canvasHeight = canvas_Main->clientHeight();
	// 熱画像サイズを取得する
	const int infraredWidth = result->getInfraredWidth();
	const int infraredHeight = result->getInfraredHeight();

	// (2020/01/08LEE) 画面を右、左に順番で対応するために追加
	int checkNorth, checkEast, checkSouth, checkWest;
	int checkposition[5] = { 0 };

	for (int i = 0; i < PICTURE_POS_COUNT; i++) {
		checkposition[i] = getAroundPictureId(getSelectedId(), i);
	}
	checkNorth = checkposition[0];
	checkEast = checkposition[1];
	checkSouth = checkposition[2];
	checkWest = checkposition[3];
	// (2020/01/08LEE) 画面を右、左に順番で対応するために追加

	for (int i = 0; i < PICTURE_POS_COUNT; i++) {
		// 画像番号を取得する
		int id = getAroundPictureId(getSelectedId(), i);

		// (2020/01/08LEE) 画面を右、左に順番で対応するために追加
		if (i == PICTURE_POS_EAST) {
			if (checkEast == checkNorth || checkEast == checkSouth) {
				id = -1;
			}
		}

		if (i == PICTURE_POS_WEST) {
			if (checkWest == checkNorth || checkWest == checkSouth) {
				id = -1;
			}
		}

		// (2020/01/08LEE) 画面を右、左に順番で対応するために追加
		// その方位に画像がない場合
		if (id < 0) {
			continue;
		}

		// 撮影方位を取得する
		const double direction = getDirection(id);

		// 画像の表示位置と画像サイズ(倍率)を取得する
		int cx, cy;
		double ratio;
		getPicturePos(i, infraredWidth, infraredHeight, canvasWidth,
				canvasHeight, direction, &cx, &cy, &ratio);

		POINT p[4];
		::getPictureRect(infraredWidth, infraredHeight, ratio, direction, cx,
				cy, p);

		POINT mousePos = { x, y };
		if (isPointInPolygon(mousePos, 4, p)) {
			return id;
		}
	}
	return -1;
}

bool MainWindow::checkHotspot(int x, int y) {
	// キャンバスサイズを取得する
	const int canvasWidth = canvas_Main->clientWidth();
	const int canvasHeight = canvas_Main->clientHeight();

	// 熱画像サイズを取得する
	const int infraredWidth = result->getInfraredWidth();
	const int infraredHeight = result->getInfraredHeight();

	// (2020/01/08LEE) 画面を右、左に順番で対応するために追加
	int checkNorth, checkEast, checkSouth, checkWest;
	int checkposition[5] = { 0 };

	for (int i = 0; i < PICTURE_POS_COUNT; i++) {
		checkposition[i] = getAroundPictureId(getSelectedId(), i);
	}
	checkNorth = checkposition[0];
	checkEast = checkposition[1];
	checkSouth = checkposition[2];
	checkWest = checkposition[3];

	// (2020/01/08LEE) 画面を右、左に順番で対応するために追加

	for (int i = 0; i < PICTURE_POS_COUNT; i++) {
		// 画像番号を取得する
		int id = getAroundPictureId(getSelectedId(), i);

		// (2020/01/08LEE) 画面を右、左に順番で対応するために追加
		if (i == PICTURE_POS_EAST) {
			if (checkEast == checkNorth || checkEast == checkSouth) {
				id = -1;
			}
		}

		if (i == PICTURE_POS_WEST) {
			if (checkWest == checkNorth || checkWest == checkSouth) {
				id = -1;
			}
		}

		// (2020/01/08LEE) 画面を右、左に順番で対応するために追加

		// その方位に画像がない場合
		if (id < 0) {
			continue;
		}

		// 撮影方位を取得する
		const double direction = getDirection(id);

		// 画像の表示位置と画像サイズ(倍率)を取得する
		int cx, cy;
		double ratio;
		getPicturePos(i, infraredWidth, infraredHeight, canvasWidth,
				canvasHeight, direction, &cx, &cy, &ratio);

		POINT p[4];
		::getPictureRect(infraredWidth, infraredHeight, ratio, direction, cx,
				cy, p);

		POINT mousePos = { x, y };
		if (!isPointInPolygon(mousePos, 4, p)) {
			continue;
		}

		for (int i = 0; i < result->getHotspotCount(id); i++) {
			int lx, ly;
			getHotspotPos_2(id, i, infraredWidth, infraredHeight, direction,
					ratio, cx, cy, &lx, &ly);
			int dx = x - lx;
			int dy = y - ly;
			if (sqrt(dx * dx + dy * dy) < HOTSPOT_CIRCLE_RADIUS) {
				return true;
			}
		}
		break;
	}
	return false;
}

void MainWindow::addHotspot(int x, int y, int type) {
	bool saveHotspot = false;
	bool saveLink = false;

	// キャンバスサイズを取得する
	const int canvasWidth = canvas_Main->clientWidth();
	const int canvasHeight = canvas_Main->clientHeight();

	// 熱画像サイズを取得する
	const int infraredWidth = result->getInfraredWidth();
	const int infraredHeight = result->getInfraredHeight();

	// (2020/01/08LEE) 画面を右、左に順番で対応するために追加
	int checkNorth, checkEast, checkSouth, checkWest;
	int checkposition[5] = { 0 };

	for (int i = 0; i < PICTURE_POS_COUNT; i++) {
		checkposition[i] = getAroundPictureId(getSelectedId(), i);
	}
	checkNorth = checkposition[0];
	checkEast = checkposition[1];
	checkSouth = checkposition[2];
	checkWest = checkposition[3];

	// (2020/01/08LEE) 画面を右、左に順番で対応するために追加
	for (int i = 0; i < PICTURE_POS_COUNT; i++) {
		// 画像番号を取得する
		int id = getAroundPictureId(getSelectedId(), i);
		// (2020/01/08LEE) 画面を右、左に順番で対応するために追加
		if (i == PICTURE_POS_EAST) {
			if (checkEast == checkNorth || checkEast == checkSouth) {
				id = -1;
			}
		}
		if (i == PICTURE_POS_WEST) {
			if (checkWest == checkNorth || checkWest == checkSouth) {
				id = -1;
			}
		}

		// その方位に画像がない場合
		if (id < 0) {
			continue;
		}

		// 撮影方位を取得する
		const double direction = getDirection(id);

		// 画像の表示位置と画像サイズ(倍率)を取得する
		int cx, cy;
		double ratio;
		getPicturePos(i, infraredWidth, infraredHeight, canvasWidth,
				canvasHeight, direction, &cx, &cy, &ratio);

		POINT p[4];
		::getPictureRect(infraredWidth, infraredHeight, ratio, direction, cx,
				cy, p);

		POINT mousePos = { x, y };
		if (!isPointInPolygon(mousePos, 4, p)) {
			continue;
		}

		// ホットスポットの削除数
		int deleted = 0;
		// 非表示のホットスポット数
		int invisible = 0;
		//
		for (int i = result->getHotspotCount(id) - 1; i >= 0; i--) {
			int lx, ly;
			getHotspotPos_2(id, i, infraredWidth, infraredHeight, direction,
					ratio, cx, cy, &lx, &ly);
			int dx = x - lx;
			int dy = y - ly;
			if (sqrt(dx * dx + dy * dy) < HOTSPOT_CIRCLE_RADIUS) {
				if (isHotspotEnabled(id, i)) {
					// (2017/10/26YM)出力対象ホットスポットリストのNoを更新する処理を追加
					int HSNO;
					HSNO = result->getHotspotID(id, i);
					mainWindow->refreshHotspotIdList(HSNO);
					// (2017/10/26YM)出力対象ホットスポットリストから削除する処理を追加
					mainWindow->removeHotspotIdList(HSNO);

					saveHotspotIdList();
					// 指定したホットスポットが表示されている場合のみ削除する
					hotspotLinker->removeHotspot(id, i);
				} else {
					invisible++;
				}
				deleted++;
			}
		}

		if ((deleted != 0) && (deleted == invisible)) {
			// 非表示ホットスポット上をクリックした場合
			for (int i = result->getHotspotCount(id) - 1; i >= 0; i--) {
				int lx, ly;
				getHotspotPos_2(id, i, infraredWidth, infraredHeight, direction,
						ratio, cx, cy, &lx, &ly);
				int dx = x - lx;
				int dy = y - ly;
				if (sqrt(dx * dx + dy * dy) < HOTSPOT_CIRCLE_RADIUS) {
					changeDisabledHotspots(id, i, HOTSPOT_TYPE_ENABLED);
					if (result->isHotspotCandidate(id, i)) {
						result->enableHotspot(id, i);
					}
				}
			}
		}
		if ((deleted == 0)
				&& (((type == HOTSPOT_TYPE_ENABLED)
						&& (this->getEnabledHotspotShowMode() == true))
						|| ((type == HOTSPOT_TYPE_KEYPOINT)
								&& (this->getKeypointShowMode() == true))
						|| ((type == HOTSPOT_TYPE_CANDIDATE)
								&& (this->getHotspotCandidateShowMode() == true))
						|| ((type == HOTSPOT_TYPE_DISABLED)
								&& (this->getDisabledHotspotShowMode() == true)))) {
			int px, py;
			px = (x - cx) / ratio;
			py = (y - cy) / ratio;
			int lx, ly;
			lx = px * cos(-direction) - py * sin(-direction);
			ly = px * sin(-direction) + py * cos(-direction);
			lx = lx + infraredWidth / 2;
			ly = ly + infraredHeight / 2;
			result->addHotspot(id, lx, ly, type);
		} else {
			unselectHotspot();
		}

		saveHotspot = true;
		saveLink = true;

		updateHotspot();
		break;
	}

	if (saveHotspot) {
		// 変更がある場合はファイルに保存する
		result->saveHotspot();
	}
	if (saveLink) {
		// リンクを更新したらデータを保存する
		hotspotLinker->saveTo(result->getHotspotLinkFileName());
	}
	int id = getClickedPictureId(mainX, mainY);
	HWND window = canvas_Main->getHandle();
	PictureWindow::Windowupdate(window, id); // (2019/12/04LEE) 追加
	canvasUpdate();
}

bool MainWindow::addLink(void) {
	// 画像上をクリックしているか確認
	int clickedId = getClickedPictureId(mainX, mainY);
	if (clickedId < 0 || clickedId >= result->getDataCount()) {
		this->blMouse = false;
		return false;
	}

	int x1 = this->prevMainPos.x;
	int y1 = this->prevMainPos.y;
	int x2 = this->mainX;
	int y2 = this->mainY;

	// 画像番号を取得する
	int pic1 = this->getClickedPictureId(x1, y1);
	int pic2 = clickedId;

	if (pic1 == pic2) {
		// 同一画像上をクリック
		this->blMouse = false;
		return false;
	}

	if ((pic1 != this->getSelectedId()) && (pic2 != this->getSelectedId())) {
		// 中央の画像以外とリンクを作成しない
		showMessageBox(
		IDS_HOTSPOT_ADD_FAILURE2,
		IDS_HOTSPOT_ADJUST_TITLE,
		MB_OK | MB_ICONINFORMATION);
		this->blMouse = false;
		return false;
	}

	bool saveHotspot = false;
	bool saveLink = false;

	// 異なる画像上をクリック
	int type1 = HOTSPOT_TYPE_UNKNOWN, type2 = HOTSPOT_TYPE_UNKNOWN;
	HotspotNumber selected1 = this->getHotspotOnAdjustmentMode(x1, y1);
	HotspotNumber selected2 = this->getHotspotOnAdjustmentMode(x2, y2);
	if (!isNotHotspotSelected(selected1)) {
		// 始点にホットスポットがある場合
		type1 = result->getHotspotType(selected1.pictureNo, selected1.pointNo);
	}

	if (!isNotHotspotSelected(selected2)) {
		// 終点にホットスポットがある場合
		type2 = result->getHotspotType(selected2.pictureNo, selected2.pointNo);
	}

	bool canAddLink = true;
	if ((type1 == HOTSPOT_TYPE_UNKNOWN) && (type2 == HOTSPOT_TYPE_UNKNOWN)) {
		// 両方とも新しい場合
		addHotspot(x1, y1, HOTSPOT_TYPE_KEYPOINT);
		addHotspot(x2, y2, HOTSPOT_TYPE_KEYPOINT);
		selected1 = this->getHotspotOnAdjustmentMode(x1, y1);
		selected2 = this->getHotspotOnAdjustmentMode(x2, y2);
	} else if (type1 == HOTSPOT_TYPE_UNKNOWN) {
		// 1つめのみ新しい場合
		int id2 = result->getHotspotId(selected2.pictureNo, selected2.pointNo);
		if (!hotspotLinker->existsId(id2, pic1)) {
			if ((type2 == HOTSPOT_TYPE_CANDIDATE)
					|| (type2 == HOTSPOT_TYPE_DISABLED)) {
				type2 = HOTSPOT_TYPE_ENABLED;
			}
			addHotspot(x1, y1, type2);
			selected1 = this->getHotspotOnAdjustmentMode(x1, y1);
		} else {
			canAddLink = false;
		}
	} else if (type2 == HOTSPOT_TYPE_UNKNOWN) {
		// 2つめのみ新しい場合
		int id1 = result->getHotspotId(selected1.pictureNo, selected1.pointNo);
		if (!hotspotLinker->existsId(id1, pic2)) {
			if ((type1 == HOTSPOT_TYPE_CANDIDATE)
					|| (type1 == HOTSPOT_TYPE_DISABLED)) {
				type1 = HOTSPOT_TYPE_ENABLED;
			}
			addHotspot(x2, y2, type1);
			selected2 = this->getHotspotOnAdjustmentMode(x2, y2);
		} else {
			canAddLink = false;
		}
	}

	int picNo1 = selected1.pictureNo;
	int ptNo1 = selected1.pointNo;
	int picNo2 = selected2.pictureNo;
	int ptNo2 = selected2.pointNo;

	if (isNotHotspotSelected(selected1) || isNotHotspotSelected(selected2)) {
		// ホットスポットのどちらかが存在しない場合
		canAddLink = false;
	} else if (hotspotLinker->testWithoutType(picNo1, ptNo1, picNo2, ptNo2)) {
		// 両方存在している
		if ((result->isDisabledHotspot(picNo1, ptNo1))
				|| (result->isDisabledHotspot(picNo2, ptNo2))) {
			// どちらか一方(または両方)が削除候補の場合
			if ((type1 == HOTSPOT_TYPE_KEYPOINT)
					|| (type2 == HOTSPOT_TYPE_KEYPOINT)) {
				// 接続相手が特徴点の場合
				changeDisabledHotspots(picNo1, ptNo1, HOTSPOT_TYPE_KEYPOINT);
				changeDisabledHotspots(picNo2, ptNo2, HOTSPOT_TYPE_KEYPOINT);
			} else {
				// 有効化する
				changeDisabledHotspots(picNo1, ptNo1, HOTSPOT_TYPE_ENABLED);
				changeDisabledHotspots(picNo2, ptNo2, HOTSPOT_TYPE_ENABLED);
			}
			saveHotspot = true;
		}
	}

	if (canAddLink == true) {
		if (hotspotLinker->add(picNo1, ptNo1, picNo2, ptNo2)
				== ResultData::NOT_ASSIGNED) {
			// リンクの作成に失敗した場合
			showMessageBox(
			IDS_HOTSPOT_ADD_FAILURE,
			IDS_HOTSPOT_ADJUST_TITLE,
			MB_OK | MB_ICONINFORMATION);
		} else {
			// 未確定ホットスポットを無効化する
			HotspotNumber hotspot1, hotspot2;
			hotspot1 = hotspotNo(picNo1, ptNo1);
			hotspot2 = hotspotNo(picNo2, ptNo2);
			bool changed1 = fixHotspotCandidates(hotspot1, hotspot2);
			bool changed2 = fixHotspotCandidates(hotspot2, hotspot1);
			if ((changed1 == true) || (changed2 == true)) {
				saveHotspot = true;
			}
			saveLink = true;
		}
	} else {
		// リンクの作成に失敗した場合
		showMessageBox(
		IDS_HOTSPOT_ADD_FAILURE,
		IDS_HOTSPOT_ADJUST_TITLE,
		MB_OK | MB_ICONINFORMATION);
	}

	if (saveHotspot) {
		// 変更がある場合はファイルに保存する
		result->saveHotspot();
		updateHotspot();
	}

	if (saveLink) {
		// リンクの作成に成功したらデータを保存する
		hotspotLinker->saveTo(result->getHotspotLinkFileName());
		updateHotspot();
	}

	if (saveHotspot || saveLink) {
		// ホットスポット/リンクに変更がある場合
		HWND window = canvas_Main->getHandle();
		PictureWindow::Windowupdate(window, selected1.pictureNo); // (2019/12/04LEE) 追加
		canvasUpdate();
	}

	this->blMouse = false;
	return true;
}

HotspotNumber MainWindow::getHotspotOnAdjustmentMode(int x, int y) {
	// キャンバスサイズを取得する
	const int canvasWidth = canvas_Main->clientWidth();
	const int canvasHeight = canvas_Main->clientHeight();

	// 熱画像サイズを取得する
	const int infraredWidth = result->getInfraredWidth();
	const int infraredHeight = result->getInfraredHeight();

	// (2020/01/08LEE) 画面を右、左に順番で対応するために追加
	int checkNorth, checkEast, checkSouth, checkWest;
	int checkposition[5] = { 0 };

	for (int i = 0; i < PICTURE_POS_COUNT; i++) {
		checkposition[i] = getAroundPictureId(getSelectedId(), i);
	}
	checkNorth = checkposition[0];
	checkEast = checkposition[1];
	checkSouth = checkposition[2];
	checkWest = checkposition[3];
	// (2020/01/08LEE) 画面を右、左に順番で対応するために追加

	for (int i = 0; i < PICTURE_POS_COUNT; i++) {
		// 画像番号を取得する
		int id = getAroundPictureId(getSelectedId(), i);
		// (2020/01/08LEE) 画面を右、左に順番で対応するために追加
		if (i == PICTURE_POS_EAST) {
			if (checkEast == checkNorth || checkEast == checkSouth) {
				id = -1;
			}
		}
		if (i == PICTURE_POS_WEST) {
			if (checkWest == checkNorth || checkWest == checkSouth) {
				id = -1;
			}
		}
		// (2020/01/08LEE) 画面を右、左に順番で対応するために追加
		// その方位に画像がない場合
		if (id < 0) {
			continue;
		}

		// 撮影方位を取得する
		const double direction = getDirection(id);

		// 画像の表示位置と画像サイズ(倍率)を取得する
		for (int j = 0; j < result->getHotspotCount(id); j++) {
			int x0, y0;
			getHotspotPos(i, infraredWidth, infraredHeight, canvasWidth,
					canvasHeight, direction, id, j, &x0, &y0);
			int dx = x - x0;
			int dy = y - y0;
			if (sqrt(dx * dx + dy * dy) < HOTSPOT_CIRCLE_RADIUS) {
				return hotspotNo(id, j);
			}
		}
	}
	return hotspotNo(NOT_SELECTED, NOT_SELECTED);
}

int MainWindow::deleteLinkLineSegment(int x, int y, int radius) {
	// キャンバスサイズを取得する
	const int canvasWidth = canvas_Main->clientWidth();
	const int canvasHeight = canvas_Main->clientHeight();

	// 熱画像サイズを取得する
	const int infraredWidth = result->getInfraredWidth();
	const int infraredHeight = result->getInfraredHeight();

	Vector2D p;
	p.x = x;
	p.y = y;

	// (2020/01/08LEE) 画面を右、左に順番で対応するために追加
	int checkNorth, checkEast, checkSouth, checkWest;
	int checkposition[5] = { 0 };

	for (int i = 0; i < PICTURE_POS_COUNT; i++) {
		checkposition[i] = getAroundPictureId(getSelectedId(), i);
	}
	checkNorth = checkposition[0];
	checkEast = checkposition[1];
	checkSouth = checkposition[2];
	checkWest = checkposition[3];

	int selected = getSelectedId();
	int count = result->getHotspotCount(selected);
	for (int i = 0; i < count; i++) {
		if (!isHotspotEnabled(selected, i)) {
			// 指定したホットスポットが非表示にされている場合
			continue;
		}
		int id = result->getHotspotId(selected, i);
		if (id == ResultData::NOT_ASSIGNED) {
			// ID未割り当ての場合
			continue;
		}
		// 中心画像のホットスポットの位置を取得する
		int x0, y0;
		getHotspotPos(PICTURE_POS_CENTER, infraredWidth, infraredHeight,
				canvasWidth, canvasHeight, getDirection(selected), selected, i,
				&x0, &y0);

		HotspotLink link = (*hotspotLinker)[id];
		__foreach (HotspotNumber, hotspot, link)
		{
			if (selected == hotspot->pictureNo) {
				continue;
			}
			for (int k = 0; k < PICTURE_POS_CENTER; k++) {
				int picNo = result->getAroundPictureId(selected, k);
				int directionId;
				if (hotspot->pictureNo == picNo) {
					if (!isHotspotEnabled(hotspot->pictureNo,
							hotspot->pointNo)) {
						// 指定したホットスポットが非表示にされている場合
						continue;
					}
					directionId = k;
					// (2020/01/23LEE) Linkを消す場合、写真の順番で位置が決めてあるのでまた、計算してホットスポットがある写真のdirectionIdを更新
					if (k == PICTURE_POS_EAST) {
						if (picNo == checkNorth) {
							directionId = PICTURE_POS_NORTH;
						}
						if (picNo == checkSouth) {
							directionId = PICTURE_POS_SOUTH;
						}
					}

					if (k == PICTURE_POS_WEST) {
						if (picNo == checkNorth) {
							directionId = PICTURE_POS_NORTH;
						}
						if (picNo == checkSouth) {
							directionId = PICTURE_POS_SOUTH;
						}
					}

					// 撮影方位を取得する
					double direction = getDirection(picNo);
					// 周囲画像のホットスポットの位置を取得する
					int x, y;
					getHotspotPos(directionId, infraredWidth, infraredHeight,
							canvasWidth, canvasHeight, direction, picNo,
							hotspot->pointNo, &x, &y);
					double dl = atan2(-y + y0, x - x0);
					int dx = cos(dl) * radius;
					int dy = sin(dl) * radius;
					// リンクを削除する
					LineSegment linkLine;
					linkLine.pt1.x = x - dx;
					linkLine.pt1.y = y + dy;
					linkLine.pt2.x = x0 + dx;
					linkLine.pt2.y = y0 - dy;
					if (distanceLineSegmentAndPoint(linkLine, p) < 4) {
						// 線分の両端のホットスポットのIDを未割り当てにする
						hotspotLinker->remove(selected, i);
						hotspotLinker->remove(picNo, hotspot->pointNo);
						// 更新したらデータを保存する
						hotspotLinker->saveTo(result->getHotspotLinkFileName());
						updateHotspot();
						return true;
					}
					break;
				}
			}
		}
	}
	return false;
}

/**
 * ホットスポット表示モードを切り替える
 */
void MainWindow::changeHotspotShowMode(int mode) {
	int modeValue = this->getHotspotShowMode();

	modeValue = (modeValue + mode + 3) % 3;

	TCHAR navigatorText[Resource::MAX_LOADSTRING];

	switch (modeValue) {
	case 0:
		hotspotShowMode = false;
		hotspotCandidateShowMode = false;
		disabledHotspotShowMode = false;
		keypointShowMode = false;
		Resource::getString(IDS_NAVI_HOTSPOT, navigatorText);
		break;

	case 1:
		hotspotShowMode = true;
		hotspotCandidateShowMode = false;
		disabledHotspotShowMode = false;
		keypointShowMode = true;
		Resource::getString(IDS_NAVI_HOTSPOT_1, navigatorText);
		break;

	default:
		hotspotShowMode = true;
		hotspotCandidateShowMode = true;
		disabledHotspotShowMode = true;
		keypointShowMode = true;
		Resource::getString(IDS_NAVI_HOTSPOT_2, navigatorText);
		break;
	}

	// モード切替時にリンクされたホットスポット候補を有効化する
	if (getSelectedId() != 0) {
		bool changed = false;
		const int DATA_COUNT = result->getDataCount();
		for (int i = 0; i < DATA_COUNT; i++) {
			const int HOTSPOT_COUNT = result->getHotspotCount(i);
			for (int j = 0; j < HOTSPOT_COUNT; j++) {
				if ((result->isHotspotCandidate(i, j) == true)
						&& (result->isUniqueHotspot(i, j) == false)) {
					result->enableHotspot(i, j);
					changed = true;
				}
			}
		}
		if (changed == true) {
			result->saveHotspot();
		}
	}
	controlPanel->setNavigatorText(navigatorText);

	canvasUpdate();
}

void MainWindow::updateHotspot(void) {
	isHotspotUpdated = true;
}
