/*
 * Resource.cpp
 *
 *  Created on: 2016/01/29
 *      Author: PC-EFFECT-012
 */

#include "resource.h"

/**
 * リソースから文字列を取得する。
 * @param[in] id リソースID
 * @return リソース文字列へのポインタ
 */
LPTSTR Resource::getString(int id) {
	static TCHAR resString[MAX_LOADSTRING];
	LoadString(GetModuleHandle(NULL), id, resString, MAX_LOADSTRING);
	return resString;
}

/**
 * リソースから文字列を取得する。
 * @param[in] id リソースID
 * @param[out] resString リソース文字列へのポインタ
 */
void Resource::getString(int id, TCHAR *resString) {
	LoadString(GetModuleHandle(NULL), id, resString, MAX_LOADSTRING);
}

/**
 * リソースからアイコンを取得する。
 * @param[in] id リソースID
 * @return アイコンのハンドル
 */
HICON Resource::getIcon(int id) {
	return LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(id));
}

/**
 * リソースからビットマップを取得する。
 * @param[in] id リソースID
 * @return ビットマップのハンドル
 */
HBITMAP Resource::getBitmap(int id) {
	return LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(id));
}

/**
 * リソース内に定義されたダイアログを表示する。
 * @param[in] id リソースID
 * @param[in] parent 親ウィンドウのハンドル
 * @param[in] handler イベント処理を行うコールバック関数へのポインタ
 */
INT_PTR Resource::showDialog(int id, HWND parent, DLGPROC handler) {
	return DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(id), parent, handler);
}

/**
 * リソース内に定義されたダイアログを表示する。
 * @param[in] id リソースID
 * @param[in] parent 親ウィンドウのハンドル
 * @param[in] handler イベント処理を行うコールバック関数へのポインタ
 * @param[in] param ダイアログ作成時に渡すパラメータ
 */
INT_PTR Resource::showDialog(int id, HWND parent, DLGPROC handler,
		LPARAM param) {
	return DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(id), parent,
			handler, param);
}

HRSRC Resource::getObject(int id, int type) {
	return FindResourceEx(GetModuleHandle(NULL), MAKEINTRESOURCE(type),
	MAKEINTRESOURCE(id), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
}

/**
 * リソース内に定義された文字列にてメッセージボックスを表示する
 * @param[in] parent 親ウィンドウのハンドル
 * @param[in] textId リソースID(本文)
 * @param[in] titleId リソースID(タイトル)
 * @param[in] type ICON種別
 */
INT_PTR Resource::showMessageBox(HWND parent, int textId, int titleId,
		int type) {
	TCHAR *title = new TCHAR[Resource::MAX_LOADSTRING];
	TCHAR *message = new TCHAR[Resource::MAX_LOADSTRING];
	Resource::getString(textId, message);
	Resource::getString(titleId, title);
	int ret = MessageBox(parent, message, title, type);
	delete title;
	delete message;
	return ret;
}
