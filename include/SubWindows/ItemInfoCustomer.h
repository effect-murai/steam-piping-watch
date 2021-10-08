/*
 * ItemInfoCustomer.h
 *
 *  Created on: 2016/03/30
 *      Author: PC-EFFECT-002
 */

#ifndef ITEMINFOCUSTOMER_H_
#define ITEMINFOCUSTOMER_H_

#include "Dialog.h"

class ItemInfoCustomerDialog: public Dialog {
public:
	ItemInfoCustomerDialog(HWND handle);
	~ItemInfoCustomerDialog();

	static INT_PTR CALLBACK handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);
	static int create(HWND parent);

private:

};

#endif /* ITEMINFOCUSTOMER_H_ */

