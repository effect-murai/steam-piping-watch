/*
 * ProgressBar.h
 *
 *  Created on: 2016/02/01
 *      Author: PC-EFFECT-012
 */

#ifndef PROGRESSBAR_H_
#define PROGRESSBAR_H_

#include "Dialog.h"

class ProgressBar: public Dialog {
public:
	typedef struct {
		int type;
		int progress;
		int forceStop;
		void *data;
		bool canClose;
	} Parameters;

	typedef enum {
		HOTSPOT_DETECTION = 0,
		LOADING,
		CREATING_WHOLE_PICTURE,
		UPDATING_HOTSPOT_INFO,
		WRITING_HOTSPOT_DETAIL,
		DETECTING_KEYPOINT,			// (2017/2/6YM追加)特徴点検出ダイアログ追加
		PANEL_TEMPANALYSIS,			// (2017/5/26YM)パネル平均温度算出ダイアログ追加
		DEFAULT,
		CHANGING
	} Type;

	typedef DWORD (*StartRoutine)(Parameters*);

	static INT_PTR create(Window *parent, int type, StartRoutine proc,
			void (*succeeded)(void*));
	static INT_PTR create(Window *parent, int type, StartRoutine proc,
			void (*succeeded)(void*), void *data);
	~ProgressBar(void);
private:
	ProgressBar(HWND handle);
	static INT_PTR CALLBACK handleEvent(HWND hwndDlg, UINT uMsg, WPARAM wParam,
			LPARAM lParam);
	void updateTitle(void);
	void initDialog(Parameters *initParam);
	void closeDialog(void);
	void updateProgressBar(void);
	int *progress;
	int type;
	int timerCount;
};

#endif /* PROGRESSBAR_H_ */
