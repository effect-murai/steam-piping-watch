/*
 * PanelInfo.cpp
 *
 *  Created on: 2016/01/27
 *      Author: PC-EFFECT-002
 */

#include "app.h"
#include <stdio.h>
#include "PanelInfo.h"
#include "Graphics.h"
#include "resource.h"

#include "StringUtils.h"
#include "Matrix.h"
#include "Data/VectorOp.h"

extern ResultData *result;

//------------------------------------------------------------------------------
// Global Functions
//------------------------------------------------------------------------------

inline bool isRectangle(Vector2D *square) {
	// 各辺が水平/垂直であることを確認する
	for (int i = 0; i < 4; i++) {
		double dx = square[i].x - square[(i + 3) % 4].x;
		double dy = square[i].y - square[(i + 3) % 4].y;
		if ((((i % 2) == 0) && (dy != 0)) || // 横線が水平でない場合は長方形ではない
				(((i % 2) == 1) && (dx != 0))    // 縦線が垂直でない場合は長方形ではない
				) {
			return false;
		}
	}
	return true;
}

inline Matrix<Vector2D>* getInternalSplitCoordinates_Rect(Vector2D *area,
		int rows, int columns) {
	double width = (area[3].x - area[0].x) / columns;
	double height = (area[0].y - area[1].y) / rows;

	Matrix<Vector2D> *points = new Matrix<Vector2D>(rows + 1, columns + 1);
	for (int i = 0; i < rows + 1; i++) {
		for (int j = 0; j < columns + 1; j++) {
			Vector2D p;
			p.x = area[1].x + width * j;
			p.y = area[1].y + height * i;
			points->set(i, j, p);
		}
	}
	return points;
}

inline Matrix<Vector2D>* getInternalSplitCoordinates_Square(Vector2D *area,
		int rows, int columns) {
	// 2直線の交点
	// 四角形の頂点も交点に含める
	Matrix<Vector2D> *crossPoints = new Matrix<Vector2D>(rows + 1, columns + 1);
	// 最初の頂点をセットする
	crossPoints->at(0, 0) = area[0];
	crossPoints->at(rows, 0) = area[1];
	crossPoints->at(rows, columns) = area[2];
	crossPoints->at(0, columns) = area[3];

	Vector2D pt1, pt2;

	// 各辺を分割する
	// pt[0] - pt[3]
	// 水平
	pt1 = area[0];
	pt2 = area[3];
	{
		double dx = pt2.x - pt1.x;
		double dy = pt2.y - pt1.y;
		double m, n; // 傾きm, 初期値n
		if (dx != 0) {
			// ※垂直は計算不可
			m = dy / dx;
			n = pt1.y - m * pt1.x;
			for (int j = 1; j < columns; j++) {
				crossPoints->at(0, j).x = dx * j / columns + pt1.x;
				crossPoints->at(0, j).y = m * crossPoints->at(0, j).x + n;
			}
		} else {
			// 垂直の場合
			for (int j = 1; j < columns; j++) {
				crossPoints->at(0, j).x = pt1.x;
				crossPoints->at(0, j).y = dy * j / columns + pt1.y;
			}
		}
	}
	// pt[0] - pt[1]
	// 垂直
	pt1 = area[0];
	pt2 = area[1];
	{
		double dx = pt2.x - pt1.x;
		double dy = pt2.y - pt1.y;
		double m, n; // 傾きm, 初期値n
		if (dx != 0) {
			// ※垂直は計算不可
			m = dy / dx;
			n = pt1.y - m * pt1.x;
			for (int i = 1; i < rows; i++) {
				crossPoints->at(i, 0).y = dy * i / rows + pt1.y;
				crossPoints->at(i, 0).x = (crossPoints->at(i, 0).y - n) / m;
			}
		} else {
			// 垂直の場合
			for (int i = 1; i < rows; i++) {
				crossPoints->at(i, 0).x = pt1.x;
				crossPoints->at(i, 0).y = dy * i / rows + pt1.y;
			}
		}
	}
	// pt[1] - pt[2]
	// 水平
	pt1 = area[1];
	pt2 = area[2];
	{
		double dx = pt2.x - pt1.x;
		double dy = pt2.y - pt1.y;
		double m, n; // 傾きm, 初期値n
		if (dx != 0) {
			// ※垂直は計算不可
			m = dy / dx;
			n = pt1.y - m * pt1.x;
			for (int j = 1; j < columns; j++) {
				crossPoints->at(rows, j).x = dx * j / columns + pt1.x;
				crossPoints->at(rows, j).y = m * crossPoints->at(rows, j).x + n;
			}
		} else {
			// 垂直の場合
			for (int j = 1; j < columns; j++) {
				crossPoints->at(rows, j).x = pt1.x;
				crossPoints->at(rows, j).y = dy * j / columns + pt1.y;
			}
		}
	}
	// pt[3] - pt[2]
	// 垂直
	pt1 = area[3];
	pt2 = area[2];
	{
		double dx = pt2.x - pt1.x;
		double dy = pt2.y - pt1.y;
		double m, n; // 傾きm, 初期値n
		if (dx != 0) {
			// ※垂直は計算不可
			m = dy / dx;
			n = pt1.y - m * pt1.x;
			for (int i = 1; i < rows; i++) {
				crossPoints->at(i, columns).y = dy * i / rows + pt1.y;
				crossPoints->at(i, columns).x = (crossPoints->at(i, columns).y
						- n) / m;
			}
		} else {
			// 垂直の場合
			for (int i = 1; i < rows; i++) {
				crossPoints->at(i, columns).y = dy * i / rows + pt1.y;
				crossPoints->at(i, columns).x = pt1.x;
			}
		}
	}

	// 交点を計算
	for (int i = 1; i < rows; i++) {
		LineSegment line_2;
		line_2.pt1 = crossPoints->at(i, 0);
		line_2.pt2 = crossPoints->at(i, columns);
		for (int j = 1; j < columns; j++) {
			LineSegment line_1;
			line_1.pt1 = crossPoints->at(0, j);
			line_1.pt2 = crossPoints->at(rows, j);
			crossPoints->at(i, j) = getCrossPoint(line_1, line_2);
			if (__isnan(crossPoints->at(i, j).x)) {
				delete crossPoints;
				crossPoints = NULL;
				return NULL;
			}
		}
	}
	return crossPoints;
}

Matrix<Vector2D>* getInternalSplitCoordinates(Vector2D *area, int rows,
		int columns) {
#ifdef UNUSE
	if (isRectangle(area)) 
	{
		return getInternalSplitCoordinates_Rect(area, rows, columns);
	} 
	else 
	{
		return getInternalSplitCoordinates_Square(area, rows, columns);
	}
#else
	return getInternalSplitCoordinates_Square(area, rows, columns);
#endif
}

//------------------------------------------------------------------------------
// Class Functions
//------------------------------------------------------------------------------

PanelData::PanelData(void) {
	selectedPanelId = NOT_SELECTED;
	internalId.x = 0;
	internalId.y = 0;
	std::vector<PanelInfo>().swap(panelInfo);
}

/**TODO
 * パネルIDをセットする
 * @return パネルID
 */
void PanelData::setSelectedPanelId(int id) {
	if ((id < 0) || (id >= (int) panelInfo.size())) {
		// 範囲外を指定した場合
		id = NOT_SELECTED;
	}

	if (selectedPanelId != id) {
		selectedPanelId = id;
		internalId.x = 0;
		internalId.y = 0;
	}
}

void PanelData::setSelectedInternalId(int x, int y) {
	internalId.x = x;
	internalId.y = y;
}

/**
 * 選択中のパネルIDを取得する
 * @return パネルID
 */
int PanelData::getSelectedPanelId(void) {
	return selectedPanelId;
}

POINT PanelData::getSelectedInternalId() {
	return internalId;
}

/**
 * Panel名をセットする
 * @param id Panel番号
 * @return bool
 */
bool PanelData::setPanelName(int id, TCHAR *name) {
	int length = lstrlen(name); //　文字数の取得
	delete panelInfo[id].panelName; //　メモリの開放
	panelInfo[id].panelName = new TCHAR[length + 1]; //　メモリの確保
	ZeroMemory(panelInfo[id].panelName, (length + 1) * sizeof(TCHAR)); //　0クリア
	memcpy(panelInfo[id].panelName, name, length * sizeof(TCHAR)); //　文字列書き込み
	return true;
}

/**
 * PanelNameを取得する
 *　@param id パネル固有番号
 * @return パネル名
 */
TCHAR* PanelData::getPanelName(int id) {
	return panelInfo[id].panelName;
}

/**TODO
 * PanelNameを取得する
 *　@param id パネル固有番号
 * @return bool
 */
bool PanelData::setPanelHotspotCount(int id, int count) {
	panelInfo[id].HotspotCount = count;
	return true;
}

/**
 * 離した座標の設定
 *　@param id パネル固有番号
 *　@param x  横座標
 *　@param y　　縦座標
 * @return bool
 */
bool PanelData::movePanel(int id, double x, double y) {
	for (int i = 0; i < 4; i++) {
		panelInfo[id].points[i].x += x;
		panelInfo[id].points[i].y += y;
	}
	return true;
}

Vector2D* PanelData::getPanelPos(int id) {
	/** @todo 暫定 */
	return panelInfo[id].points;
}

/**
 * Panel管理番号割当総数取得
 *　@return パネル管理番号割当総数
 */
int PanelData::getPanelNameCountMax(void) {
	return panelInfo.size();
}

/**
 * Panel内のホットスポット総数取得
 * @param id パネル管理番号
 * @return ホットスポット総数
 */
int PanelData::getHotspotPanelCount(int id) {
	return panelInfo[id].HotspotCount;
}

/**
 * panelData削除時のidをつめる処理
 * @param id パネル管理番号
 * @return bool
 */
bool PanelData::panelSettingDelete(int id) {
	if (0 <= id && id <= getPanelNameCountMax() - 1) {
		panelInfo.erase(panelInfo.begin() + id);
	}
	return true;
}

/**
 * panelData追加処理
 * @param x1　左下x座標
 * @param y1　左下y座標
 * @param x2　右上x座標
 * @param y2　右上y座標
 * @return パネルID
 */
int PanelData::panelSettingAdd(Vector2D *panelArea) {
	// 配列に要素を追加
	int newId = panelInfo.size();
	panelInfo.push_back(PanelInfo());

	// 追加した配列の要素に初期値を設定
	std::vector<PanelInfo>::iterator newPanel = panelInfo.begin() + newId;
	for (int i = 0; i < 4; i++) {
		newPanel->points[i] = panelArea[i];
	}

	newPanel->HotspotCount = 0;
	newPanel->panelName = NULL;
	setPanelName(newId, Resource::getString(IDS_PANEL_NAME_IS_REQUIRED));

	return newId;
}

/**
 * パネルデータを保存する
 */
bool PanelData::savePanelInfo(TCHAR *filePath) {
	// データをファイルに保存する
	FILE *file = fileOpen(filePath, TEXT("w"));
	if (file == NULL) {
		return false;
	}

	for (int i = 0; i < getPanelNameCountMax(); i++) {
		TCHAR *name = getPanelName(i);
		int length = lstrlen(name) + 1;
		int maxLen = length * 4;
		char *utf8String = new char[maxLen]; //　バッファ確保
		ZeroMemory(utf8String, maxLen); //　0クリア
		if (name != NULL) {
			toUTF8String(name, length, utf8String, maxLen);
		}
		PanelInfo *panelInfo = &this->panelInfo[i];
		fprintf(file, "%s %d", utf8String, panelInfo->HotspotCount);
		for (int i = 0; i < 4; i++) {
			fprintf(file, ",%lf,%lf", panelInfo->points[i].x,
					panelInfo->points[i].y);
		}
		fprintf(file, ",%d,%d", panelInfo->cx, panelInfo->cy);
		fprintf(file, "\n");
		delete utf8String; //　バッファ解放
	}
	fclose(file);
	return true;
}

/**
 * パネルデータを読み込む
 */
bool PanelData::loadPanelInfo(const TCHAR *path) {
	// ファイルを開く
	FILE *file = fileOpen(path, TEXT("r"));
	if (file == NULL) {
		return false;
	}
	int i = 0;

	int length = 100;
	int maxLen = length * 4;
	//　バッファ確保
	char *utf8String = new char[maxLen];
	TCHAR *name = new TCHAR[length];
	while (!feof(file)) {

		int count = 0, cx, cy;
		Vector2D square[4];

		ZeroMemory(utf8String, maxLen); //　0クリア
		int ret = fscanf(file, "%s %d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%d,%d\n",
				utf8String, &count, &square[0].x, &square[0].y, &square[1].x,
				&square[1].y, &square[2].x, &square[2].y, &square[3].x,
				&square[3].y, &cx, &cy);
		if (ret != 12) {
			// 指定されたフォーマットでない場合はその行を無視する
			continue;
		}
		fromUTF8String(utf8String, maxLen, name, length);
		panelSettingAdd(square);
		setPanelHotspotCount(i, count);
		setPanelName(i, name);
		setInternalSplitCount(i, cx, cy);
		i++;
	}

	//　バッファ解放
	delete name;
	name = NULL;
	delete utf8String;
	utf8String = NULL;

	fclose(file);
	return true;
}

bool PanelData::savePanelInfo(void) {
	TCHAR *Path = result->getPanelInfoPath();
	return savePanelInfo(Path);
}

bool PanelData::loadPanelInfo(void) {
	TCHAR *Path = result->getPanelInfoPath();
	return loadPanelInfo(Path);
}

/**
 * ホットスポット数を保存する
 */
bool PanelData::saveHotspotNumInfo(TCHAR *filePath) {
	// データをファイルに保存する
	FILE *file = fileOpen(filePath, TEXT("w"));
	if (file == NULL) {
		return false;
	}

	char *utf8String;
	int max = getPanelNameCountMax();
	this->panelInfo[0].HotspotCount;
	for (int i = 0; i < max; i++) {
		PanelInfo *panelInfo = &this->panelInfo[i];
		utf8String = toUTF8String(panelInfo->panelName);
		fprintf(file, "%s,", utf8String);					// 管理番号
		fprintf(file, "%d\n", panelInfo->HotspotCount);		// ホットスポット数
		delete utf8String; //　バッファ解放
	}

	fclose(file);
	return true;
}

/*
 * 緯度経度からパネル固有番号を取得する
 * 	@param x;// 経度 x軸
 * 	@param y;// 緯度 y軸
 */
TCHAR* PanelData::getPanelNameDetail(double x, double y) {
	int i;
	for (i = 0; i < (int) panelInfo.size(); i++) {
		Vector2D point;
		point.x = x;
		point.y = y;
		if (isPointInPolygon(point, 4, panelInfo[i].points)) {
			return panelInfo[i].panelName;
		}
	}
	return NULL;
}

/**
 * 指定した座標にあるパネルのIDを取得する。@n
 * 指定した座標にパネルがない場合は -1 を返す。
 * @param x x座標
 * @param y y座標
 * @return 指定した座標にあるパネルのID。@n
 * 指定した座標にパネルがない場合は -1。
 */
int PanelData::findPanel(double x, double y) {
	int last = (int) panelInfo.size() - 1;
	for (int i = last; i >= 0; i--) {
		Vector2D point;
		point.x = x;
		point.y = y;
		if (isPointInPolygon(point, 4, panelInfo[i].points)) {
			return i;
		}
	}
	return -1;
}

Vector2D PanelData::getPoint(int id, int pos) {
	Vector2D point;
	switch (pos) {
	case 0:
	case 1:
	case 2:
	case 3:
		point = panelInfo[id].points[pos];
		break;
	default:
		point.x = 0;
		point.y = 0;
	}
	return point;
}

void PanelData::setHotspotCount(int id, int count) {
	panelInfo[id].HotspotCount = count;
}

void PanelData::getInternalSplitCount(int id, SIZE *size) {
	size->cx = panelInfo[id].cx;
	size->cy = panelInfo[id].cy;
}

void PanelData::setInternalSplitCount(int id, int cx, int cy) {
	panelInfo[id].cx = cx;
	panelInfo[id].cy = cy;
}

bool PanelData::getInternalSquareFind(double x, double y, int *id, int *row,
		int *col, Vector2D *area) {
	int _id = findPanel(x, y);
	if (_id == -1) {
		return false;
	}

	Vector2D target;
	target.x = x;
	target.y = y;

	SIZE splitCount;
	getInternalSplitCount(_id, &splitCount);
	const int columns = splitCount.cx;
	const int rows = splitCount.cy;

	Vector2D panelArea[4];
	for (int i = 0; i < 4; i++) {
		panelArea[i] = getPoint(_id, i);
	}

	Matrix<Vector2D> *points = getInternalSplitCoordinates(panelArea, rows,
			columns);

	for (int i = 1; i < rows + 1; i++) {
		for (int j = 1; j < columns + 1; j++) {
			area[0] = points->get(i - 1, j - 1);
			area[1] = points->get(i, j - 1);
			area[2] = points->get(i, j);
			area[3] = points->get(i - 1, j);
			if (isPointInPolygon(target, 4, area)) {
				*id = _id;
				*row = i;
				*col = j;
				return true;
			}
		}
	}
	return false;
}

void PanelData::getInternalSquare(int id, int row, int column,
		Vector2D *square) {
	SIZE splitCount;
	getInternalSplitCount(id, &splitCount);
	const int columns = splitCount.cx;
	const int rows = splitCount.cy;

	Vector2D panelArea[4];
	for (int i = 0; i < 4; i++) {
		panelArea[i] = getPoint(id, i);
	}

	Matrix<Vector2D> *points = getInternalSplitCoordinates(panelArea, rows,
			columns);

	// 各座標を取得
	square[0] = points->get(row - 1, column - 1);
	square[1] = points->get(row, column - 1);
	square[2] = points->get(row, column);
	square[3] = points->get(row - 1, column);

	delete points;
	points = NULL;
}
