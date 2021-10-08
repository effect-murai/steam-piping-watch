/*
 * Button.cpp
 *
 *  Created on: 2016/01/29
 *      Author: PC-EFFECT-011
 */

#include "Button.h"

//------------------------------------------------------------------------------
// Button Class
//------------------------------------------------------------------------------

Button::Button(LPCTSTR title, int style, int x, int y, int width, int height,
		WindowContainer *parent) :
		Control(TEXT("BUTTON"), title, style, x, y, width, height, parent) {
}

Button::~Button() {
}

void Button::setCheck(void) {
	setCheck(true);
}

void Button::setCheck(bool value) {
	Button_SetCheck(getHandle(), value);
}

int Button::getCheck(void) {
	return Button_GetCheck(getHandle());
}
