/*
 * Control.cpp
 *
 *  Created on: 2016/01/29
 *      Author: PC-EFFECT-011
 */

#include "Control.h"

#ifndef TTM_SETMAXTIPWIDTH
#define TTM_SETMAXTIPWIDTH (WM_USER + 24)
#endif
#ifndef TTS_BALLOON
#define TTS_BALLOON 0x40
#endif

//------------------------------------------------------------------------------
// Inline Functions
//------------------------------------------------------------------------------
inline int calcToolTipWidth(HWND toolTip, TCHAR *text) {
	RECT toolTipRect;
	ZeroMemory(&toolTipRect, sizeof(RECT));
	HFONT currentFont = (HFONT) SendMessage(toolTip, WM_GETFONT, 0, 0);
	HDC hDC = GetDC(toolTip);
	HFONT originalFont = (HFONT) SelectObject(hDC, currentFont);
	int length = lstrlen(text) + 1;
	DrawText(hDC, text, length, &toolTipRect, DT_CALCRECT | DT_LEFT | DT_TOP);
	SelectObject(hDC, originalFont);
	ReleaseDC(toolTip, hDC);
	return toolTipRect.right;
}

//------------------------------------------------------------------------------
// Control Class
//------------------------------------------------------------------------------

Control::Control(LPCTSTR className, LPCTSTR title, int style, int x, int y,
		int width, int height, WindowContainer *parent, int exStyle) :
		Window(className, title, style, x, y, width, height, parent, exStyle,
		NULL, NULL) {
	toolTip = NULL;
}

Control::Control(LPCTSTR className, LPCTSTR title, int style, int x, int y,
		int width, int height, WindowContainer *parent) :
		Window(className, title, style, x, y, width, height, parent, 0, NULL,
		NULL) {
	toolTip = NULL;
}

Control::~Control() {
}

void Control::setCheck(void) {
	// 仮想関数(実装なし)
}

void Control::addToolTipForRect(const TCHAR *text) {
	addToolTipForRect(text, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP);
}

void Control::addBalloonToolTipForRect(const TCHAR *text) {
	addToolTipForRect(text,
	WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON);
}

void Control::updateToolTipText(const TCHAR *text) {
	if (toolTip == NULL) {
		return;
	}

	toolInfo.lpszText = (TCHAR*) text;
	getClientRect(&toolInfo.rect);

	SendMessage(toolTip, TTM_UPDATETIPTEXT, 0, (LPARAM) (LPTOOLINFO) &toolInfo);

	updateToolTipWidth();
}

void Control::deleteToolTip(void) {
	if (toolTip == NULL) {
		return;
	}

	SendMessage(toolTip, TTM_DELTOOL, 0, (LPARAM) (LPTOOLINFO) &toolInfo);
	DestroyWindow(toolTip);
	ZeroMemory(&toolInfo, sizeof(TOOLINFO));
	toolTip = NULL;
}

void Control::setToolTipInitialTime(int time) {
	if (toolTip == NULL) {
		return;
	}
	SendMessage(toolTip, TTM_SETDELAYTIME, (WPARAM) TTDT_INITIAL,
			(LPARAM) time);
}

void Control::setToolTipPopupTime(int time) {
	if (toolTip == NULL) {
		return;
	}
	SendMessage(toolTip, TTM_SETDELAYTIME, (WPARAM) TTDT_AUTOPOP,
			(LPARAM) time);
}

void Control::setToolTipReshowTime(int time) {
	if (toolTip == NULL) {
		return;
	}
	SendMessage(toolTip, TTM_SETDELAYTIME, (WPARAM) TTDT_RESHOW, (LPARAM) time);
}

void Control::addToolTipForRect(const TCHAR *text, int style) {
	if (toolTip != NULL) {
		updateToolTipText(text);
		return;
	}

	// ツールチップを作成
	toolTip = CreateWindowEx(
	WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, style,
	CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, getHandle(),
	NULL, GetModuleHandle(NULL), NULL);

	SetWindowPos(toolTip, HWND_TOPMOST, 0, 0, 0, 0,
	SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	// ツール情報を設定する。ここでツールに親ウィンドウを設定する。
	ZeroMemory(&toolInfo, sizeof(TOOLINFO));
	toolInfo.cbSize = sizeof(TOOLINFO);
	toolInfo.uFlags = TTF_SUBCLASS;
	toolInfo.hwnd = getHandle();
	toolInfo.hinst = GetModuleHandle(NULL);
	toolInfo.lpszText = (TCHAR*) text;

	getClientRect(&toolInfo.rect);

	// ツールウィンドウにツールチップを関連付ける
	SendMessage(toolTip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &toolInfo);

	updateToolTipWidth();
}

void Control::updateToolTipWidth(void) {
	// ツールチップのサイズを調節する
	int toolTipWidth = calcToolTipWidth(toolTip, toolInfo.lpszText);
	SendMessage(toolTip, TTM_SETMAXTIPWIDTH, 0, toolTipWidth);
}

void Control::setHandler(WNDPROC procedure) {
	SetWindowLongPtr(getHandle(), GWLP_WNDPROC, (LONG_PTR) procedure);
}

LRESULT Control::callDefaultProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return CallWindowProc((WNDPROC) GetClassLongPtr(getHandle(), GCLP_WNDPROC),
			getHandle(), uMsg, wParam, lParam);
}
