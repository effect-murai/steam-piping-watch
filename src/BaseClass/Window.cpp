/*
 * Window.cpp
 *
 *  Created on: 2015/10/23
 *      Author: effectet
 */
#include "Window.h"

//------------------------------------------------------------------------------
// Window Class
//------------------------------------------------------------------------------

Window::Window(void) :
		handle(NULL) {
}

Window::Window(HWND handle) :
		handle(handle) {
	registration(handle, this);
}

Window::Window(LPCTSTR className, LPCTSTR title, int style, int x, int y,
		int width, int height, WindowContainer *parent, int exStyle, HMENU menu,
		void *param) {
	handle = CreateWindowEx(exStyle, className, title, style | WS_CHILD, x, y,
			width, height, parent->getHandle(), menu,
			GetModuleHandle(NULL), param);
	registration(handle, this);
}

Window::~Window(void) {
	if (handle != NULL) {
		unregistration(handle);
	}
}

HWND Window::getHandle(void) {
	return handle;
}

/**
 * フォントを設定する
 * @param[in] hFont フォントのハンドル
 */
void Window::setFont(HFONT hFont) {
	SendMessage(handle, WM_SETFONT, (WPARAM) hFont, (LPARAM) TRUE);
}

/**
 * フォントを取得する
 * @return フォントのハンドル
 */
HFONT Window::getFont() {
	return (HFONT) SendMessage(handle, WM_GETFONT, (WPARAM) 0, (LPARAM) 0);
}

/**
 * テキストを設定する
 * @param[in] text 設定するテキスト
 */
void Window::setText(LPCTSTR text) {
	SendMessage(handle, WM_SETTEXT, (WPARAM) 0, (LPARAM) text);
}

/**
 * テキストを取得する
 * @param[out] text テキストを取得する文字列バッファへのポインタ
 * @param[in] length 文字列バッファのサイズ
 */
void Window::getText(LPTSTR text, unsigned int length) {
	SendMessage(handle, WM_GETTEXT, (WPARAM) length, (LPARAM) text);
}

/**
 * テキストの文字数を取得する@n
 * ※Windowsの仕様により、実際の文字数よりも多い文字数を返すことがある。
 * 正確な文字数を得るためにはテキストを取得し、その文字列の解析が必要となる。
 * @return テキストの文字数
 */
int Window::getTextLength(void) {
	return (int) SendMessage(handle, WM_GETTEXTLENGTH, (WPARAM) 0, (LPARAM) 0);
}

void Window::move(int x, int y, int width, int height) {
	SetWindowPos(handle, NULL, x, y, width, height, SWP_NOZORDER);
}

void Window::move(int x, int y) {
	SetWindowPos(handle, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

void Window::resize(int width, int height) {
	SetWindowPos(handle, NULL, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);
}

void Window::show(bool state) {
	SetWindowPos(handle, NULL, 0, 0, 0, 0,
			SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE
					| (state ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
}

void Window::show(void) {
	show(SW_SHOW);
}

void Window::hide(void) {
	show(SW_HIDE);
}

void Window::getClientRect(RECT *rect) {
	GetClientRect(handle, rect);
}

void Window::getWindowRect(RECT *rect) {
	GetWindowRect(handle, rect);
}

int Window::clientWidth(void) {
	RECT rect;
	getClientRect(&rect);
	return rect.right - rect.left;
}

int Window::clientHeight(void) {
	RECT rect;
	getClientRect(&rect);
	return rect.bottom - rect.top;
}

void Window::update(void) {
	RECT rect;
	getWindowRect(&rect);
	rect.right -= rect.left;
	rect.bottom -= rect.top;
	rect.left = rect.top = 0;
	InvalidateRect(handle, &rect, true);
}

void Window::refresh(void) {
	InvalidateRect(handle, NULL, true);
}

void Window::enable(bool state) {
	EnableWindow(handle, (state == true ? TRUE : FALSE));
}

void Window::enable(void) {
	enable(true);
}

void Window::disable(void) {
	enable(false);
}

void Window::setFocus(void) {
	SetFocus(handle);
}

/**
 * ポイント単位をpixelに変換する。
 * @param[in] pointSize 変換するポイント数
 * @return 変換したpixel数
 */
int Window::pointToPixel(int pointSize) {
	HDC hdc = GetWindowDC(handle);
	int ret = MulDiv(pointSize, GetDeviceCaps(hdc,
	LOGPIXELSY), 72);
	ReleaseDC(handle, hdc);
	return ret;
}

#define GetWindowStyleL(hwnd) (GetWindowLongPtr((hwnd), GWL_STYLE))
#define SetWindowStyleL(hwnd, style) (SetWindowLongPtr((hwnd), GWL_STYLE, (style)))

void Window::unsetStyle(int style) {
	LONG_PTR _style = GetWindowStyleL(handle);
	SetWindowStyleL(handle, _style & (~style));
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Window Class Functions
//------------------------------------------------------------------------------

Window* Window::fromHandle(HWND hWnd) {
	std::map<HWND, Window*>::iterator it;
	it = getItems().find(hWnd);
	if (it == getItems().end()) {
		return NULL;
	}
	return it->second;
}

void Window::registration(HWND hWnd, Window *window) {
	if ((hWnd == NULL) || (window == NULL)) {
		return;
	}
	if (fromHandle(hWnd) == NULL) {
		if (window->handle == NULL) {
			window->handle = hWnd;
		}
		getItems().insert(std::pair<HWND, Window*>(hWnd, window));
	}
}

void Window::unregistration(HWND hWnd) {
	std::map<HWND, Window*>::iterator it;
	it = getItems().find(hWnd);
	if (it != getItems().end()) {
		getItems().erase(it);
	}
}

void Window::clear(void) {
	getItems().clear();
}

std::map<HWND, Window*>& Window::getItems(void) {
	static std::map<HWND, Window*> items;
	return items;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global Functions
//------------------------------------------------------------------------------

bool operator==(Window &window, HWND handle) {
	return handle == window.getHandle();
}

bool operator==(HWND handle, Window &window) {
	return handle == window.getHandle();
}
