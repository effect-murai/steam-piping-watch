/*
 * PushButton.cpp
 *
 *  Created on: 2016/01/29
 *      Author: PC-EFFECT-011
 */

#include "PushButton.h"

//------------------------------------------------------------------------------
// PushButton Class
//------------------------------------------------------------------------------

PushButton::PushButton(LPCTSTR title, int style, int x, int y, int width,
		int height, WindowContainer *parent) :
		Button(title, (style) | WS_TABSTOP, x, y, width, height, parent) {
}

PushButton::~PushButton() {
}
