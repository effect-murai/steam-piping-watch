/*
 * ControlPanelArea.h
 *
 *  Created on: 2016/02/29
 *      Author: PC-EFFECT-011
 */

#ifndef CONTROLPANELAREA_H_
#define CONTROLPANELAREA_H_

#include <windows.h>

#include "ControlPanelBase.h"

class ControlPanelArea: public ControlPanelBase {
public:
	ControlPanelArea(GroupBox *pWnd);
	~ControlPanelArea();

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
	 * クリアボタンの有効化
	 */
	void enableClearButton();

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

private:
	//--------------------------------------------------------
	// 操作パネル
	//--------------------------------------------------------
	/** クリアボタン */
	PushButton *button_Clear;

	/** タイトルラベル */
	Label *label_Title;
	/** 説明ラベル */
	Label *label_Info;

};

#endif /* CONTROLPANELAREA_H_ */
