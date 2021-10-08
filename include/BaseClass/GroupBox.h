/*
 * GroupBox.h
 *
 *  Created on: 2016/01/28
 *      Author: PC-EFFECT-011
 */

#ifndef GROUPBOX_H_
#define GROUPBOX_H_

#include "Button.h"

/**
 * グループボックス
 */
class GroupBox: public Button, public WindowContainer {
public:
	/**
	 * グループボックスを作成する。
	 * @param title グループボックス上部に表示する文字列
	 * @param style スタイル
	 * @param x 左の座標
	 * @param y 上の座標
	 * @param width 幅
	 * @param height 高さ
	 * @param parent 親ウィンドウ
	 */
	GroupBox(LPCTSTR title, int style, int x, int y, int width, int height,
			WindowContainer *parent);

	virtual ~GroupBox();

	/**
	 * ウィンドウハンドルを取得する。
	 */
	HWND getHandle(void);

private:
	/**
	 * グループボックスのイベントを処理する。
	 * @param hWnd グループボックスのハンドル
	 * @param uMsg メッセージID
	 * @param wParam 最初のメッセージパラメータ
	 * @param lParam 2番目のメッセージパラメータ
	 */
	static LRESULT CALLBACK handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);
};

#endif /* GROUPBOX_H_ */
