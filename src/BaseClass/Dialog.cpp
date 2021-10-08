/*
 * Dialog.cpp
 *
 *  Created on: 2016/02/01
 *      Author: PC-EFFECT-012
 */

#include "app.h"
#include "Dialog.h"
#include <winuser.h>
#include <stdio.h>

//------------------------------------------------------------------------------
// Dialog class
//------------------------------------------------------------------------------
Dialog::Dialog(HWND handle) :
		Window(handle) {
}

Dialog::~Dialog(void) {
}

/**
 * ダイアログのハンドルを取得する。
 * @param result ダイアログのハンドル
 */
HWND Dialog::getHandle(void) {
	return Window::getHandle();
}

/**
 * ダイアログを終了する。
 * @param result ダイアログの戻り値
 */
void Dialog::close(INT_PTR result) {

	EndDialog(this->getHandle(), result);
}

/**
 * ダイアログアイテムのハンドルを取得する。
 * @param itemId ダイアログアイテムID
 * @return ダイアログアイテムのハンドル
 */
HWND Dialog::getDlgItem(int itemId) {
	return GetDlgItem(this->getHandle(), itemId);
}

/**
 * 空のダイアログを作成する。
 * @param parent 親ウィンドウ
 * @param eventHandler イベント処理を行う関数へのポインタ
 * @param param ダイアログ初期化時の与えるパラメータ
 * @return ダイアログの戻り値
 */
HWND Dialog::create(Window *parent, DLGPROC eventHandler, LPARAM param) {
	int left = 200;
	int top = 200;
	int width = 200;
	int height = 200;
	const TCHAR *title = TEXT("");
	const TCHAR *fontName = TEXT("Meiryo UI");
	int fontSize = 10;

	const int MAX_SIZE = 1024;
	char dlgTemplateData[MAX_SIZE];
	ZeroMemory(dlgTemplateData, MAX_SIZE);
	LPDLGTEMPLATE dlgTemplate = (LPDLGTEMPLATE) dlgTemplateData;
	dlgTemplate->style = WS_POPUP | WS_BORDER | WS_SYSMENU | DS_NOFAILCREATE
			| WS_CAPTION | DS_SETFONT;
	dlgTemplate->cdit = 0;  // コントロールなし
	dlgTemplate->x = left;
	dlgTemplate->y = top;
	dlgTemplate->cx = width;
	dlgTemplate->cy = height;

	// 次のメンバの設定に移ります。
	LPWORD lpw = (LPWORD) (dlgTemplate + 1);

	// <Menu ID>
	//   *lpw++ = の文は、その位置に値を設定した後 ++ を行います。
	//   (つまり *lpw = 0; lpw++; と同じ)
	*lpw++ = 0;

	// <Window Class>
	*lpw++ = 0;

	// <Dialog Title>
	LPWSTR lpwsz = (LPWSTR) lpw;

	// MultiByteToWideChar 関数で ANSI 文字列を Unicode 文字列に変換します。
	// 戻り値 (書き込んだ文字列の長さ) に NULL 文字が含まれているので + 1 しません。

#ifndef UNICODE
	int nChar = MultiByteToWideChar(CP_ACP, 0, title, -1, lpwsz, 50);
#else
	wcscpy(lpwsz, title);
	int nChar = lstrlen(title);
#endif

	// lpw = (LPWORD) lpwsz は必要ありません (アドレスが同じであるため)。
	lpw += nChar;

	// WORD fontSize
	// ※ DS_SETFONT を指定しなかった場合、コントロールの設定に
	//   移るまでのデータ設定は行いません。
	*lpw++ = fontSize;

	// WCHAR fontName[]
	lpwsz = (LPWSTR) lpw;
#ifndef UNICODE
	nChar = MultiByteToWideChar(CP_ACP, 0, fontName, -1, lpwsz, 50);
#else
	wcscpy(lpwsz, fontName);
	nChar = lstrlen(fontName);
#endif
	lpw += nChar;

	return CreateDialogIndirectParam(
	GetModuleHandle(NULL), dlgTemplate, parent->getHandle(), eventHandler,
			param);
}

void Dialog::setDialogItemTextAsInt(HWND dialog, int itemId, int value) {
	TCHAR text[256];

	sntprintf(text, 256, TEXT("%d"), value);
	SetDlgItemText(dialog, itemId, text);
}

int Dialog::getDialogItemTextAsInt(HWND dialog, int itemId) {
	TCHAR text[256];
	int value = 0;

	GetDlgItemText(dialog, itemId, text, 256);
	if (wcslen(text) != 0) {
		value = toInt(text);
	}

	return value;
}
