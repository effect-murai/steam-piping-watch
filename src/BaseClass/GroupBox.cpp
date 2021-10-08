/*
 * GroupBox.cpp
 *
 *  Created on: 2016/01/29
 *      Author: PC-EFFECT-011
 */

#include "GroupBox.h"

//------------------------------------------------------------------------------
// GroupBox Class
//------------------------------------------------------------------------------

GroupBox::GroupBox(LPCTSTR title, int style, int x, int y, int width,
		int height, WindowContainer *parent) :
		Button(title, BS_GROUPBOX | (style), x, y, width, height, parent) {
	this->setHandler(handleEvent);
}

GroupBox::~GroupBox() {
}

HWND GroupBox::getHandle(void) {
	// 複数の親に同じ名前の関数が定義されているので
	// 再定義の必要がある(?ω?)
	return Control::getHandle();
}

// グループボックスのイベントを処理するためのコールバック関数
LRESULT CALLBACK GroupBox::handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam) {
	// 固有のイベント処理
	switch (uMsg) {
	case WM_COMMAND:
		// 親ウィンドウにメッセージを転送
		PostMessage(GetParent(hWnd), uMsg, wParam, lParam);
		break;
	case WM_CTLCOLORSTATIC:
		// 親ウィンドウにメッセージを転送
		return SendMessage(GetParent(hWnd), uMsg, wParam, lParam);
	}

	// デフォルトのイベント処理
	return CallWindowProc((WNDPROC) GetClassLongPtr(hWnd, GCLP_WNDPROC), hWnd,
			uMsg, wParam, lParam);
}
