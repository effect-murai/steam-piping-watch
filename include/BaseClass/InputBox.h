/*
 * InputBox.h
 *
 *  Created on: 2016/04/06
 *      Author: PC-EFFECT-002
 */

#ifndef INPUTBOX_H_
#define INPUTBOX_H_

#include "EditBox.h"

/**
 * 1行テキストボックス
 */
class InputBox: public EditBox {

public:
	/**
	 * 1行テキストボックスを作成する。
	 * @param title デフォルトの入力文字列
	 * @param style スタイル
	 * @param x 左の座標
	 * @param y 上の座標
	 * @param width 幅
	 * @param height 高さ
	 * @param parent 親ウィンドウ
	 */
	InputBox(LPCTSTR title, int style, int x, int y, int width, int height,
			WindowContainer *parent);
	virtual ~InputBox();

private:
	/**
	 * テキストボックスのイベントを処理する。
	 * @param hWnd テキストボックスのハンドル
	 * @param uMsg メッセージID
	 * @param wParam 最初のメッセージパラメータ
	 * @param lParam 2番目のメッセージパラメータ
	 */
	static LRESULT CALLBACK handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);

};

#endif /* INPUTBOX_H_ */
