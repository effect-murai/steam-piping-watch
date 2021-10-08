/*
 * RadioButton.h
 *
 *  Created on: 2016/01/28
 *      Author: PC-EFFECT-011
 */

#ifndef RADIOBUTTON_H_
#define RADIOBUTTON_H_

#include "Button.h"

/**
 * ラジオボタン
 */
class RadioButton: public Button {
public:
	/**
	 * ラジオボタンを作成する。
	 * @param title 表示文字列
	 * @param style スタイル
	 * @param x 左の座標
	 * @param y 上の座標
	 * @param width 幅
	 * @param height 高さ
	 * @param parent 親ウィンドウ
	 */
	RadioButton(LPCTSTR title, int style, int x, int y, int width, int height,
			WindowContainer *parent);

	virtual ~RadioButton();
};

#endif /* RADIOBUTTON_H_ */
