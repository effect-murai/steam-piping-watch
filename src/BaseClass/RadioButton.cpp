/*
 * RadioButton.cpp
 *
 *  Created on: 2016/01/29
 *      Author: PC-EFFECT-011
 */

#include "RadioButton.h"

//------------------------------------------------------------------------------
// RadioButton Class
//------------------------------------------------------------------------------

RadioButton::RadioButton(LPCTSTR title, int style, int x, int y, int width,
		int height, WindowContainer *parent) :
		Button(title, BS_AUTORADIOBUTTON | (style) | WS_TABSTOP | BS_NOTIFY, x,
				y, width, height, parent) {
}

RadioButton::~RadioButton() {
}
