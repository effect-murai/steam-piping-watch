/*
 * StatusPanel.h
 *
 *  Created on: 2016/02/19
 *      Author: PC-EFFECT-011
 */

// (2016/12/28YM)ホットスポット自動リンクボタン追加
#ifndef STATUSPANEL_H_
#define STATUSPANEL_H_

#include <windows.h>

#include "Controls.h"

class StatusPanel {
public:
	StatusPanel(GroupBox *pWnd);
	~StatusPanel();

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
	 * ホットスポット検出ボタンの有効化
	 */
	void enableDetectionButton();

	/**
	 * 解析範囲設定ボタンの有効化
	 */
	void enableAreaButton();

	/**
	 * 管理番号入力ボタンの有効化
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

	/**
	 * モードを設定する。
	 * @param mode 操作モード
	 */
	void setMode(int mode);

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

	void sethotspotcount();
	//(2020/01/14LEE) ホットスポットモードだけ出力ために追加
	void showhotspotdisplay();
	void hidehotspotdisplay();

	/**
	 * コマンド発行時の処理を行う。
	 * @param uMsg メッセージID(WM_COMMAND)
	 * @param wParam 1つ目のパラメータ
	 * @param hwndControl コマンドが発生したコントロールのハンドル
	 */
	LRESULT onCommand(UINT uMsg, WPARAM wParam, HWND hwndControl);

	/**
	 * コントロールの背景色を指定する処理を行う。
	 * @param hDC 再描画対象のデバイスコンテキスト
	 * @param hWnd 再描画対象のウィンドウハンドル
	 * @return 背景色に使用するブラシのハンドル(0はデフォルト)
	 */
	LRESULT onColorCtrl(HDC hDC, HWND hWnd);

private:
	//--------------------------------------------------------
	// ステータスパネルウィンドウ
	//--------------------------------------------------------
	GroupBox *m_pWnd;

	// Homeボタン
	PushButton *button_Home;
	// 画像補正(カメラ特性)ボタン
	PushButton *button_SetCamera;
	// 画像補正(回転、高度)ボタン
	PushButton *button_SetShootingData;
	// ホットスポット解析ボタン
	PushButton *button_Detection;
	// 解析範囲設定ボタン
	PushButton *button_SetArea;
	// ホットスポット補正ボタン
	PushButton *button_SetHotspot;
	// 管理番号入力ボタン
	PushButton *button_SetNo;
	// CSV出力ボタン
	PushButton *button_Output;
	// 案件情報出力ボタン
	PushButton *button_itemOutput;
	// (2016/12/28YM)ホットスポット自動リンクボタン追加
	PushButton *button_hotspotlink;
	// 色合い調整
	ColorPanel *m_ColorPanel;

	// インジケータ
	Label *m_Label_Home;
	Label *m_Label_Camera;
	Label *m_Label_ShootingData;
	Label *m_Label_Area;
	Label *m_Label_Hotspot;
	Label *m_Label_No;

	// (2020/01/08LEE) HOMEで表示されているホットスポット総数を表示
	Label *hotspotdisplay;
	EditBox *edit_hotspotdisplay;
	// (2020/01/15LEE) LabelとEditBoxをHideするために追加
	Label *Displaymask;

	// ブラシ
	HBRUSH m_Brush;
};

#endif /* STATUSPANEL_H_ */
