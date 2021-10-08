/*
 * ComboBox.h
 *
 *  Created on: 2016/01/28
 *      Author: PC-EFFECT-011
 */

#ifndef COMBOBOX_H_
#define COMBOBOX_H_

#include "Control.h"

/**
 * コンボボックス
 */
class ComboBox: public Control {
	/**
	 * コンボボックスを作成する。
	 * @param style スタイル
	 * @param x 左の座標
	 * @param y 上の座標
	 * @param width 幅
	 * @param height 高さ
	 * @param parent 親ウィンドウ
	 */
	ComboBox(int style, int x, int y, int width, int height,
			WindowContainer *parent);

	virtual ~ComboBox();
};

#endif /* COMBOBOX_H_ */
