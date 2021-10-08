/*
 * EditBox.h
 *
 *  Created on: 2016/01/28
 *      Author: PC-EFFECT-011
 */

#ifndef EDITBOX_H_
#define EDITBOX_H_

#include "Control.h"

/**
 * 1行テキストボックス
 */
class EditBox: public Control {
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
	EditBox(LPCTSTR title, int style, int x, int y, int width, int height,
			WindowContainer *parent);
	virtual ~EditBox();

	int getTextAsInt(void);
	double getTextAsFloat(void);
	void setTextAsFloat(double data, int number);
	void setTextAsInt(int data);

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

#endif /* EDITBOX_H_ */
