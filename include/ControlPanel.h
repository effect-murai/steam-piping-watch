/*
 * ControlPanel.h
 *
 *  Created on: 2016/01/08
 *      Author: PC-EFFECT-011
 */

#ifndef CONTROLPANEL_H_
#define CONTROLPANEL_H_

#include <windows.h>

#include "ControlPanelDefault.h"
#include "ControlPanelCamera.h"
#include "ControlPanelShootingData.h"
#include "ControlPanelArea.h"
#include "ControlPanelHotspot.h"
#include "ControlPanelPanelSetting.h"
#include "StatusPanel.h"

class ControlPanel {
public:
	ControlPanel(int canvasWidth, int canvasHeight);
	~ControlPanel();

	/**
	 * 初期設定
	 */
	void init();

	/**
	 * フォントを設定する。
	 * @param hFont フォント
	 */
	void setFont(HFONT hFont);

	/**
	 * 全てのコントロールの表示
	 */
	void allShow();

	/**
	 * 全てのコントロールの非表示
	 */
	void allHide();

	/**
	 * 表示画像種別の取得
	 */
	int getPictureType();

	// (2017/4/4YM)赤外線画像タイプ取得関数追加
	/**
	 * 赤外線画像種別の取得
	 */
	int getInfraredType();

	// (2017/4/4YM)赤外線画像タイプセット関数追加
	/**
	 * 赤外線画像種別のセット
	 */
	void setInfraredType(int Type);

	/**
	 * 移動値(垂直方向)の取得
	 */
	double getMoveV();
	/**
	 * 移動値(垂直方向)の設定
	 */
	void setMoveV(double data);

	/**
	 * 移動値(水平方向)の取得
	 */
	double getMoveH();
	/**
	 * 移動値(水平方向)の設定
	 */
	void setMoveH(double data);

	/**
	 * 拡大・縮小値の取得
	 */
	double getZoom();
	/**
	 * 拡大・縮小値の設定
	 */
	void setZoom(double data);

	// 拡大縮小
	void zoomIn(void);
	void zoomOut(void);

	// (2019/11/06LEE) カメラ特性で全体と個別を区分。
	int getsetingtype();

	// (2019/11/08LEE) カメラ特性で全体と個別の設定を読み込み。
	void setdatatype(int data);

	// (2019/11/08LEE) カメラ特性で全体と個別の設定を読み込み。
	int getdatatype();

	/**
	 * 回転角の取得
	 */
	double getTurn();
	/**
	 * 回転角の設定
	 */
	void setTurn(double data);
	/**
	 * パネル名の設定
	 */
	void setEditName(LPTSTR data);
	/**
	 * パネル内ホットスポット数の設定
	 */
	void setHotspotCount(LPTSTR data);

	/**
	 * ホットスポット検出ボタンの有効化
	 */
	void enableDetectionButton();

	/**
	 * 解析範囲設定ボタンの有効化
	 */
	void enableAreaButton();

	/**
	 * 解析範囲設定:クリアボタンの有効化
	 */
	void enableAreaClearButton();

	/**
	 * ホットスポット検出ボタンの有効化
	 */
	void enableNoButton();

	/**
	 * CSV出力ボタンの有効化
	 */
	void enableCSVButton();

	/**
	 * 案件情報出力ボタンの有効化
	 */
	void enableItemButton();

	// (2016/12/28YM)ホットスポット自動リンクボタン追加
	/**
	 * ホットスポット自動リンクボタンの有効化
	 */
	void enableAutoLinkButton();

	/**
	 * ボタンの有効化
	 */
	void enableButton();

	/**
	 * ボタンの無効化
	 */
	void disableButton();

	void updateMode();

	/**
	 * コントロールの有効化
	 */
	void enableControl();

	/**
	 * コントロールの無効化
	 */
	void disableControl();

	/**
	 * 画面右の開始日付を更新
	 */
	void setInfoFirstDate(LPTSTR firstDate);

// (2017/6/1YM)閾値温度を表示
	void setInfoThresholdTemp(LPTSTR threshold);

	/**
	 * 画面右の枚数表示を更新
	 */
	void setInfoDataCount(LPTSTR dataCount);

	/**
	 * 画面右のホットスポット総数表示を更新
	 */
	void setInfoHotspotMaxCount(LPTSTR maxCount);

	/**
	 * 画面右の最高温度表示を更新
	 */
	void setInfoTempMax(LPTSTR tempMax);

	/**
	 * 画面右の平均温度表示を更新
	 */
	void setInfoTempAve(LPTSTR tempAve);

	/**
	 * 温度に対応する色合いのカラー値を取得する。
	 * @param temp 温度
	 */
	COLORREF getHueColor(double temp);

	/**
	 * 色合い調整の最高温度を取得する。
	 */
	double getHueTempMax();

	/**
	 * 色合い調整の最高温度を設定する。
	 * @param temp 最高温度
	 */
	void setHueTempMax(double temp);

	/**
	 * 色合い調整の最低温度を取得する。
	 */
	double getHueTempMin();

	/**
	 * 色合い調整の最低温度を設定する。
	 * @param temp 最低温度
	 */
	void setHueTempMin(double temp);

	// (2020/01/08LEE)
	void sethotspotcount();

	//(2020/01/14LEE) ホットスポットモードだけ出力ために追加
	void hidehotspotcount();

	/**
	 * リサイズを行う。
	 * @param canvasWidth  幅
	 * @param canvasHeight 高さ
	 */
	void Resize(int canvasWidth, int canvasHeight);

	/**
	 * フォーカスの設定
	 */
	void setFocus();

	/**
	 * コマンド発行時の処理を行う。
	 * @param uMsg メッセージID(WM_COMMAND)
	 * @param wParam 1つ目のパラメータ
	 * @param hwndControl コマンドが発生したコントロールのハンドル
	 */
	LRESULT onCommand(UINT uMsg, WPARAM wParam, HWND hwndControl);

	/**
	 * キー入力時の処理を行う。
	 * @param uMsg メッセージID
	 * @param wParam 1つ目のパラメータ
	 */
	LRESULT onKey(UINT uMsg, WPARAM wParam);

	/**
	 * コントロールの背景色を指定する処理を行う。
	 * @param hDC 再描画対象のデバイスコンテキスト
	 * @param hWnd 再描画対象のウィンドウハンドル
	 * @return 背景色に使用するブラシのハンドル(0はデフォルト)
	 */
	LRESULT onColorCtrl(HDC hDC, HWND hWnd);

public:
	/**
	 * モードを設定する。
	 * @param mode 操作モード
	 */
	void setMode(int mode);

	/**
	 * モードを取得する。
	 * @return 操作モード
	 */
	int getMode(void);

	/**
	 * ナビゲート内容の設定
	 */
	void setNavigatorText(LPTSTR sMessage);

public:
	enum {
		CONTROL_PANEL_MODE_DEFAULT = 0,
		CONTROL_PANEL_MODE_CAMERA,
		CONTROL_PANEL_MODE_SHOOTINGDATA,
		CONTROL_PANEL_MODE_AREA,
		CONTROL_PANEL_MODE_HOTSPOT,
		CONTROL_PANEL_MODE_PANELSETTING
	};

private:
	//--------------------------------------------------------
	// 操作パネル(共通)
	//--------------------------------------------------------
	ControlPanelBase *m_ControlPanel;

	// コントロールパネルウィンドウ
	GroupBox *m_Group_ControlPanel;
	// ナビゲーター
	Label *m_Label_Navigator;
	// ステータスパネルウィンドウ
	GroupBox *m_Group_StatusPanel;

	//--------------------------------------------------------
	// 操作パネル(標準)
	//--------------------------------------------------------
	ControlPanelDefault *m_ControlPanelDefault;

	//--------------------------------------------------------
	// 操作パネル(画像補正 カメラ特性)
	//--------------------------------------------------------
	ControlPanelCamera *m_ControlPanelCamera;

	//--------------------------------------------------------
	// 操作パネル(画像補正 回転、高度)
	//--------------------------------------------------------
	ControlPanelShootingData *m_ControlPanelShootingData;

	//--------------------------------------------------------
	// 操作パネル(解析範囲設定)
	//--------------------------------------------------------
	ControlPanelArea *m_ControlPanelArea;

	//--------------------------------------------------------
	// 操作パネル(ホットスポット補正)
	//--------------------------------------------------------
	ControlPanelHotspot *m_ControlPanelHotspot;

	//--------------------------------------------------------
	// 操作パネル(PanelSetting)
	//--------------------------------------------------------
	ControlPanelPanelSetting *m_ControlPanelPanelSetting;

	//--------------------------------------------------------
	// ステータスパネル
	//--------------------------------------------------------
	StatusPanel *m_StatusPanel;

	//--------------------------------------------------------
	// 内部保存データ
	//--------------------------------------------------------
	// 幅
	int m_Width;
	// 高さ
	int m_Height;
	// 操作モード
	int m_ControlMode;

};

#endif /* CONTROLPANEL_H_ */
