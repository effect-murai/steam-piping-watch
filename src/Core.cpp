/*
 * Fatal.cpp
 *
 *  Created on: 2020/06/17
 *      Author: k.tasaki
 */

#include <Core.h>
#include <windows.h>
#include "app.h"
#include "resource.h"
#include "StringUtils.h"
#include "MainWindow.h"

extern MainWindow *mainWindow;

void* allocateMemory(size_t size, const char *file, int line) {
	void *address = malloc(size);
	if (address == NULL) {
		TCHAR *title = new TCHAR[Resource::MAX_LOADSTRING];
		Resource::getString(IDS_ERROR, title);
		TCHAR *filePath = fromOEMCodePageString(file);
		TCHAR *message = new TCHAR[Resource::MAX_LOADSTRING + lstrlen(filePath)];
		stprintf(message, TEXT("メモリ確保エラーが発生しました。\n%s:%d\nアプリケーションを終了します。"),
				filePath, line);
		delete filePath;
		HWND owner = NULL;
		if (mainWindow != NULL) {
			owner = mainWindow->getHandle();
		}
		MessageBox(owner, message, title, MB_OK | MB_ICONERROR);
		delete title;
		delete message;

		exit(1);
	}
	return address;
}
