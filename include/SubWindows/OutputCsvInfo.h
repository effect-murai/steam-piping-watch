/*
 * OutputCsvInfo.h
 *
 *  Created on: 2016/03/04
 *      Author: PC-EFFECT-011
 */

#ifndef OUTPUTCSVINFO_H_
#define OUTPUTCSVINFO_H_

#include "Dialog.h"

class OutputCsvInfoDialog: public Dialog {
public:
	OutputCsvInfoDialog(HWND handle);
	~OutputCsvInfoDialog();

	static INT_PTR CALLBACK handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);
	static INT_PTR create(HWND parent);

private:

};

#endif /* OUTPUTCSVINFO_H_ */
