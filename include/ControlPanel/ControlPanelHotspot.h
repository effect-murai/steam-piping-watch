/*
 * ControlPanelHotspot.h
 *
 *  Created on: 2016/01/12
 *      Author: PC-EFFECT-011
 */

#ifndef CONTROLPANELHOTSPOT_H_
#define CONTROLPANELHOTSPOT_H_

#include <windows.h>

#include "ControlPanelBase.h"

class ControlPanelHotspot: public ControlPanelBase {
public:
	ControlPanelHotspot(GroupBox *pWnd);
	~ControlPanelHotspot();

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

	/** タイトルラベル */
	Label *label_Title;
	/** 説明ラベル */
	Label *label_Info;
	/** 説明ラベル
	 * (2019/12/20LEE) ホットスポット補正の部分を追加
	 *  */
	Label *label_Info2;

	// (2017/6/14YM)ホットスポット削除ボタンを追加
	PushButton *button_HSDel;
	// (2017/6/14YM)特徴点削除ボタンを追加
	PushButton *button_KPDel;

};

#endif /* CONTROLPANELHOTSPOT_H_ */
