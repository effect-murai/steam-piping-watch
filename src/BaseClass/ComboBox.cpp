/*
 * ComboBox.cpp
 *
 *  Created on: 2016/01/29
 *      Author: PC-EFFECT-011
 */

#include "ComboBox.h"

//------------------------------------------------------------------------------
// ComboBox Class
//------------------------------------------------------------------------------

ComboBox::ComboBox(int style, int x, int y, int width, int height,
		WindowContainer *parent) :
		Control(TEXT("COMBOBOX"), NULL, CBS_DROPDOWNLIST | (style) | WS_TABSTOP,
				x, y, width, height, parent) {
}

ComboBox::~ComboBox() {
}
