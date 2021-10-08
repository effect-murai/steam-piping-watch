/*
 * StatusPanel.cpp
 *
 *  Created on: 2016/02/19
 *      Author: PC-EFFECT-011
 */

#include "StatusPanel.h"
#include "MainWindow.h"
#include "resource.h"
#include "Resultdata.h"
#include <cstdio>
#include "SubWindows/ItemInfomation.h"
#include "SubWindows/DetectSetting.h"

double casecase;
extern ResultData *result;

StatusPanel::StatusPanel(GroupBox *pWnd) {
	this->m_pWnd = pWnd;

	// ボタン
	this->button_Home = new PushButton(Resource::getString(IDS_PANEL_HOME),
	WS_VISIBLE, 10, 10, 150, 36, this->m_pWnd);
	this->button_SetCamera = new PushButton(
			Resource::getString(IDS_CAMERA_ADJUST_TITLE), WS_VISIBLE, 10, 50,
			150, 36, this->m_pWnd);
	this->button_SetShootingData = new PushButton(
			Resource::getString(IDS_SHOOTING_DATA_ADJUST), WS_VISIBLE, 10, 90,
			150, 36, this->m_pWnd);
	this->button_SetArea = new PushButton(
			//　(2017/8/1YM)解析範囲設定ボタンの位置ずらす
			Resource::getString(IDS_SET_AREA_TITLE), WS_VISIBLE, 10, 170, 150,
			36, this->m_pWnd);
	this->button_Detection = new PushButton(
			//　(2017/8/1YM)ホットスポット解析ボタンの位置ずらす
			Resource::getString(IDS_HOTSPOT_DETECTION), WS_VISIBLE, 10, 210,
			150, 36, this->m_pWnd);
	this->button_SetHotspot = new PushButton(
			//　(2017/7/7YM)ホットスポット補正ボタンの位置ずらす
			Resource::getString(IDS_HOTSPOT_ADJUST_TITLE), WS_VISIBLE, 10, 250,
			150, 36, this->m_pWnd);
	this->button_SetNo = new PushButton(
			// (2017/7/7YM)管理番号ボタンの位置ずらす
			Resource::getString(IDS_SET_NO_TITLE), WS_VISIBLE, 10, 290, 150, 36,
			this->m_pWnd);
	this->button_Output = new PushButton(
			// (2017/7/7YM)報告書出力ボタンの位置ずらす
			Resource::getString(IDS_OUTPUT_TITLE), WS_VISIBLE, 10, 330, 150, 36,
			this->m_pWnd);
	this->button_itemOutput = new PushButton(
			// (2017/7/7YM)案件出力ボタンの位置ずらす
			Resource::getString(IDS_ITEM_OUTPUT_TITLE), WS_VISIBLE, 10, 370,
			150, 36, this->m_pWnd);
	// (2016/12/28YM)ホットスポット自動リンクボタン追加
	this->button_hotspotlink = new PushButton(
			// (2017/8/1YM)特徴点検出ボタンの位置ずらす
			Resource::getString(IDS_HOTSPOT_AUTOLINK), WS_VISIBLE, 10, 130, 150,
			36, this->m_pWnd);

	// 色合い調整
	// (2016/12/28YM)ホットスポット自動リンクボタン追加に伴いY座標を下にずらす
	this->m_ColorPanel = new ColorPanel(
			Resource::getString(IDS_STATUS_HUE_TITLE), 10, 410, this->m_pWnd);

	// (2020/01/08LEE) HOMEで表示されているホットスポット総数を表示
	this->hotspotdisplay = new Label(
			Resource::getString(IDS_STATUS_HOTSPOTCOUNT), WS_VISIBLE, 80, 410,
			70, 40, this->m_pWnd);
	this->edit_hotspotdisplay = new InputBox(TEXT("0"),
	WS_VISIBLE | ES_CENTER | ES_READONLY, 80, 450, 60, 30, this->m_pWnd);
	this->Displaymask = new Label(
			Resource::getString(IDS_PANEL_INFO_DISPLAYMASK), WS_VISIBLE, 80,
			410, 70, 80, this->m_pWnd);

	// インジケータ
	this->m_Label_Home = new Label(Resource::getString(IDS_NULL), WS_VISIBLE, 1,
			12, 9, 32, this->m_pWnd);
	this->m_Label_Camera = new Label(Resource::getString(IDS_NULL), WS_VISIBLE,
			1, 52, 9, 32, this->m_pWnd);
	this->m_Label_ShootingData = new Label(Resource::getString(IDS_NULL),
	WS_VISIBLE, 1, 92, 9, 32, this->m_pWnd);
	this->m_Label_Area = new Label(Resource::getString(IDS_NULL), WS_VISIBLE, 1,
			172, 9, 32, this->m_pWnd);		// (2017/8/2YM)インジケーター座標変更(132→172)
	this->m_Label_Hotspot = new Label(Resource::getString(IDS_NULL), WS_VISIBLE,
			1, 252, 9, 32, this->m_pWnd);	// (2017/7/11YM)インジケーター座標変更(212→252)
	this->m_Label_No = new Label(Resource::getString(IDS_NULL), WS_VISIBLE,	// (2017/7/11YM)インジケーター座標変更(252→292)
			1, 292, 9, 32, this->m_pWnd);

	// ブラシ
	this->m_Brush = CreateSolidBrush(RGB(0, 0, 255));
}

StatusPanel::~StatusPanel() {
	delete this->button_Home;
	delete this->button_SetCamera;
	delete this->button_SetShootingData;
	delete this->button_Detection;
	delete this->button_SetArea;
	delete this->button_SetHotspot;
	delete this->button_SetNo;
	delete this->button_Output;
	delete this->button_itemOutput;
	// (2016/12/28YM)ホットスポット自動リンクボタン追加
	delete this->button_hotspotlink;

	delete this->m_ColorPanel;

	delete this->m_Label_Home;
	delete this->m_Label_Camera;
	delete this->m_Label_ShootingData;
	delete this->m_Label_Area;
	delete this->m_Label_Hotspot;
	delete this->m_Label_No;

	delete this->edit_hotspotdisplay;
	delete this->hotspotdisplay;
	delete this->Displaymask;
	DeleteObject(this->m_Brush);
}

/**
 * 初期設定
 */
void StatusPanel::init() {

	hidehotspotdisplay();

	// ボタンを無効化する
	this->button_SetCamera->disable();
	this->button_SetShootingData->disable();
	this->button_Detection->disable();
	this->button_SetArea->disable();
	this->button_SetHotspot->disable();
	this->button_SetNo->disable();
	this->button_Output->disable();
	this->button_itemOutput->disable();
	// (2016/12/28YM)ホットスポット自動リンクボタン追加
	this->button_hotspotlink->disable();
}

/**
 * フォントを設定する。
 * @param hFont フォント
 */
void StatusPanel::setFont(HFONT hFont) {
	this->button_Home->setFont(hFont);
	this->button_SetCamera->setFont(hFont);
	this->button_SetShootingData->setFont(hFont);
	this->button_Detection->setFont(hFont);
	this->button_SetArea->setFont(hFont);
	this->button_SetHotspot->setFont(hFont);
	this->button_SetNo->setFont(hFont);
	this->button_Output->setFont(hFont);
	this->button_itemOutput->setFont(hFont);
	// (2016/12/28YM)ホットスポット自動リンクボタン追加
	this->button_hotspotlink->setFont(hFont);

	this->edit_hotspotdisplay->setFont(hFont);
	this->hotspotdisplay->setFont(hFont);
	this->Displaymask->setFont(hFont);

	this->m_ColorPanel->setFont(hFont);
}

/**
 * ホットスポット検出ボタンの有効化
 */
void StatusPanel::enableDetectionButton() {
	this->button_Detection->enable();
}

/**
 * 解析範囲設定ボタンの有効化
 */
void StatusPanel::enableAreaButton() {
	this->button_SetArea->enable();
}

/**
 * 管理番号入力ボタンの有効化
 */
void StatusPanel::enableNoButton() {
	this->button_SetNo->enable();
}

/**
 * CSV出力ボタンの有効化
 */
void StatusPanel::enableCSVButton() {
	this->button_Output->enable();
}

/**
 * 案件情報出力ボタンの有効化
 */
void StatusPanel::enableItemButton() {
	this->button_itemOutput->enable();
}

// (2016/12/28YM)ホットスポット自動リンクボタンの有効化
/**
 * ホットスポット自動リンクボタンの有効化
 */
void StatusPanel::enableAutoLinkButton() {
	this->button_hotspotlink->enable();
}

/**
 * ボタンの有効化
 */
void StatusPanel::enableButton() {
	if (result->hasVisible()) {
		// 可視光画像がある場合はカメラ特性補正を有効化する
		this->button_SetCamera->enable();
	}

	this->button_SetShootingData->enable();
	this->button_SetHotspot->enable();
	this->button_SetNo->enable();

}

/**
 * ボタンの無効化
 */
void StatusPanel::disableButton() {
	this->button_SetCamera->disable();
	this->button_SetShootingData->disable();
	this->button_SetHotspot->disable();
}

void StatusPanel::sethotspotcount() {

	int count = result->getHotspotSum();
	this->edit_hotspotdisplay->setTextAsInt(count);

}

/**
 * モードを設定する。
 * @param mode 操作モード
 */
void StatusPanel::setMode(int mode) {
	int controlPanelMode = ControlPanel::CONTROL_PANEL_MODE_DEFAULT;

	// 設定値を保存
	MainWindow::getInstance()->resultDataSave();
	// 設定状態のクリア
	MainWindow::getInstance()->panelSettingData->clearPointCount();

	// メインウィンドウ：モード設定
	MainWindow::getInstance()->setMode(mode);

	// コントロールパネル：モード設定
	switch (mode) {
	case MainWindow::CONTROL_PANEL_MODE_DEFAULT:
		controlPanelMode = ControlPanel::CONTROL_PANEL_MODE_DEFAULT;
		break;

	case MainWindow::CONTROL_PANEL_MODE_CAMERA:
		controlPanelMode = ControlPanel::CONTROL_PANEL_MODE_CAMERA;
		break;

	case MainWindow::CONTROL_PANEL_MODE_SHOOTINGDATA:
		controlPanelMode = ControlPanel::CONTROL_PANEL_MODE_SHOOTINGDATA;
		break;

	case MainWindow::CONTROL_PANEL_MODE_AREA:
		controlPanelMode = ControlPanel::CONTROL_PANEL_MODE_AREA;
		break;

	case MainWindow::CONTROL_PANEL_MODE_HOTSPOT:
		controlPanelMode = ControlPanel::CONTROL_PANEL_MODE_HOTSPOT;
		break;

	case MainWindow::CONTROL_PANEL_MODE_PANELSETTING:
		controlPanelMode = ControlPanel::CONTROL_PANEL_MODE_PANELSETTING;
		break;

	default:
		break;
	}
	MainWindow::getInstance()->setControlPanel(controlPanelMode);

	// インジケータ 表示更新
	this->m_Label_Home->refresh();
	this->m_Label_Camera->refresh();
	this->m_Label_ShootingData->refresh();
	this->m_Label_Area->refresh();
	this->m_Label_Hotspot->refresh();
	this->m_Label_No->refresh();

}

/**
 * 温度に対応する色合いのカラー値を取得する。
 * @param temp 温度
 */
COLORREF StatusPanel::getHueColor(double temp) {
	return this->m_ColorPanel->getColor(temp);
}

/**
 * 色合い調整の最高温度を取得する。
 */
double StatusPanel::getHueTempMax() {
	return this->m_ColorPanel->getTempMax();
}

/**
 * 色合い調整の最高温度を設定する。
 * @param temp 最高温度
 */
void StatusPanel::setHueTempMax(double temp) {
	this->m_ColorPanel->setTempMax(temp);
}

/**
 * 色合い調整の最低温度を取得する。
 */
double StatusPanel::getHueTempMin() {
	return this->m_ColorPanel->getTempMin();
}

/**
 * 色合い調整の最低温度を設定する。
 * @param temp 最低温度
 */
void StatusPanel::setHueTempMin(double temp) {
	this->m_ColorPanel->setTempMin(temp);
}

//(2020/01/14LEE) ホットスポットモードだけ出力ために追加
void StatusPanel::showhotspotdisplay() {
	this->hotspotdisplay->show();
	this->edit_hotspotdisplay->show();
	this->Displaymask->hide();
}

void StatusPanel::hidehotspotdisplay() {
	this->hotspotdisplay->hide();
	this->edit_hotspotdisplay->hide();
	this->Displaymask->show();
}

/**
 * コマンド発行時の処理を行う。
 * @param uMsg メッセージID(WM_COMMAND)
 * @param wParam 1つ目のパラメータ
 * @param hwndControl コマンドが発生したコントロールのハンドル
 */
LRESULT StatusPanel::onCommand(UINT uMsg, WPARAM wParam, HWND hwndControl) {
	switch (HIWORD(wParam)) {
	case BN_CLICKED:
		if (hwndControl == *button_Home) {
			this->setMode(MainWindow::CONTROL_PANEL_MODE_DEFAULT);
		} else if (hwndControl == *button_SetCamera) {
			hidehotspotdisplay();
			this->setMode(MainWindow::CONTROL_PANEL_MODE_CAMERA);
		} else if (hwndControl == *button_SetShootingData) {
			hidehotspotdisplay();
			this->setMode(MainWindow::CONTROL_PANEL_MODE_SHOOTINGDATA);
		} else if (hwndControl == *button_Detection) {
			MainWindow::getInstance()->detectHotspot(true);
		} else if (hwndControl == *button_SetArea) {
			hidehotspotdisplay();
			this->setMode(MainWindow::CONTROL_PANEL_MODE_AREA);
		} else if (hwndControl == *button_SetHotspot) {
			showhotspotdisplay();
			this->setMode(MainWindow::CONTROL_PANEL_MODE_HOTSPOT);
		} else if (hwndControl == *button_SetNo) {
			hidehotspotdisplay();
			this->setMode(MainWindow::CONTROL_PANEL_MODE_PANELSETTING);
		} else if (hwndControl == *button_Output) {
			MainWindow::getInstance()->outputCsv();
		} else if (hwndControl == *button_itemOutput) {
			ItemInfomationDialog::create(hwndControl);
		} else if (hwndControl == *button_hotspotlink) // (2017/2/2YM)ホットスポット自動リンクボタンクリック時処理追加
				{
			MainWindow::getInstance()->HotspotAutoLink();
		}
		break;

	case EN_CHANGE:
		if (hwndControl == *m_ColorPanel) {
			MainWindow::getInstance()->createTempPictures();
		}
		break;

	default:
		break;
	}

	return 0;
}

/**
 * コントロールの背景色を指定する処理を行う。
 * @param hDC 再描画対象のデバイスコンテキスト
 * @param hWnd 再描画対象のウィンドウハンドル
 * @return 背景色に使用するブラシのハンドル(0はデフォルト)
 */
LRESULT StatusPanel::onColorCtrl(HDC hDC, HWND hWnd) {
	LRESULT ret = 0;
	int mode = MainWindow::getInstance()->getMode();

	if ((hWnd == *this->m_Label_Home)
			&& (mode == MainWindow::CONTROL_PANEL_MODE_DEFAULT)) {
		ret = (INT_PTR) this->m_Brush;
	} else if ((hWnd == *this->m_Label_Camera)
			&& (mode == MainWindow::CONTROL_PANEL_MODE_CAMERA)) {
		ret = (INT_PTR) this->m_Brush;
	} else if ((hWnd == *this->m_Label_ShootingData)
			&& (mode == MainWindow::CONTROL_PANEL_MODE_SHOOTINGDATA)) {
		ret = (INT_PTR) this->m_Brush;
	} else if ((hWnd == *this->m_Label_Area)
			&& (mode == MainWindow::CONTROL_PANEL_MODE_AREA)) {
		ret = (INT_PTR) this->m_Brush;
	} else if ((hWnd == *this->m_Label_Hotspot)
			&& (mode == MainWindow::CONTROL_PANEL_MODE_HOTSPOT)) {
		ret = (INT_PTR) this->m_Brush;
	} else if ((hWnd == *this->m_Label_No)
			&& (mode == MainWindow::CONTROL_PANEL_MODE_PANELSETTING)) {
		ret = (INT_PTR) this->m_Brush;
	}

	return ret;
}
