/*
 * ChgReg.h
 *
 *  Created on: 2020/11/26
 *      Author: A.Masago
 */

#ifndef INCLUDE_SUBWINDOWS_CHGREG_H_
#define INCLUDE_SUBWINDOWS_CHGREG_H_

#include <windows.h>
#include "resource.h"
#include "Dialog.h"

class ChgRegDialog: public Dialog {
public:
	ChgRegDialog(HWND handle);
	~ChgRegDialog(void);

	static INT_PTR CALLBACK handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);
	static INT_PTR create(HWND parent);
};

#endif /* INCLUDE_SUBWINDOWS_CHGREG_H_ */
