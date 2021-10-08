/*
 * Label.h
 *
 *  Created on: 2016/01/28
 *      Author: PC-EFFECT-011
 */

#ifndef LABEL_H_
#define LABEL_H_

#include "Control.h"

/**
 * ラベル
 */
class Label: public Control {
public:
	/**
	 * 境界線なしのラベルを作成する。
	 * @param title 表示文字列
	 * @param style スタイル
	 * @param x 左の座標
	 * @param y 上の座標
	 * @param width 幅
	 * @param height 高さ
	 * @param parent 親ウィンドウ
	 */
	Label(LPCTSTR title, int style, int x, int y, int width, int height,
			WindowContainer *parent);

	/**
	 * 境界線の有無を指定してラベルを作成する。
	 * @param title ボタンの表示文字列
	 * @param style スタイル
	 * @param x 左の座標
	 * @param y 上の座標
	 * @param width 幅
	 * @param height 高さ
	 * @param parent 親ウィンドウ
	 * @param clientEdge 境界線の有無(true:あり/false:なし)
	 */
	Label(LPCTSTR title, int style, int x, int y, int width, int height,
			WindowContainer *parent, bool clientEdge);

	virtual ~Label();

	/**
	 * サイズを自動調整する。
	 */
	void autoSize(void);
};

#endif /* LABEL_H_ */
