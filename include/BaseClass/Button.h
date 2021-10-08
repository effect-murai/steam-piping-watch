/*
 * Button.h
 *
 *  Created on: 2016/01/28
 *      Author: PC-EFFECT-011
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include "Control.h"

/**
 * ボタン
 */
class Button: public Control {
protected:
	/**
	 * ボタンを作成する。
	 * @param title ボタンの表示文字列
	 * @param style スタイル
	 * @param x 左の座標
	 * @param y 上の座標
	 * @param width 幅
	 * @param height 高さ
	 * @param parent 親ウィンドウ
	 */
	Button(LPCTSTR title, int style, int x, int y, int width, int height,
			WindowContainer *parent);

	virtual ~Button();

public:
	/**
	 * チェックを入れる。@n
	 * 内容は setCheck(true) と同じ。
	 * @see setCheck(bool value)
	 */
	void setCheck(void);

	/**
	 * ボタンのチェック状態を設定する。
	 * @param value チェック状態(true:チェック/false:チェックなし)
	 */
	void setCheck(bool value);

	/**
	 * ボタンのチェック状態を取得する。
	 * @return チェック状態(0:チェックなし/1:チェックあり/2:無効(3Stateの場合))
	 */
	int getCheck(void);
};

#endif /* BUTTON_H_ */
