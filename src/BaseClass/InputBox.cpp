/*
 * InputBox.cpp
 *
 *  Created on: 2016/04/06
 *      Author: PC-EFFECT-002
 */

#include "app.h"
#include "InputBox.h"
#include "StringUtils.h"

inline int Edit_GetCommaCount(HWND hWnd) {
	int commaCount = 0;
	int length = GetWindowTextLength(hWnd) + 1;
	TCHAR text[length];
	ZeroMemory(text, length * sizeof(TCHAR));
	GetWindowText(hWnd, text, length);
	int selStart = LOWORD(Edit_GetSel(hWnd));
	int selEnd = HIWORD(Edit_GetSel(hWnd));
	for (int i = 0; i < length; i++) {
		if (selStart == i) {
			i = selEnd;
		}
		if (text[i] == TEXT('.')) {
			commaCount++;
		}
	}
	return commaCount;
}

inline bool onPaste(HWND hWnd) {
	if (IsClipboardFormatAvailable(CF_TEXT)) {
		if (OpenClipboard(hWnd) == TRUE) {
			HANDLE clipBoard = GetClipboardData(CF_TEXT);
			int length = GlobalSize(clipBoard) / sizeof(TCHAR);
			TCHAR *text = fromOEMCodePageString((char*) GlobalLock(clipBoard));
			int isNumeric = true;
			int commaCount = 0;
			int start = LOWORD(Edit_GetSel(hWnd));
			commaCount = Edit_GetCommaCount(hWnd);
			for (int i = 0; (i < length) && (isNumeric == true); i++) {
				if ((text[i] < TEXT('0')) || (text[i] > TEXT('9'))) {
					// 数字以外
					if (text[i] == TEXT('-')) {
						if ((i != 0) || (start != 0)) {
							isNumeric = false;
						}
					} else if (text[i] == TEXT('.')) {
						commaCount++;
						if (commaCount >= 2) {
							isNumeric = false;
						}
					} else {
						isNumeric = false;
					}
				}
			}
			delete text;
			GlobalUnlock(clipBoard);
			CloseClipboard();
			if (isNumeric == true) {
				return false;
			}
		}
	}
	return true;
}

inline bool onChar(HWND hWnd, WPARAM wParam, LPARAM lParam) {
	// 文字列取得
	int length;
	length = GetWindowTextLength(hWnd) + 1;
	TCHAR text[length];
	GetWindowText(hWnd, text, length);
	switch (wParam) {
	case '.':
		// ピリオド
		if (LOWORD(Edit_GetSel(hWnd)) == 0) {
			// 先頭には打てない
			return true;
		}
		if (length > 1) {
			if ((LOWORD(Edit_GetSel(hWnd)) == 1) && (text[0] == TEXT('-'))) {
				// マイナスの直後も打てない
				return true;
			}
			for (int i = 0; i < length; i++) {
				if (text[i] == TEXT('.')) {
					// 既に点が入力されている
					return true;
				}
			}
		}
		break;

	case '0':

	case '1':

	case '2':

	case '3':

	case '4':

	case '5':

	case '6':

	case '7':

	case '8':

	case '9':
		// 数字キー
		break;

	case '-':
		// マイナスキー
		if (LOWORD(Edit_GetSel(hWnd)) != 0) {
			return true;
		}
		break;
	case VK_BACK:
		// BACK SPACEキーはなぜか送られてくるので
		break;
	default:
		if (((GetKeyState(VK_MENU) & 0x8000) != 0)
				|| ((GetKeyState(VK_CONTROL) & 0x8000) != 0)) {
			// Control / Alt が押されている場合は従来どおり
			return false;
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
// InputBox Class
//------------------------------------------------------------------------------

InputBox::InputBox(LPCTSTR title, int style, int x, int y, int width,
		int height, WindowContainer *parent) :
		EditBox(title, style & (~ES_MULTILINE), x, y, width, height, parent) {
	this->setHandler(handleEvent);
}

InputBox::~InputBox() {
}

// テキストボックスのイベントを処理するためのコールバック関数
LRESULT CALLBACK InputBox::handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam) {

	// 固有のイベント処理
	switch (uMsg) {

	case WM_CHAR:
		if (onChar(hWnd, wParam, lParam)) {
			return 0;
		}
		break;

	case WM_KEYDOWN:
		switch (wParam) {
		case VK_RETURN:
			// ルートウィンドウへフォーカス
			SetFocus(GetAncestor(hWnd, GA_ROOT));
			return 0;
		}
		break;

	case WM_PASTE:
		if (onPaste(hWnd)) {
			return 0;
		}
		break;

	}

	// デフォルトのイベント処理
	return CallWindowProc((WNDPROC) GetClassLongPtr(hWnd, GCLP_WNDPROC), hWnd,
			uMsg, wParam, lParam);
}
