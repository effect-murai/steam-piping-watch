/*
 * Label.cpp
 *
 *  Created on: 2016/01/29
 *      Author: PC-EFFECT-011
 */

#include "Label.h"

//------------------------------------------------------------------------------
// Label Class
//------------------------------------------------------------------------------

Label::Label(LPCTSTR title, int style, int x, int y, int width, int height,
		WindowContainer *parent) :
		Control(TEXT("STATIC"), title, style, x, y, width, height, parent, 0) {
}

Label::Label(LPCTSTR title, int style, int x, int y, int width, int height,
		WindowContainer *parent, bool clientEdge) :
		Control(TEXT("STATIC"), title, style, x, y, width, height, parent,
				(clientEdge ? WS_EX_CLIENTEDGE : 0)) {
}

Label::~Label() {
}

void Label::autoSize(void) {
	HFONT currentFont = (HFONT) SendMessage(getHandle(), WM_GETFONT, 0, 0);
	HDC hDC = GetDC(getHandle());
	HFONT originalFont = (HFONT) SelectObject(hDC, currentFont);
	int length = getTextLength() + 1;
	TCHAR *text = new TCHAR[length];
	getText(text, length);
	SIZE size;
	GetTextExtentPoint32(hDC, text, length, &size);
	delete text;
	SelectObject(hDC, originalFont);
	ReleaseDC(getHandle(), hDC);
	resize(size.cx, size.cy);
}
