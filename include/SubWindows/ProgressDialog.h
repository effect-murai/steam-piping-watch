#ifndef _PROGRESS_DIALOG_H
#define _PROGRESS_DIALOG_H

#include <Dialog.h>
#include <Resource.h>
#include <Common/Future.h>

#define ID_UPDATE_TIMER 1
#define UPDATE_TIMER_INTERVAL 100

class ProgressDialog: public Dialog {
protected:
	ProgressDialog(HWND handle);

	INT_PTR onInitialize(WPARAM wParam, LPARAM lParam);

	INT_PTR onClose(WPARAM wParam, LPARAM lParam);

	virtual INT_PTR CALLBACK handleEvent(UINT uMsg, WPARAM wParam,
			LPARAM lParam);

public:
	static INT_PTR CALLBACK handleEvent(HWND hwndDlg, UINT uMsg, WPARAM wParam,
			LPARAM lParam);

	template<typename Type>
	static INT_PTR show(Future<Type> &future) {
		return Resource::showDialog(IDD_PROGRESS, NULL,
				ProgressDialog::handleEvent, (LONG_PTR) &future);
	}

private:
	Future<void*> *result;
};

#endif /* _PROGRESS_DIALOG_H */
