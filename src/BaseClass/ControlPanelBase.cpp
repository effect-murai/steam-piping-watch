/*
 * ControlPanelBase.cpp
 *
 *  Created on: 2016/01/28
 *      Author: PC-EFFECT-011
 */

#include "ControlPanelBase.h"

// (2017/2/16YM)インクルード宣言
#include "MainWindow.h"
#include "ResultData.h"

// (2017/2/16YM)外部参照変数宣言
extern MainWindow *mainWindow;
extern ResultData *result;

ControlPanelBase::ControlPanelBase() {
	this->m_pWnd = NULL;
	this->m_MoveV = ControlPanelBase::DEFAULT_VALUE_MOVE;
	this->m_MoveH = ControlPanelBase::DEFAULT_VALUE_MOVE;
	this->m_Turn = ControlPanelBase::DEFAULT_VALUE_TURN;
	this->m_Zoom = ControlPanelBase::DEFAULT_VALUE_ZOOM;
	this->m_Enabled = true;
	this->m_datatype = ControlPanelBase::DEFAULT_VALUE_DATA; // (2019/11/08LEE) 追加
}

ControlPanelBase::~ControlPanelBase() {
}

/**
 * 移動値(垂直方向)の取得
 */
double ControlPanelBase::getMoveV() {
	return 0;
}
/**
 * 移動値(垂直方向)の設定
 */
void ControlPanelBase::setMoveV(double data) {
}

/**
 * 移動値(水平方向)の取得
 */
double ControlPanelBase::getMoveH() {
	return 0;
}
/**
 * 移動値(水平方向)の設定
 */
void ControlPanelBase::setMoveH(double data) {
}

/**
 * 拡大・縮小値の取得
 */
double ControlPanelBase::getZoom() {
	return 0;
}
/**
 * 拡大・縮小値の設定
 */
void ControlPanelBase::setZoom(double data) {
}

/**
 * 拡大
 */
void ControlPanelBase::zoomIn() {
}

/**
 * 縮小
 */
void ControlPanelBase::zoomOut() {
}

/**
 * 回転角の取得
 */
double ControlPanelBase::getTurn() {
	return 0;
}
/**
 * 回転角の設定
 */
void ControlPanelBase::setTurn(double data) {
}

// (2019/11/06LEE) カメラ特性で全体と個別を区分。
int ControlPanelBase::getsetingtype() {
	return 0;
}

void ControlPanelBase::setdatatype(int data) {
}
int ControlPanelBase::getdatatype() {
	return 0;
}

/**
 * パネル名の設定
 */
void ControlPanelBase::setEditName(LPTSTR data) {
}
/**
 * パネル内ホットスポットの設定
 */
void ControlPanelBase::setHotspotCount(LPTSTR data) {
}
/**
 * フォーカスの設定
 */
void ControlPanelBase::setFocus(void) {
}

/**
 * コマンド発行時の処理を行う。
 * @param uMsg メッセージID(WM_COMMAND)
 * @param wParam 1つ目のパラメータ
 * @param hwndControl コマンドが発生したコントロールのハンドル
 */
LRESULT ControlPanelBase::onCommand(UINT uMsg, WPARAM wParam,
		HWND hwndControl) {
	return 1;
}

/**
 * キー入力時の処理を行う。
 * @param uMsg メッセージID
 * @param wParam 1つ目のパラメータ
 */
LRESULT ControlPanelBase::onKey(UINT uMsg, WPARAM wParam) {
	// (2017/2/9YM)キー押された時の処理追加
	if (uMsg == WM_KEYDOWN) {
		// キーダウンメッセージの時
		if (result != NULL) {
			//　データが読まれていない場合は何もしない
			switch (wParam) {
			int id;

		case VK_LEFT:
		case VK_DOWN:
			//　←↓押したとき
			id = mainWindow->getSelectedId();
			id--;
			if (id < 0) {
				id = result->getDataCount() - 1;
			}
			//　選択中IDをセット
			mainWindow->setSelectedId(id);
			// 表示を更新する
			mainWindow->canvasUpdate();
			break;

		case VK_RIGHT:
		case VK_UP:
			//　↑→押したとき
			id = mainWindow->getSelectedId();
			id++;
			if (id > result->getDataCount() - 1) {
				id = 0;
			}
			//　選択中IDをセット
			mainWindow->setSelectedId(id);
			// 表示を更新する
			mainWindow->canvasUpdate();
			break;
			}
		}
	}
	return 1;
}

/**
 * コントロールの背景色を指定する処理を行う。
 * @param hDC 再描画対象のデバイスコンテキスト
 * @param hWnd 再描画対象のウィンドウハンドル
 * @return 背景色に使用するブラシのハンドル(0はデフォルト)
 */
LRESULT ControlPanelBase::onColorCtrl(HDC hDC, HWND hWnd) {
	return 0;
}
