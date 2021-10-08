/*
 * CheckBox.h
 *
 *  Created on: 2016/01/28
 *      Author: PC-EFFECT-011
 */

#ifndef CHECKBOX_H_
#define CHECKBOX_H_

#include "Button.h"

/**
 * チェックボックス
 */
class CheckBox: public Button {
public:
	/**
	 * チェックボックスを作成する。
	 * @param title 表示文字列
	 * @param style スタイル
	 * @param x 左の座標
	 * @param y 上の座標
	 * @param width 幅
	 * @param height 高さ
	 * @param parent 親ウィンドウ
	 */
	CheckBox(LPCTSTR title, int style, int x, int y, int width, int height,
			WindowContainer *parent);

	virtual ~CheckBox();
};

#endif /* CHECKBOX_H_ */
