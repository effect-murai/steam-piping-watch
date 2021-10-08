/*
 * Window.h
 *
 *  Created on: 2015/10/23
 *      Author: effectet
 */

#ifndef WINDOW_H_
#define WINDOW_H_

#include <map>

#include "WindowContainer.h"

/**
 * ウィンドウ
 */
class Window {
protected:
	/**
	 * ウィンドウのインスタンスを生成する。@n
	 * ウィンドウは作成しない。
	 */
	Window(void);
	/**
	 * 既存のウィンドウを指定してインスタンスを生成する。
	 * @param handle 既存のウィンドウのハンドル
	 */
	Window(HWND handle);
	/**
	 * パラメータを指定してウィンドウのインスタンスを生成する。
	 * @param className ウィンドウクラス名
	 * @param title ウィンドウ文字列
	 * @param style 基本スタイル
	 * @param x x座標
	 * @param y y座標
	 * @param width 幅
	 * @param height 高さ
	 * @param parent 親ウィンドウ
	 * @param exStyle 拡張スタイル
	 * @param menu メニュー
	 * @param param ウィンドウ作成時のパラメータ
	 */
	Window(LPCTSTR className, LPCTSTR title, int style, int x, int y, int width,
			int height, WindowContainer *parent, int exStyle, HMENU menu,
			void *param);

public:
	virtual ~Window(void);

	/**
	 *  ウィンドウハンドルを取得する。
	 *  @return ウィンドウハンドル
	 */
	HWND getHandle(void);

	/**
	 * フォントを設定する。
	 * @param hFont フォント
	 */
	void setFont(HFONT hFont);
	/**
	 * フォントを取得する。
	 * @return フォント
	 */
	HFONT getFont();
	/**
	 * ウィンドウ文字列を設定する。
	 * @param text ウィンドウ文字列
	 */
	void setText(LPCTSTR text);
	/**
	 * ウィンドウ文字列を取得する。
	 * @param text ウィンドウ文字列を取得するバッファ
	 * @param length バッファの長さ
	 */
	void getText(LPTSTR text, unsigned int length);
	/**
	 * ウィンドウ文字列の長さを取得する。
	 * @return ウィンドウ文字列の長さ@n
	 * Windowsの仕様で実際の文字列よりも長くなることがある。
	 */
	int getTextLength(void);
	/**
	 * ウィンドウの位置とサイズを設定する。
	 * @param x ウィンドウのx座標
	 * @param y ウィンドウのy座標
	 * @param width ウィンドウの幅
	 * @param height ウィンドウの高さ
	 */
	void move(int x, int y, int width, int height);
	/**
	 * ウィンドウの位置を設定する。
	 * @param x ウィンドウのx座標
	 * @param y ウィンドウのy座標
	 */
	void move(int x, int y);
	/**
	 * ウィンドウのサイズを設定する。
	 * @param width ウィンドウの幅
	 * @param height ウィンドウの高さ
	 */
	void resize(int width, int height);
	/**
	 * ウィンドウの表示/非表示を設定する。
	 * @param state ウィンドウの表示状態(true:表示/false:非表示)
	 */
	void show(bool state);
	/**
	 * ウィンドウを表示する。
	 */
	void show(void);
	/**
	 * ウィンドウを非表示にする。
	 */
	void hide(void);
	/**
	 * ウィンドウを有効/無効を設定する。
	 * @param state ウィンドウの有効状態(true:有効/false:無効)
	 */
	void enable(bool state);
	/**
	 * ウィンドウを有効化する。
	 */
	void enable(void);
	/**
	 * ウィンドウを無効化する。
	 */
	void disable(void);
	/**
	 * ウィンドウのクライアント領域を取得する。
	 * @param rect クライアント領域
	 */
	void getClientRect(RECT *rect);
	/**
	 * ウィンドウのクライアント領域の幅を取得する。
	 * @return クライアント領域の幅
	 */
	int clientWidth(void);
	/**
	 * ウィンドウのクライアント領域の高さを取得する。
	 * @return クライアント領域の高さ
	 */
	int clientHeight(void);
	/**
	 * ウィンドウ領域を取得する。
	 * @param rect ウィンドウ領域
	 */
	void getWindowRect(RECT *rect);

	/**
	 * フォーカスの設定
	 */
	void setFocus(void);

	/**
	 * ウィンドウの内容を更新する。
	 */
	virtual void update(void);

	/**
	 * ウィンドウの表示内容を更新する。
	 */
	void refresh(void);

	/**
	 * ポイント単位からピクセルに変換する。
	 * @param pointSize ポイント単位
	 * @return 変換後のピクセル数
	 */
	int pointToPixel(int pointSize);

	/**
	 * 指定したスタイル設定を解除する
	 * @param style 解除するスタイル
	 */
	void unsetStyle(int style);

private:
	HWND handle;

public:
	/**
	 * ウィンドウハンドルからインスタンスを取得する。
	 * @param hWnd ウィンドウハンドル
	 * @return インスタンスへのポインタ
	 */
	static Window* fromHandle(HWND hWnd);
	/**
	 * ウィンドウハンドルをインスタンスを関連付ける。
	 * @param hWnd ウィンドウハンドル
	 * @param window インスタンスへのポインタ
	 */
	static void registration(HWND hWnd, Window *window);
	/**
	 * ウィンドウハンドルとインスタンスの関連付けを解除する。
	 * @param hWnd ウィンドウハンドル
	 */
	static void unregistration(HWND hWnd);
	/**
	 * ウィンドウハンドルの関連付けをすべて解除する。
	 */
	static void clear(void);

private:
	/**
	 * ウィンドウハンドルとインスタンスの関連マップを取得する。
	 */
	static std::map<HWND, Window*>& getItems(void);
};

//------------------------------------------------------------------------------
// Global Functions
//------------------------------------------------------------------------------
bool operator==(Window &window, HWND handle);
bool operator==(HWND handle, Window &window);

#endif /* WINDOW_H_ */
