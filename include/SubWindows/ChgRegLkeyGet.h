/*
 * ChgRegLkeyGet.h
 *
 *  Created on: 2020/11/26
 *      Author: A.Masago
 */

#ifndef INCLUDE_SUBWINDOWS_CHGREGLKEYGET_H_
#define INCLUDE_SUBWINDOWS_CHGREGLKEYGET_H_

#include <windows.h>
#include "resource.h"
#include "Dialog.h"

class ChgRegLkeyGetDialog: public Dialog {
public:
	// ライセンスキー発行依頼データ
	typedef struct {
		TCHAR company[Resource::MAX_LOADSTRING];
		TCHAR department[Resource::MAX_LOADSTRING];
		TCHAR fullname[Resource::MAX_LOADSTRING];
		TCHAR email[Resource::MAX_LOADSTRING];
		int contract;
	} LKeyRequestParam;

	ChgRegLkeyGetDialog(HWND handle);
	~ChgRegLkeyGetDialog(void);

	static INT_PTR CALLBACK handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);
	static INT_PTR create(HWND parent);
};

#endif /* INCLUDE_SUBWINDOWS_CHGREGLKEYGET_H_ */
