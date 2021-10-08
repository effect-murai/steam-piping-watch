/*
 * Control.h
 *
 *  Created on: 2016/01/28
 *      Author: PC-EFFECT-011
 */

#ifndef CONTROL_H_
#define CONTROL_H_

#include "Window.h"

/**
 * コントロールの基底クラス
 */
class Control: public Window {
protected:
	/**
	 * 拡張スタイルを指定してコントロールを作成する。
	 * @param className ウィンドウクラス名
	 * @param title タイトル文字列
	 * @param style 基本スタイル
	 * @param x 左の座標
	 * @param y 上の座標
	 * @param width 幅
	 * @param height 高さ
	 * @param parent 親ウィンドウ
	 * @param exStyle 拡張スタイル
	 */
	Control(LPCTSTR className, LPCTSTR title, int style, int x, int y,
			int width, int height, WindowContainer *parent, int exStyle);

	/**
	 * 基本パラメータのみを指定してコントロールを作成する。
	 * @param className ウィンドウクラス名
	 * @param title タイトル文字列
	 * @param style 基本スタイル
	 * @param x 左の座標
	 * @param y 上の座標
	 * @param width 幅
	 * @param height 高さ
	 * @param parent 親ウィンドウ
	 */
	Control(LPCTSTR className, LPCTSTR title, int style, int x, int y,
			int width, int height, WindowContainer *parent);

	virtual ~Control();

public:
	virtual void setCheck(void);

	/**
	 * コントロールにツールチップを追加する。
	 * @param Text ツールチップに表示する文字列
	 */
	void addToolTipForRect(LPCTSTR Text);
	void addBalloonToolTipForRect(const TCHAR *Text);

	void updateToolTipText(LPCTSTR text);

	void deleteToolTip(void);

	void setToolTipInitialTime(int time);
	void setToolTipPopupTime(int time);
	void setToolTipReshowTime(int time);

	/**
	 * イベントハンドラを設定する。
	 * @param　procedure イベント処理を行うコールバック関数へのポインタ
	 */
	void setHandler(WNDPROC procedure);
	/**
	 * デフォルトのイベントハンドラを呼び出す。
	 * @param uMsg メッセージID
	 * @param wParam 1つ目のパラメータ
	 * @param lParam 2つ目のパラメータ
	 * @return デフォルト処理の戻り値
	 */
	LRESULT callDefaultProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	void addToolTipForRect(const TCHAR *text, int style);
	void updateToolTipWidth(void);

	HWND toolTip;
	TOOLINFO toolInfo;
};

#endif /* CONTROL_H_ */
