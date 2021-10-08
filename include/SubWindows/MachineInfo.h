/*
 * MachineInfo.h
 *
 *  Created on: 2016/02/15
 *      Author: PC-EFFECT-002
 */

#ifndef MACHINEINFO_H_
#define MACHINEINFO_H_

#include "Dialog.h"

class MachineInfoDialog: public Dialog {
public:
	MachineInfoDialog(HWND handle);
	~MachineInfoDialog(void);

	static INT_PTR CALLBACK handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);
	static INT_PTR create(HWND parent);

	static bool saveMachineInfo(const TCHAR *filePath);
	static bool loadMachineInfo(const TCHAR *filePath);
	static bool itemOutput_MachineInfo(HWND hwnd);
	static bool getParameterText(char *pBuff, TCHAR *pParam);

private:
	enum {
		PARAM_INSPECTOR = 0,
		PARAM_INSPECTOR_ADD,
		PARAM_DRONE_TYPE,
		PARAM_DRONE_NUMBER,
		PARAM_DRONE_DATE,
		PARAM_CAMERA_TYPE,
		PARAM_CAMERA_NUMBER,
		PARAM_CAMERA_DATE,
	};

};

#endif /* MACHINEINFO_H_ */
