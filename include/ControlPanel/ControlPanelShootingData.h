/*
 * ControlPanelShootingData.h
 *
 *  Created on: 2016/01/12
 *      Author: PC-EFFECT-011
 */

// (2017/4/3YM追加)『角度一括適用』ボタンを追加
#ifndef CONTROLPANELSHOOTINGDATA_H_
#define CONTROLPANELSHOOTINGDATA_H_

#include <windows.h>

#include "ControlPanelBase.h"

class ControlPanelShootingData: public ControlPanelBase {
public:
	ControlPanelShootingData(GroupBox *pWnd);
	~ControlPanelShootingData();

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
	 * 拡大・縮小値の取得
	 */
	double getZoom();
	/**
	 * 拡大・縮小値の設定
	 */
	void setZoom(double data);

	void zoomIn(void);
	void zoomOut(void);

	/**
	 * 回転角の取得
	 */
	double getTurn();
	/**
	 * 回転角の設定
	 */
	void setTurn(double data);

	// 左回転(反時計回り)
	void turnLeft(void);
	// 右回転(時計回り)
	void turnRight(void);

	/**
	 * ボタンの有効化
	 */
	void enableButton();

	/**
	 * ボタンの無効化
	 */
	void disableButton();

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

private:
	//--------------------------------------------------------
	// 操作パネル
	//--------------------------------------------------------

	/** 前画像ボタン */
	PushButton *button_Prev;
	/** 次画像ボタン */
	PushButton *button_Next;

	/** 拡大ボタン */
	PushButton *button_ZoomIn;
	/** 拡大・縮小 入力テキストボックス */
	EditBox *edit_Zoom;
	/** 縮小ボタン */
	PushButton *button_ZoomOut;
	/** 拡大・縮小 一括適用ボタン */
	PushButton *button_ZoomAll;

	/** 回転←ボタン */
	PushButton *button_TurnLeft;
	/** 回転 入力テキストボックス */
	EditBox *edit_Turn;
	/** 回転→ボタン */
	PushButton *button_TurnRight;

	// (2017/4/3YM追加)『角度一括適用』ボタン追加
	/** 角度 一括適用ボタン */
	PushButton *button_TurnAll;

	/** タイトルラベル */
	Label *label_Title;
	/** 説明ラベル */
	Label *label_Info;

	HFONT miniFont;

};

#endif /* CONTROLPANELSHOOTINGDATA_H_ */
