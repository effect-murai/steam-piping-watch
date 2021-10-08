//============================================================================
// Name        : Drone.cpp
// Author      : a
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <windows.h>
#include "MainWindow.h"

// Windowsアプリケーションのエントリポイント
int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdline,
		int cmdshow) {
	MSG msg;

	// 初期化処理(アプリ)
	Initialize(instance);

	// メッセージループ
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 終了処理
	Finalize();

	return 0;
}
