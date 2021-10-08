/*
 * DetectSetting.cpp
 *
 *  Created on: 2016/01/29
 *      Author: PC-EFFECT-012
 */

#include <stdio.h>

#include "app.h"
#include "resource.h"
#include "Dialog.h"
#include "MainWindow.h"
#include "SubWindows/DetectSetting.h"

#include "Data/ResultData.h"
extern ResultData *result;

#define TrackBar_SetTickFrequency(hwnd, freq) SNDMSG(hwnd, TBM_SETTICFREQ, freq, 0)
#define TrackBar_SetRangeMin(hwnd, value, redrawFlag) SNDMSG(hwnd, TBM_SETRANGEMIN, redrawFlag, value)
#define TrackBar_SetRangeMax(hwnd, value, redrawFlag) SNDMSG(hwnd, TBM_SETRANGEMAX, redrawFlag, value)
#define TrackBar_SetPos(hwnd, value, redrawFlag) SNDMSG(hwnd, TBM_SETPOS, redrawFlag, value)
#define TrackBar_GetPos(hwnd) SNDMSG(hwnd, TBM_GETPOS, 0, 0)

#define SUMMER 40.0
#define WINTER 20.0

inline double toTemp(float value) {
	return (value * (100 - 0) / 200);
}

//------------------------------------------------------------------------------
// Class Functions
//------------------------------------------------------------------------------

/**
 * コンストラクタ
 * @param[in] hwnd ウィンドウのハンドル
 */
DetectSettingDialog::DetectSettingDialog(HWND handle) :
		Dialog(handle) {
}

/**
 * デストラクタ
 */
DetectSettingDialog::~DetectSettingDialog() {
}

INT_PTR DetectSettingDialog::initDialog() {
	const double RANGE_MAX = 100;

	//　小数点以下１桁まで表示
	TCHAR *text;
	text = new TCHAR[128];
	stprintf(text, TEXT("%.1lf"), RANGE_MAX);
	SetWindowText(this->getDlgItem(IDC_DETECT_TEMP_MAX), text);
	delete text;

	const int tickCelcius = 3;
	const int tickFreq = (int) floor(200 / RANGE_MAX * tickCelcius + 0.5);
	TrackBar_SetTickFrequency(this->getDlgItem(IDC_DETECT_TEMP_SLIDER),
			tickFreq);
	TrackBar_SetRangeMin(this->getDlgItem(IDC_DETECT_TEMP_SLIDER), 0, FALSE);
	TrackBar_SetRangeMax(this->getDlgItem(IDC_DETECT_TEMP_SLIDER), 200, FALSE);
	// 最高温度と最低温度の温度差から初期値を求める
	double ondosa = (result->getTempMax() - result->getTempMin());
	double shokival = ((ondosa / 10) / 100) * 200;
	TrackBar_SetPos(this->getDlgItem(IDC_DETECT_TEMP_SLIDER), shokival, TRUE);
	return (INT_PTR) TRUE;
}

//------------------------------------------------------------------------------
// Event Handler
//------------------------------------------------------------------------------

INT_PTR CALLBACK DetectSettingDialog::handleEvent(HWND hwndDlg, // ダイアログボックスのハンドル
		UINT uMsg,     // メッセージ
		WPARAM wParam, // 最初のメッセージパラメータ
		LPARAM lParam  // 2 番目のメッセージパラメータ
		) {
	DetectSettingDialog *target = (DetectSettingDialog*) fromHandle(hwndDlg);
	if (target == NULL) {
		if (uMsg == WM_INITDIALOG) {
			target = new DetectSettingDialog(hwndDlg);
		} else {
			return (INT_PTR) FALSE;
		}
	}

	UINT wmId = LOWORD(wParam);
	float value = 0;

	switch (uMsg) {
	case WM_INITDIALOG:
		return target->initDialog();
	case WM_DESTROY:
		delete target;
		break;
	case WM_CLOSE:
		target->close(0);
		break;
	case WM_NOTIFY:
		switch (wmId) {
		case IDC_DETECT_TEMP_SLIDER:
			value = TrackBar_GetPos(target->getDlgItem(IDC_DETECT_TEMP_SLIDER));
			TCHAR text[128];
			ZeroMemory(text, sizeof(text));
			stprintf(text, TEXT("%.1lf"), toTemp(value));
			SetWindowText(target->getDlgItem(IDC_DETECT_TEMP_EDIT), text);
			break;
		}
		break;
	case WM_COMMAND:
		switch (wmId) {
		case IDC_DETECT_OK:
			value = TrackBar_GetPos(target->getDlgItem(IDC_DETECT_TEMP_SLIDER));
			//　パネル検出モードを設定する。
			result->setPanelDetection(
					(bool) IsDlgButtonChecked(target->getHandle(),
					IDC_DETECT_PANEL));

			target->close(value);
			break;
		case IDC_SUMMER_TEMP:
			TrackBar_SetPos(target->getDlgItem(IDC_DETECT_TEMP_SLIDER), SUMMER,
					TRUE);
			break;
		case IDC_WINTER_TEMP:
			TrackBar_SetPos(target->getDlgItem(IDC_DETECT_TEMP_SLIDER), WINTER,
					TRUE);
			break;

		case IDC_DETECT_CANCEL:
			target->close(0);
			break;
		}
		return (INT_PTR) TRUE;
	}
	return (INT_PTR) FALSE;
}

//------------------------------------------------------------------------------
// Static Class Functions
//------------------------------------------------------------------------------

/**
 * 検出設定ダイアログを表示する。
 * @param parent 親ウィンドウのハンドル
 *
 */
INT_PTR DetectSettingDialog::create(HWND parent) {
	return Resource::showDialog(IDD_DETECT_HOTSPOT, parent, handleEvent);
}
