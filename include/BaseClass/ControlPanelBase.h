/*
 * ControlPanelBase.h
 *
 *  Created on: 2016/01/26
 *      Author: PC-EFFECT-011
 */

#ifndef CONTROLPANELBASE_H_
#define CONTROLPANELBASE_H_

#include "Controls.h"

class ControlPanelBase {
public:
	ControlPanelBase();
	virtual ~ControlPanelBase();

	/**
	 * 初期設定
	 */
	virtual void init() = 0;

	/**
	 * フォントを設定する。
	 * @param hFont フォント
	 */
	virtual void setFont(HFONT hFont) = 0;

	/**
	 * 全てのコントロールの表示
	 */
	virtual void allShow() = 0;

	/**
	 * 全てのコントロールの非表示
	 */
	virtual void allHide() = 0;

	/**
	 * 移動値(垂直方向)の取得
	 */
	virtual double getMoveV();
	/**
	 * 移動値(垂直方向)の設定
	 */
	virtual void setMoveV(double data);

	/**
	 * 移動値(水平方向)の取得
	 */
	virtual double getMoveH();
	/**
	 * 移動値(水平方向)の設定
	 */
	virtual void setMoveH(double data);

	/**
	 * 拡大・縮小値の取得
	 */
	virtual double getZoom();
	/**
	 * 拡大・縮小値の設定
	 */
	virtual void setZoom(double data);

	// 拡大縮小
	virtual void zoomIn();
	virtual void zoomOut();

	/**
	 * 回転角の取得
	 */
	virtual double getTurn();
	/**
	 * 回転角の設定
	 */
	virtual void setTurn(double data);

	/**
	 * パネル名の設定
	 */
	virtual void setEditName(LPTSTR data);

	/**
	 * パネル名の設定
	 */
	virtual void setHotspotCount(LPTSTR data);

	/**
	 * フォーカスの設定
	 */
	virtual void setFocus();

	/**
	 * ボタンの有効化
	 */
	virtual void enableButton() = 0;

	/**
	 * ボタンの無効化
	 */
	virtual void disableButton() = 0;

	/*
	 *  (2019/11/06LEE) カメラ特性で全体と個別を区分。
	 */
	virtual int getsetingtype();

	/*
	 *  (2019/11/08LEE) カメラ特性で全体と個別の設定を読み込み。
	 */

	virtual int getdatatype();
	virtual void setdatatype(int data);

	/**
	 * コマンド発行時の処理を行う。
	 * @param uMsg メッセージID(WM_COMMAND)
	 * @param wParam 1つ目のパラメータ
	 * @param hwndControl コマンドが発生したコントロールのハンドル
	 */
	virtual LRESULT onCommand(UINT uMsg, WPARAM wParam, HWND hwndControl);

	/**
	 * キー入力時の処理を行う。
	 * @param uMsg メッセージID
	 * @param wParam 1つ目のパラメータ
	 */
	virtual LRESULT onKey(UINT uMsg, WPARAM wParam);

	/**
	 * コントロールの背景色を指定する処理を行う。
	 * @param hDC 再描画対象のデバイスコンテキスト
	 * @param hWnd 再描画対象のウィンドウハンドル
	 * @return 背景色に使用するブラシのハンドル(0はデフォルト)
	 */
	virtual LRESULT onColorCtrl(HDC hDC, HWND hWnd);

public:
	// 初期値
	static const double DEFAULT_VALUE_MOVE = 0;
	static const double DEFAULT_VALUE_TURN = 0;
	static const double DEFAULT_VALUE_ZOOM = 1;
	static const int DEFAULT_VALUE_DATA = 0;              // (2019/11/08LEE) 追加
	// 増減単位
	static const double SCALE_VALUE_MOVE = 1;
	static const double SCALE_VALUE_TURN = 0.25L;
	static const double SCALE_VALUE_ZOOM = 0.01L;
	// 最小値
	static const double MIN_VALUE_ZOOM = 0.01L;
	static const double MIN_VALUE_HEIGHT = 1.00L;
	// 最大値
	static const double MAX_VALUE_ZOOM = 5.00L;
	static const double MAX_VALUE_HEIGHT = 150.0L;

protected:
	//--------------------------------------------------------
	// コントロールパネルウィンドウ
	//--------------------------------------------------------
	GroupBox *m_pWnd;

	//--------------------------------------------------------
	// 内部保存データ
	//--------------------------------------------------------
	// 移動値(垂直方向)
	double m_MoveV;
	// 移動値(水平方向)
	double m_MoveH;
	// 拡大・縮小値
	double m_Zoom;
	// 回転角
	double m_Turn;
	// コントロールの有効／無効
	bool m_Enabled;
	// (2019/11/08LEE) カメラ特性で全体と個別の中でデータtypeを設定を表示。
	int m_datatype;

};

#endif /* CONTROLPANELBASE_H_ */
