/*
 * EditBox.cpp
 *
 *  Created on: 2016/01/29
 *      Author: PC-EFFECT-011
 */

#include <stdio.h>

#include "app.h"
#include "EditBox.h"

//------------------------------------------------------------------------------
// EditBox Class
//------------------------------------------------------------------------------

EditBox::EditBox(LPCTSTR title, int style, int x, int y, int width, int height,
		WindowContainer *parent) :
		Control(TEXT("EDIT"), title, style, x, y, width, height, parent,
		WS_EX_CLIENTEDGE) {
	this->setHandler(handleEvent);
}

EditBox::~EditBox() {
}

/**
 * EditBoxのテキストを整数値として取得する。
 * @return 変換した整数値
 */
int EditBox::getTextAsInt(void) {
	int value;
	const int bufferLength = getTextLength() + 1;
	TCHAR *buffer = new TCHAR[bufferLength];
	ZeroMemory(buffer, bufferLength * sizeof(TCHAR));
	getText(buffer, bufferLength);
	value = toInt(buffer);
	delete buffer;
	return value;
}

/**
 * EditBoxのテキストを実数値として取得する。
 * @return 変換した実数値
 */
double EditBox::getTextAsFloat(void) {
	double value;
	const int bufferLength = getTextLength() + 1;
	TCHAR *buffer = new TCHAR[bufferLength];
	ZeroMemory(buffer, bufferLength * sizeof(TCHAR));
	getText(buffer, bufferLength);
	value = toDouble(buffer);
	delete buffer;
	return value;

}

/**
 * EditBoxへ実数値をテキストとして設定する。
 * @param data 設定する実数値
 * @param number 小数点以下の桁数
 */
void EditBox::setTextAsFloat(double data, int number) {
	const int FORMAT_LEN = 16;
	const int BUFFER_SIZE = 24;
	TCHAR format[FORMAT_LEN];
	if (number > 16) {
		// 最大16桁
		number = 16;
	} else if (number < 1) {
		// 最小0桁
		number = 0;
	}
	sntprintf(format, FORMAT_LEN, TEXT("%%.%df"), number);
	TCHAR buffer[BUFFER_SIZE];
	ZeroMemory(buffer, sizeof(buffer));
	sntprintf(buffer, BUFFER_SIZE, format, data);
	setText(buffer);
}

/**
 * EditBoxへ実数値をテキストとして設定する。
 * @param data 設定する実数値
 */
void EditBox::setTextAsInt(int data) {
	const int BUFFER_SIZE = 24;
	TCHAR buffer[BUFFER_SIZE];
	ZeroMemory(buffer, sizeof(buffer));
	sntprintf(buffer, BUFFER_SIZE, TEXT("%d"), data);
	setText(buffer);
}

// テキストボックスのイベントを処理するためのコールバック関数
LRESULT CALLBACK EditBox::handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam) {

	// 固有のイベント処理
	switch (uMsg) {

	case WM_KEYDOWN:
		if (wParam == VK_RETURN) {
			// ルートウィンドウへフォーカス
			SetFocus(GetAncestor(hWnd, GA_ROOT));
			return 0;

		}

	}

	// デフォルトのイベント処理
	return CallWindowProc((WNDPROC) GetClassLongPtr(hWnd, GCLP_WNDPROC), hWnd,
			uMsg, wParam, lParam);
}
