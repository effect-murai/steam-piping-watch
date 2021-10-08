/*
 * CheckBox.cpp
 *
 *  Created on: 2016/01/29
 *      Author: PC-EFFECT-011
 */

#include "CheckBox.h"

//------------------------------------------------------------------------------
// CheckBox Class
//------------------------------------------------------------------------------

CheckBox::CheckBox(LPCTSTR title, int style, int x, int y, int width,
		int height, WindowContainer *parent) :
		Button(title, BS_AUTOCHECKBOX | (style) | WS_TABSTOP | BS_NOTIFY, x, y,
				width, height, parent) {
}

CheckBox::~CheckBox() {
}
