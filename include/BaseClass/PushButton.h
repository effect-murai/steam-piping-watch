/*
 * PushButton.h
 *
 *  Created on: 2016/01/28
 *      Author: PC-EFFECT-011
 */

#ifndef PUSHBUTTON_H_
#define PUSHBUTTON_H_

#include "Button.h"

/**
 * 押しボタン
 */
class PushButton: public Button {
public:
	/**
	 * 押しボタンを作成する。
	 * @param title ボタンの表示文字列
	 * @param style スタイル
	 * @param x 左の座標
	 * @param y 上の座標
	 * @param width 幅
	 * @param height 高さ
	 * @param parent 親ウィンドウ
	 */
	PushButton(LPCTSTR title, int style, int x, int y, int width, int height,
			WindowContainer *parent);

	virtual ~PushButton();
};

#endif /* PUSHBUTTON_H_ */
