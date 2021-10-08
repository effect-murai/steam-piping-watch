/*
 * CustomerInfo.h
 *
 *  Created on: 2016/02/15
 *      Author: PC-EFFECT-002
 */

#ifndef CUSTOMERINFO_H_
#define CUSTOMERINFO_H_

#include "Dialog.h"

class CustomerInfoDialog: public Dialog {
public:
	CustomerInfoDialog(HWND handle);
	~CustomerInfoDialog(void);

	static INT_PTR CALLBACK handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);
	static INT_PTR create(HWND parent);

	static bool saveCustomerInfo(const TCHAR *filePath);
	static bool loadCustomerInfo(const TCHAR *filePath);
	static bool itemOutput_CustomerInfo(HWND hwnd);
	static bool getParameterText(char *pBuff, TCHAR *pParam);
	static bool getParameterInt(char *pBuff, int *pParam);
	static bool getParameterDate(char *pBuff, SYSTEMTIME *pDate);

private:
	enum {
		PARAM_CUSTOMER_NAME = 0,
		PARAM_ADDRESS,
		PARAM_PLANT_NAME,
		PARAM_PLANT_OUTPUT,
		PARAM_PANEL_COUNT,
		PARAM_PANEL_TYPE,
		PARAM_PANEL_MAKER,
		PARAM_WEATHER,
		PARAM_FLIGHT_PERSON,
		PARAM_PATROL_PERSON,
		PARAM_SETTING_DATE
	};

	enum {
		DATE_YEAR = 0, DATE_MONTH, DATE_DAY
	};

};
#endif /* CUSTOMERINFO_H_ */
