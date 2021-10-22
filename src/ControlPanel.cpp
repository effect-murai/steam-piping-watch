/*
 * ControlPanel.cpp
 *
 *  Created on: 2016/01/08
 *      Author: PC-EFFECT-011
 */

#include "windows.h"

#include "ControlPanel.h"
#include "MainWindow.h"
#include "ResultData.h"
#include "resource.h"

//------------------------------------------------------------------------------
// External Global Variables
//------------------------------------------------------------------------------
extern ResultData *result;

//------------------------------------------------------------------------------
// ControlPanel Class
//------------------------------------------------------------------------------

ControlPanel::ControlPanel(int canvasWidth, int canvasHeight) {
	this->m_Width = canvasWidth;
	this->m_Height = canvasHeight;
	this->m_ControlMode = ControlPanel::CONTROL_PANEL_MODE_DEFAULT;

	// コントロールパネル
	this->m_Group_ControlPanel = new GroupBox(Resource::getString(IDS_NULL),
	WS_VISIBLE, this->m_Width, 0, 340, 619, MainWindow::getInstance());

	this->m_Label_Navigator = new Label(Resource::getString(IDS_SELECT_FOLDER),
	WS_VISIBLE, 10, 10, 320, 100, this->m_Group_ControlPanel, true);

	// ステータスパネル
	this->m_Group_StatusPanel = new GroupBox(Resource::getString(IDS_NULL),
	WS_VISIBLE, this->m_Width + 340, 0, 170, this->m_Height - 10,
			MainWindow::getInstance());

	this->m_ControlPanelDefault = new ControlPanelDefault(
			this->m_Group_ControlPanel);
	this->m_ControlPanelCamera = new ControlPanelCamera(
			this->m_Group_ControlPanel);
	this->m_ControlPanelShootingData = new ControlPanelShootingData(
			this->m_Group_ControlPanel);
	this->m_ControlPanelArea = new ControlPanelArea(this->m_Group_ControlPanel);
	this->m_ControlPanelHotspot = new ControlPanelHotspot(
			this->m_Group_ControlPanel);
	this->m_ControlPanelPanelSetting = new ControlPanelPanelSetting(
			this->m_Group_ControlPanel);
	this->m_StatusPanel = new StatusPanel(this->m_Group_StatusPanel);

	this->m_ControlPanel = this->m_ControlPanelDefault;

	this->m_ControlPanelCamera->allHide();
	this->m_ControlPanelShootingData->allHide();
	this->m_ControlPanelArea->allHide();
	this->m_ControlPanelHotspot->allHide();
	this->m_ControlPanelPanelSetting->allHide();
}

ControlPanel::~ControlPanel() {
	delete this->m_ControlPanelCamera;
	delete this->m_ControlPanelShootingData;
	delete this->m_ControlPanelArea;
	delete this->m_ControlPanelHotspot;
	delete this->m_ControlPanelPanelSetting;
	delete this->m_ControlPanelDefault;
	delete this->m_StatusPanel;

	delete this->m_Label_Navigator;
	delete this->m_Group_ControlPanel;
	delete this->m_Group_StatusPanel;
}

/**
 * 初期設定
 */
void ControlPanel::init() {
	this->m_ControlPanelDefault->init();
	this->m_ControlPanelCamera->init();
	this->m_ControlPanelShootingData->init();
	this->m_ControlPanelArea->init();
	this->m_ControlPanelHotspot->init();
	this->m_ControlPanelPanelSetting->init();
	this->m_StatusPanel->init();

	/**
	 * 高さが940px未満の場合、MiniPanel用の配置に変更
	 */
	if (MainWindow::getInstance()->IsMiniPanelMode()) {
		// 操作パネルのコントロールを移動
		// (2017/8/9YM)サイズを変更
		this->m_Group_ControlPanel->resize(340, 428);
		this->m_Group_StatusPanel->resize(170, 700);
	}

	// 画面右上のメッセージを設定
	this->setNavigatorText(Resource::getString(IDS_NAVI_DEFAULT));
}

/**
 * フォントを設定する
 * @param[in] hFont フォントのハンドル
 */
void ControlPanel::setFont(HFONT hFont) {
	this->m_Label_Navigator->setFont(hFont);
	this->m_ControlPanelDefault->setFont(hFont);
	this->m_ControlPanelCamera->setFont(hFont);
	this->m_ControlPanelShootingData->setFont(hFont);
	this->m_ControlPanelArea->setFont(hFont);
	this->m_ControlPanelHotspot->setFont(hFont);
	this->m_ControlPanelPanelSetting->setFont(hFont);
	this->m_StatusPanel->setFont(hFont);
}

/**
 * モードを設定する。
 * @param mode 操作モード
 */
void ControlPanel::setMode(int mode) {
	if (this->m_ControlMode != mode) {
		switch (mode) {
		case ControlPanel::CONTROL_PANEL_MODE_CAMERA:
			this->m_ControlPanelDefault->allHide();
			this->m_ControlPanelShootingData->allHide();
			this->m_ControlPanelArea->allHide();
			this->m_ControlPanelHotspot->allHide();
			this->m_ControlPanelPanelSetting->allHide();
			this->m_ControlPanelCamera->allShow();
			this->m_ControlPanel = this->m_ControlPanelCamera;
			this->m_StatusPanel->hidehotspotdisplay(); // (2020/01/14LEE)
			this->setNavigatorText(Resource::getString(IDS_NAVI_CAMERA));
			break;

		case ControlPanel::CONTROL_PANEL_MODE_SHOOTINGDATA:
			this->m_ControlPanelDefault->allHide();
			this->m_ControlPanelCamera->allHide();
			this->m_ControlPanelArea->allHide();
			this->m_ControlPanelHotspot->allHide();
			this->m_ControlPanelPanelSetting->allHide();
			this->m_ControlPanelShootingData->allShow();
			this->m_ControlPanel = this->m_ControlPanelShootingData;
			this->m_StatusPanel->hidehotspotdisplay(); // (2020/01/14LEE)
			this->setNavigatorText(Resource::getString(IDS_NAVI_SHOOTINGDATA));
			break;

		case ControlPanel::CONTROL_PANEL_MODE_AREA:
			this->m_ControlPanelDefault->allHide();
			this->m_ControlPanelCamera->allHide();
			this->m_ControlPanelShootingData->allHide();
			this->m_ControlPanelHotspot->allHide();
			this->m_ControlPanelPanelSetting->allHide();
			this->m_ControlPanelArea->allShow();
			this->m_ControlPanel = this->m_ControlPanelArea;
			this->m_StatusPanel->hidehotspotdisplay(); // (2020/01/14LEE)
			this->setNavigatorText(Resource::getString(IDS_NAVI_AREA));
			break;

		case ControlPanel::CONTROL_PANEL_MODE_HOTSPOT:
			this->m_ControlPanelDefault->allHide();
			this->m_ControlPanelCamera->allHide();
			this->m_ControlPanelShootingData->allHide();
			this->m_ControlPanelArea->allHide();
			this->m_ControlPanelPanelSetting->allHide();
			this->m_ControlPanelHotspot->allShow();
			this->m_ControlPanel = this->m_ControlPanelHotspot;
			this->setNavigatorText(Resource::getString(IDS_NAVI_HOTSPOT));
			break;

		case ControlPanel::CONTROL_PANEL_MODE_PANELSETTING:
			this->m_ControlPanelDefault->allHide();
			this->m_ControlPanelCamera->allHide();
			this->m_ControlPanelShootingData->allHide();
			this->m_ControlPanelArea->allHide();
			this->m_ControlPanelHotspot->allHide();
			this->m_ControlPanelPanelSetting->allShow();
			this->m_ControlPanel = this->m_ControlPanelPanelSetting;
			this->m_StatusPanel->hidehotspotdisplay(); // (2020/01/14LEE)
			this->setNavigatorText(Resource::getString(IDS_NAVI_PANELSETTING));
			break;

		case ControlPanel::CONTROL_PANEL_MODE_DEFAULT:
		default:
			this->m_ControlPanelCamera->allHide();
			this->m_ControlPanelShootingData->allHide();
			this->m_ControlPanelArea->allHide();
			this->m_ControlPanelHotspot->allHide();
			this->m_ControlPanelPanelSetting->allHide();
			this->m_ControlPanelDefault->allShow();
			this->m_StatusPanel->hidehotspotdisplay(); // (2020/01/14LEE)
			this->m_ControlPanel = this->m_ControlPanelDefault;

			if (result->isAvailable()) {
				this->setNavigatorText(Resource::getString(IDS_NAVI_HOME));
			} else {
				this->setNavigatorText(Resource::getString(IDS_NAVI_DEFAULT));
			}
			break;
		}
		// コントロールパネル 表示更新
		RECT RectMain;
		RECT RectPanel;
		MainWindow::getInstance()->getWindowRect(&RectMain);
		this->m_Group_ControlPanel->getWindowRect(&RectPanel);
		RectPanel.left -= RectMain.left;
		RectPanel.top -= RectMain.top;
		RectPanel.right -= RectMain.left;
		RectPanel.bottom -= RectMain.top;
		InvalidateRect(MainWindow::getInstance()->getHandle(), &RectPanel,
		TRUE);

		this->m_ControlMode = mode;
	}
}

/**
 * モードを取得する
 */
int ControlPanel::getMode(void) {
	return this->m_ControlMode;
}

/**
 * 表示画像種別の取得
 */
int ControlPanel::getPictureType() {
	return this->m_ControlPanelDefault->getPictureType();
}

// (2017/4/4YM)赤外線画像タイプ取得関数追加
/**
 * 赤外線画像種別の取得
 */
int ControlPanel::getInfraredType() {
	return this->m_ControlPanelDefault->getInfraredType();
}

// (2017/4/4YM)赤外線画像タイプセット関数追加
/**
 * 赤外線画像種別のセット
 */
void ControlPanel::setInfraredType(int Type) {
	this->m_ControlPanelDefault->setInfraredType(Type);
}

/**
 * 移動値(垂直方向)の取得
 */
double ControlPanel::getMoveV() {
	return this->m_ControlPanel->getMoveV();
}
/**
 * 移動値(垂直方向)の設定
 */
void ControlPanel::setMoveV(double data) {
	this->m_ControlPanel->setMoveV(data);
}

/**
 * 移動値(水平方向)の取得
 */
double ControlPanel::getMoveH() {
	return this->m_ControlPanel->getMoveH();
}
/**
 * 移動値(水平方向)の設定
 */
void ControlPanel::setMoveH(double data) {
	this->m_ControlPanel->setMoveH(data);
}

/**
 * 拡大・縮小値の取得
 */
double ControlPanel::getZoom() {
	return this->m_ControlPanel->getZoom();
}
/**
 * 拡大・縮小値の設定
 */
void ControlPanel::setZoom(double data) {
	this->m_ControlPanel->setZoom(data);
}

/**
 * 拡大
 */
void ControlPanel::zoomIn(void) {
	this->m_ControlPanel->zoomIn();
}

/**
 * 縮小
 */
void ControlPanel::zoomOut(void) {
	this->m_ControlPanel->zoomOut();
}

// (2019/11/06LEE) カメラ特性で全体と個別を区分。
int ControlPanel::getsetingtype() {
	return this->m_ControlPanel->getsetingtype();
}
// (2019/11/08LEE) カメラ特性で全体と個別の設定を読み込み。
void ControlPanel::setdatatype(int data) {
	this->m_ControlPanel->setdatatype(data);
}

int ControlPanel::getdatatype() {
	return this->m_ControlPanel->getdatatype();
}

/**
 * 回転角の取得
 */
double ControlPanel::getTurn() {
	return this->m_ControlPanel->getTurn();
}
/**
 * 回転角の設定
 */
void ControlPanel::setTurn(double data) {
	this->m_ControlPanel->setTurn(data);
}

/**
 * パネル名の設定
 */
void ControlPanel::setEditName(LPTSTR data) {
	this->m_ControlPanel->setEditName(data);
}

/**
 * パネル内ホットスポット数の設定
 */
void ControlPanel::setHotspotCount(LPTSTR data) {
	this->m_ControlPanel->setHotspotCount(data);
}
void ControlPanel::setFocus(void) {
	this->m_ControlPanel->setFocus();
}

/**
 * ナビゲート内容の設定
 */
void ControlPanel::setNavigatorText(LPTSTR sMessage) {
	this->m_Label_Navigator->setText(sMessage);
}

/**
 * 画面右の開始日付を更新
 */
void ControlPanel::setInfoFirstDate(LPTSTR firstDate) {
	this->m_ControlPanelDefault->setInfoFirstDate(firstDate);
}

// (2017/6/1YM)閾値温度を表示
void ControlPanel::setInfoThresholdTemp(LPTSTR threshold) {
	this->m_ControlPanelDefault->setInfoThresholdTemp(threshold);
}

/**
 * 画面右の枚数表示を更新
 */
void ControlPanel::setInfoDataCount(LPTSTR dataCount) {
	this->m_ControlPanelDefault->setInfoDataCount(dataCount);
}

/**
 * 画面右のホットスポット総数表示を更新
 */
void ControlPanel::setInfoHotspotMaxCount(LPTSTR maxCount) {
	this->m_ControlPanelDefault->setInfoHotSpotCount(maxCount);
}

/**
 * 画面右の最高温度表示を更新
 */
void ControlPanel::setInfoTempMax(LPTSTR tempMax) {
	this->m_ControlPanelDefault->setInfoTempMax(tempMax);
}

/**
 * 画面右の平均温度表示を更新
 */
void ControlPanel::setInfoTempAve(LPTSTR tempAve) {
	this->m_ControlPanelDefault->setInfoTempAve(tempAve);
}

/**
 * 温度に対応する色合いのカラー値を取得する。
 * @param temp 温度
 */
COLORREF ControlPanel::getHueColor(double temp) {
	return this->m_StatusPanel->getHueColor(temp);
}

/**
 * 色合い調整の最高温度を取得する。
 */
double ControlPanel::getHueTempMax() {
	return this->m_StatusPanel->getHueTempMax();
}

/**
 * 色合い調整の最高温度を設定する。
 * @param temp 最高温度
 */
void ControlPanel::setHueTempMax(double temp) {
	this->m_StatusPanel->setHueTempMax(temp);
}

/**
 * 色合い調整の最低温度を取得する。
 */
double ControlPanel::getHueTempMin() {
	return this->m_StatusPanel->getHueTempMin();
}

/**
 * 色合い調整の最低温度を設定する。
 * @param temp 最低温度
 */
void ControlPanel::setHueTempMin(double temp) {
	this->m_StatusPanel->setHueTempMin(temp);
}

/**
 * ホットスポット検出ボタンの有効化
 */
void ControlPanel::enableDetectionButton() {
	this->m_StatusPanel->enableDetectionButton();
}

/**
 * 解析範囲設定ボタンの有効化
 */
void ControlPanel::enableAreaButton() {
	this->m_StatusPanel->enableAreaButton();
}

/**
 * 解析範囲設定:クリアボタンの有効化
 */
void ControlPanel::enableAreaClearButton() {
	this->m_ControlPanelArea->enableClearButton();
}

/**
 * panelSettingボタンの有効化
 */
void ControlPanel::enableNoButton() {
	this->m_StatusPanel->enableNoButton();
}

/**
 * CSV出力ボタンの有効化
 */
void ControlPanel::enableCSVButton() {
	this->m_StatusPanel->enableCSVButton();
}

/**
 * 案件情報出力ボタンの有効化
 */
void ControlPanel::enableItemButton() {
	this->m_StatusPanel->enableItemButton();
}

// (2016/12/28YM)ホットスポット自動リンクボタン追加
/**
 * ホットスポット自動リンクボタンの有効化
 */
void ControlPanel::enableAutoLinkButton() {
	this->m_StatusPanel->enableAutoLinkButton();
}

//(2021/10/22YM)テスト　高温部検出ボタン追加
/**
 * 高温部検出・表示ボタンの有効化
 */
void ControlPanel::enableHighTempButton(){
	this->m_StatusPanel->enableHighTempButton();
}

/**
 * ボタンの有効化
 */
void ControlPanel::enableButton() {
	if (this->m_ControlMode == MainWindow::getInstance()->getMode()) {
		this->m_StatusPanel->enableButton();
	}
}

/**
 * ボタンの無効化
 */
void ControlPanel::disableButton() {
	this->m_StatusPanel->disableButton();
}

/**
 * 表示モードの更新
 */
void ControlPanel::updateMode() {
	this->m_ControlPanelDefault->updateMode();
}

/**
 * コントロールの有効化
 */
void ControlPanel::enableControl() {
	this->m_ControlPanel->enableButton();
}

/**
 * コントロールの無効化
 */
void ControlPanel::disableControl() {
	this->m_ControlPanel->disableButton();
}

/**
 * リサイズを行う。
 * @param canvasWidth  幅
 * @param canvasHeight 高さ
 */
void ControlPanel::Resize(int canvasWidth, int canvasHeight) {
	this->m_Width = canvasWidth;
	this->m_Height = canvasHeight;
	// 操作パネルを移動
	this->m_Group_ControlPanel->move(canvasWidth, 0);
	// ステータスパネルを移動
	this->m_Group_StatusPanel->move(canvasWidth + 340, 0);
	this->m_Group_StatusPanel->resize(canvasWidth - 340, canvasHeight - 10);
}

/**
 * コマンド発行時の処理を行う。
 * @param uMsg メッセージID(WM_COMMAND)
 * @param wParam 1つ目のパラメータ
 * @param hwndControl コマンドが発生したコントロールのハンドル
 */
LRESULT ControlPanel::onCommand(UINT uMsg, WPARAM wParam, HWND hwndControl) {
	LRESULT ret;
	ret = this->m_ControlPanel->onCommand(uMsg, wParam, hwndControl);
	ret |= this->m_StatusPanel->onCommand(uMsg, wParam, hwndControl);
	return ret;
}

/**
 * キー入力時の処理を行う。
 * @param uMsg メッセージID
 * @param wParam 1つ目のパラメータ
 */
LRESULT ControlPanel::onKey(UINT uMsg, WPARAM wParam) {
	return this->m_ControlPanel->onKey(uMsg, wParam);
}

/**
 * コントロールの背景色を指定する処理を行う。
 * @param hDC 再描画対象のデバイスコンテキスト
 * @param hWnd 再描画対象のウィンドウハンドル
 * @return 背景色に使用するブラシのハンドル(0はデフォルト)
 */
LRESULT ControlPanel::onColorCtrl(HDC hDC, HWND hWnd) {
	LRESULT ret;

	if (*this->m_Label_Navigator == hWnd) {
		ret = (LRESULT) GetStockObject(WHITE_BRUSH);
	} else {
		ret = this->m_ControlPanel->onColorCtrl(hDC, hWnd);
		if (ret == 0) {
			ret = this->m_StatusPanel->onColorCtrl(hDC, hWnd);
		}
	}
	return ret;
}

// (2020/01/08LEE)
void ControlPanel::sethotspotcount() {
	this->m_StatusPanel->sethotspotcount();
}

//(2020/01/14LEE) ホットスポットモードだけ出力ために追加

void ControlPanel::hidehotspotcount() {
	this->m_StatusPanel->hidehotspotdisplay();
}
