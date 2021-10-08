/*
 * ProgressBar.cpp
 *
 *  Created on: 2015/11/20
 *      Author: PC-EFFECT-012
 */

#include <Windows.h>
#include <commctrl.h>
#include "resource.h"
#include "app.h"

#include "Dialog.h"
#include "SubWindows/ProgressBar.h"

#include <stdio.h>

#define ProgressBar_SetPos(hwnd, value) SNDMSG((hwnd), PBM_SETPOS, (value), 0)
#define ProgressBar_GetPos(hwnd) SNDMSG((hwnd), PBM_SETPOS, 0, 0)

#define ID_UPDATE_TIMER 1
#define UPDATE_TIMER_INTERVAL 100
#define PROGRESS_VALUE_MAX 100

//------------------------------------------------------------------------------
// Class Functions
//------------------------------------------------------------------------------

/**
 * コンストラクタ
 * @param[in] hwnd ウィンドウのハンドル
 */
ProgressBar::ProgressBar(HWND hwnd) :
		Dialog(hwnd) {
	this->progress = NULL;
	this->type = DEFAULT;
	this->timerCount = 0;
}

ProgressBar::~ProgressBar(void) {
}

/**
 * ダイアログの初期化処理
 * @param initParam パラメータを格納した構造体へのポインタ
 */
void ProgressBar::initDialog(Parameters *initParam) {

	this->progress = &initParam->progress;
	this->type = initParam->type;
	this->timerCount = 0;

	SetTimer(this->getHandle(), ID_UPDATE_TIMER, UPDATE_TIMER_INTERVAL, NULL);
	this->updateTitle();
	if (initParam->canClose == false) {
		this->unsetStyle(WS_SYSMENU);
	}
}

/**
 * ダイアログの終了処理
 */
void ProgressBar::closeDialog(void) {
	TCHAR message[Resource::MAX_LOADSTRING];
	TCHAR title[Resource::MAX_LOADSTRING];
	switch (this->type) {
	case HOTSPOT_DETECTION:
		Resource::getString(IDS_STOP_HOTSPOT_DETECT, message);
		Resource::getString(IDS_HOTSPOT_DETECTION_TITLE, title);
		break;

	default:
		Resource::getString(IDS_CANCEL_PROCESSING, message);
		Resource::getString(IDS_PROCESSING_TITLE, title);
		break;
	}

	if (MessageBox(this->getHandle(), message, title, MB_YESNO) == IDYES) {
		KillTimer(this->getHandle(), ID_UPDATE_TIMER);
		this->close(IDCANCEL);
	}
}

/**
 * タイトルの更新処理
 */
void ProgressBar::updateTitle(void) {
	TCHAR title[Resource::MAX_LOADSTRING];
	switch (this->type) {
	case HOTSPOT_DETECTION:
		Resource::getString(IDS_DETECTING_HOTSPOT, title);
		break;

	case LOADING:
		Resource::getString(IDS_LOADING, title);
		break;

	case CHANGING:
		Resource::getString(IDS_CHANGING, title);
		break;

	case CREATING_WHOLE_PICTURE:
		Resource::getString(IDS_CREATING_WHOLE_PICTURE, title);
		break;

	case UPDATING_HOTSPOT_INFO:
		Resource::getString(IDS_UPDATING_HOTSPOT_INFO, title);
		break;

	case WRITING_HOTSPOT_DETAIL:
		Resource::getString(IDS_WRITING_HOTSPOT_DETAIL, title);
		break;

	case DETECTING_KEYPOINT:
		Resource::getString(IDS_DETECTING_KEYPOINT, title);
		break;

	case PANEL_TEMPANALYSIS:
		Resource::getString(IDS_PANELTEMP_ANALYSIS, title);
		break;

	default:
		Resource::getString(IDS_PROCESSING_TITLE, title);
		break;
	}

	int max = PROGRESS_VALUE_MAX;
	int value = ((double) (*progress) / max) * PROGRESS_VALUE_MAX;
	TCHAR text[Resource::MAX_LOADSTRING * 2 + 10];
	if (value < 10) {
		if (this->type == CHANGING) {
			int remaining = timerCount / 10;
			stprintf(text, Resource::getString(IDS_PROGRESSBAR_TITLE3), title,
					value, remaining);
		} else {
			stprintf(text, Resource::getString(IDS_PROGRESSBAR_TITLE), title,
					value);
		}
	} else {
		int remaining = (max - value) * timerCount / 10 / value;
		stprintf(text, Resource::getString(IDS_PROGRESSBAR_TITLE2), title,
				value, remaining);
	}
	this->setText(text);
}

/**
 * プログレスバーの更新処理
 */
void ProgressBar::updateProgressBar(void) {
	ProgressBar_SetPos(this->getDlgItem(IDC_PROGRESS), *this->progress);
	this->timerCount++;
	this->updateTitle();
	if (*this->progress == PROGRESS_VALUE_MAX) {
		KillTimer(this->getHandle(), ID_UPDATE_TIMER);
		this->close(IDOK);
	}
}
//------------------------------------------------------------------------------
// Event Handler
//------------------------------------------------------------------------------
/**
 * ダイアログのイベント処理用コールバック関数
 * @param hwndDlg ダイアログボックスのハンドル
 * @param uMsg メッセージ
 * @param wParam 最初のメッセージパラメータ
 * @param lParam 2 番目のメッセージパラメータ
 * @return メッセージを処理した場合に 0 以外の値（TRUE）を、処理しなかった場合に 0（FALSE）を返す。@n
 * 0（FALSE）を返した場合、既定のダイアログ処理が実行される。
 */
INT_PTR CALLBACK ProgressBar::handleEvent(HWND hwndDlg, UINT uMsg,
		WPARAM wParam, LPARAM lParam) {
	ProgressBar *target = (ProgressBar*) fromHandle(hwndDlg);
	if (target == NULL) {
		if (uMsg == WM_INITDIALOG) {
			target = new ProgressBar(hwndDlg);
		} else {
			return (INT_PTR) FALSE;
		}
	}
	switch (uMsg) {
	case WM_INITDIALOG:
		target->initDialog((Parameters*) lParam);
		return (INT_PTR) TRUE;
	case WM_DESTROY:
		delete target;
		break;
	case WM_CLOSE:
		target->closeDialog();
		return (INT_PTR) TRUE;
	case WM_COMMAND:
		break;
	case WM_TIMER:
		target->updateProgressBar();
		return (INT_PTR) TRUE;
	}
	return (INT_PTR) FALSE;
}

//------------------------------------------------------------------------------
// Static Class Functions
//------------------------------------------------------------------------------

INT_PTR ProgressBar::create(Window *parent, int type, StartRoutine proc,
		void (*succeeded)(void*)) {
	return create(parent, type, proc, succeeded, NULL);
}

INT_PTR ProgressBar::create(Window *parent, int type, StartRoutine proc,
		void (*succeeded)(void*), void *data) {
	class {
	public:
		static DWORD __stdcall proc(void *arg) {
			void **thParam = (void**) arg;
			ProgressBar::Parameters *param =
					(ProgressBar::Parameters*) thParam[0];
			ProgressBar::StartRoutine routine =
					(ProgressBar::StartRoutine) thParam[1];
			int retVal = routine(param);
			param->progress = PROGRESS_VALUE_MAX;
			return retVal;
		}
	} th;

	ProgressBar::Parameters param;
	ZeroMemory(&param, sizeof(param));
	param.type = type;
	param.data = data;
	switch (type) {
	case CREATING_WHOLE_PICTURE:
	case UPDATING_HOTSPOT_INFO:
	case WRITING_HOTSPOT_DETAIL:
	case PANEL_TEMPANALYSIS:
		// 閉じれないプログレスウィンドウ
		param.canClose = false;
		break;
	default:
		param.canClose = true;
		break;
	}

	void *thParam[2] = { (void*) &param, (void*) proc };

	// スレッドを生成
	DWORD threadId = 0;
	HANDLE thread = CreateThread(NULL, 0, th.proc, &thParam, 0, &threadId);

	// (2017/8/29YM)フリーズして止まっている箇所↓
	INT_PTR ret = Resource::showDialog(IDD_PROGRESS, parent->getHandle(),
			handleEvent, (LPARAM) &param);

	// スレッドとの同期を取る
	if (param.progress < PROGRESS_VALUE_MAX) {
		// 完了していない場合はスレッドを強制停止させる
		param.forceStop = 1;
	} else if (succeeded != NULL) {
		succeeded(&param);
	}
	WaitForSingleObject(thread, INFINITE);

	// スレッドを閉じる
	CloseHandle(thread);

	return ret;
}
