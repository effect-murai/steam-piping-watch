/*
 * MainWindow.cpp
 *
 *  Created on: 2017/10/27
 *      Author: PC-EFFECT-012
 */

// (2017/6/13YM)特徴点検出最大数
#define MAXKP 1

// Windows APIのヘッダをインクルード
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shlobj.h>
#include <shlwapi.h>

#include "MainWindow.h"
#include "ResultData.h"
#include "Graphics.h"
#include "HotspotLinker.h"
#include "PanelInfo.h"
#include "Controls.h"

#include "Mutex.h"
#include <iostream>
#include <list>
#include "resource.h"
#include <pthread.h>
#include <unistd.h>

#include "Dialog.h"
#include "SubWindows/DetectSetting.h"
#include "SubWindows/ProgressBar.h"
#include "SubWindows/PanelSetting.h"
#include "SubWindows/CustomerInfo.h"
#include "SubWindows/MachineInfo.h"
#include "SubWindows/OutputCsvInfo.h"
#include "SubWindows/PictureWindow.h"
#include "SubWindows/TrialInfomation.h"

#include "app.h"
#include "FileUtils.h"
#include "StringUtils.h"
#include <Core.h>
#include "Protection.h"
#include "LicenseManager.h"
#include <SubWindows/ProgressDialog.h>

#include <Data/TiffConverter.h>

#include "debug.h"

// (2017/2/2/YM)特徴点検出アルゴリズムヘッダーインクルード
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>

// 暫定
#ifdef UNICODE
typedef std::wstring String;
typedef std::wstringstream StringStream;
#else
typedef std::string String;
typedef std::stringstream StringStream;
#endif

//　(2017/4/6YM)const宣言追加
enum {
	PICTURE_POS_NORTH = 0,
	PICTURE_POS_EAST,
	PICTURE_POS_SOUTH,
	PICTURE_POS_WEST,
	PICTURE_POS_CENTER,
	PICTURE_POS_COUNT
};

// (2017/4/4YM)キー入力判定マクロ追加
#define isKeyDown(keycode) ((GetKeyState(keycode) & 0x8000) != 0)

//------------------------------------------------------------------------------
// Emulation Options (For debugging)
//------------------------------------------------------------------------------
//#define EMULATE_LOWRESOLUTION_DISPLAY
//#define EMULATE_MIDDLE_SIZE_FONT
//#define EMULATE_LARGE_SIZE_FONT
#if defined(EMULATE_LARGE_SIZE_FONT)
#define EMULATE_FONT_SIZE	1.5
#elif defined(EMULATE_MIDDLE_SIZE_FONT)
#define EMULATE_FONT_SIZE	1.25
#endif

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------
// Instance of the Main Window.
MainWindow *mainWindow = NULL;

// Instance of a result data.
ResultData *result = NULL;

// Instance of a hot spot linker.
HotspotLinker *hotspotLinker = NULL;

// Instance of a PanelInfo
PanelData *panelData = NULL;

// (2020/01/08LEE)
StatusPanel *statuspanel = NULL;

// For GDI+ library
Gdiplus::GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;

// Main Window class name
LPCTSTR MainWndClassName = TEXT("MainWindow");

// ライセンスキー登録有無
bool licenseKeyRegistered = false;

// Activation結果
bool ActivationResult = false;

//------------------------------------------------------------------------------
// External Global Functions
//------------------------------------------------------------------------------

extern void openReport(TCHAR *pFilePath);

// (2017/4/6YM)インライン関数宣言
//------------------------------------------------------------------------------
// インライン関数
//------------------------------------------------------------------------------
inline int getAroundPictureId(int id, int directionId) {
	if (directionId < PICTURE_POS_CENTER) {
		return result->getAroundPictureId(id, directionId);
	} else {
		return id;
	}
}

//------------------------------------------------------------------------------
// Static Class Functions
//------------------------------------------------------------------------------

/**
 * メインウィンドウのインスタンス
 */
MainWindow* MainWindow::getInstance(void) {
	if (mainWindow == NULL) {
		new MainWindow();
	}
	return mainWindow;
}

//------------------------------------------------------------------------------
// Global Functions
//------------------------------------------------------------------------------

/**
 * ライセンスキー登録有無の更新
 */
void setLicenseKeyRegistered(bool Registered) {
	// ライセンスキー登録有無を更新
	licenseKeyRegistered = Registered;
}

/**
 * ライセンスキー登録有無の取得
 */
bool getLicenseKeyRegistered(void) {
	// ライセンスキー登録有無
	return licenseKeyRegistered;
}

/**
 * アクティベーション結果の更新
 */
void setActivationResult(bool Activation) {
	// アクティベーション結果を更新
	ActivationResult = Activation;
}

/**
 * アクティベーション結果の取得
 */
bool getActivationResult(void) {
	// アクティベーション結果を返す
	return ActivationResult;
}

/**
 * アプリケーションの初期化
 * @param[in] hInst インスタンスハンドル
 */
void Initialize(HINSTANCE hInst) {
	// Initialize GDI+.
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	if (!checkInstallKey()) {
		// ライセンスキー未登録
		showTrialInfomationDialog();
	} else {
		// ライセンスキー登録済
		setLicenseKeyRegistered(true);
	}

	if (getLicenseKeyRegistered()) {
		// 認証チェック実行
		if (checkLicense()) {
			// 認証チェック成功
			setActivationResult(true);
		} else {
			// 認証チェック失敗
			setActivationResult(false);
		}
	}
	MainWindow::getInstance();
}

/**
 * アプリケーションの後片付け
 */
void Finalize(void) {
	delete mainWindow;
	mainWindow = NULL;
	if (hotspotLinker != NULL) {
		// 破棄する
		delete hotspotLinker;
		hotspotLinker = NULL;
	}
	if (panelData != NULL) {
		// 破棄する
		delete panelData;
		panelData = NULL;
	}

	if (result != NULL) {
		// 保存後、破棄する
		delete result;
		result = NULL;
	}
	Gdiplus::GdiplusShutdown(gdiplusToken);
}

/**
 * フォルダ選択ダイアログを表示@n
 * この関数から返されるポインタ値はfree関数で解放してください。
 * @param[in] hWnd ダイアログの親ウィンドウ
 * @param[in] ダイアログに表示する文字列
 * @return 選択したフォルダのパス
 * @todo 要確認:SHGetPathFromIDListのリソース解放処理
 */
LPTSTR showSelectFolder(HWND hWnd, LPCTSTR Msg) {
	BROWSEINFO bInfo;
	LPTSTR pPath;
	int result, length;
	ITEMIDLIST *idList;
	ZeroMemory(&bInfo, sizeof(BROWSEINFO));
	bInfo.hwndOwner = hWnd;
	bInfo.pidlRoot = 0;
	bInfo.lpszTitle = Msg;
	bInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
	idList = SHBrowseForFolder(&bInfo);

	pPath = allocMemory(TCHAR, MAX_PATH * sizeof(TCHAR));

	ZeroMemory(pPath, MAX_PATH * sizeof(TCHAR));
	result = SHGetPathFromIDList(idList, pPath);

	if (result != 0) {
		length = lstrlen(pPath);
		return (LPTSTR) realloc(pPath, (length + 1) * sizeof(TCHAR));
	} else {
		free(pPath);
		return NULL;
	}
}

/**
 * ファイルを開くダイアログを表示@n
 * この関数から返されるポインタ値はfree関数で解放してください。
 * @param[in] parent ダイアログの親ウィンドウ
 * @param[in] filter フィルタ
 * @param[in] defaultExt デフォルト拡張子
 * @return 選択したファイルのパス
 */
LPTSTR showOpenFile(HWND parent, LPCTSTR filter, LPCTSTR defaultExt) {
	OPENFILENAME ofDialog;
	LPTSTR fileName = allocMemory(TCHAR, MAX_PATH * sizeof(TCHAR));

	ZeroMemory(fileName, MAX_PATH * sizeof(TCHAR));
	ZeroMemory(&ofDialog, sizeof(OPENFILENAME));
	ofDialog.hwndOwner = parent;
	ofDialog.lpstrFilter = filter;
	ofDialog.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;
	ofDialog.lStructSize = sizeof(OPENFILENAME);
	ofDialog.lpstrFile = fileName;
	ofDialog.nMaxFile = MAX_PATH - 1;
	ofDialog.lpstrDefExt = defaultExt;
	if (defaultExt != NULL) {
		ofDialog.Flags |= OFN_EXTENSIONDIFFERENT;
	}
	if (GetSaveFileName(&ofDialog) == 0) {
		free(fileName);
		return NULL;
	}

	const int length = lstrlen(fileName);
	return (LPTSTR) realloc(fileName, (length + 1) * sizeof(TCHAR));
}

/**
 * ファイルを選択するダイアログを表示
 * この関数から返されるポインタ値はfree関数で解放してください。
 * @param[in] parent ダイアログの親ウィンドウ
 * @param[in] filter フィルタ
 * @param[in] defaultExt デフォルト拡張子
 * @return 選択したファイルのパス
 */
LPTSTR showSelectFile(HWND parent, LPCTSTR filter, LPCTSTR defaultExt) {
	OPENFILENAME ofDialog;
	LPTSTR fileName = allocMemory(TCHAR, MAX_PATH * sizeof(TCHAR));

	ZeroMemory(fileName, MAX_PATH * sizeof(TCHAR));
	ZeroMemory(&ofDialog, sizeof(OPENFILENAME));
	ofDialog.hwndOwner = parent;
	ofDialog.lpstrFilter = filter;
	ofDialog.Flags = OFN_EXPLORER | OFN_READONLY;
	ofDialog.lStructSize = sizeof(OPENFILENAME);
	ofDialog.lpstrFile = fileName;
	ofDialog.nMaxFile = MAX_PATH - 1;
	ofDialog.lpstrDefExt = defaultExt;
	if (GetOpenFileName(&ofDialog) == 0) {
		free(fileName);
		return NULL;
	}

	const int length = lstrlen(fileName);
	return (LPTSTR) realloc(fileName, (length + 1) * sizeof(TCHAR));
}

inline double angle(POINT pt1, POINT pt2, POINT pt0) {
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1 * dx2 + dy1 * dy2)
			/ sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2));
}

/**
 * 未実装の機能を示す関数
 * @param handle ウィンドウハンドル
 * @param uMsg メッセージID
 * @param wParam メッセージパラメータ
 * @param lParam メッセージパラメータ
 */
void UnimplementedFunction(HWND handle, UINT uMsg, WPARAM wParam,
		LPARAM lParam) {
	TCHAR *const title = new TCHAR[Resource::MAX_LOADSTRING];
	TCHAR *const message = new TCHAR[Resource::MAX_LOADSTRING];
	Resource::getString(IDS_UNIMPLEMENTED, title);
	Resource::getString(IDS_SORRY, message);
	MessageBox(handle, title, message, MB_OK);
	delete title;
	delete message;
}

inline void decimalToMinSec(double decimal, int *integer, int *minute,
		double *second) {
	int _integer = floor(decimal);
	double _dec = decimal - _integer;
	double _minsec = _dec * 3600;
	int _minute = floor(_minsec / 60);
	double _second = _minsec - _minute * 60;

	*integer = _integer;
	*minute = _minute;
	*second = _second;
}

/**
 * 無効化されたホットスポットがあるかどうか確認する
 * @return true:あり/false:なし
 */
bool existsDisabledHotspot(void) {
	const int DATA_COUNT = result->getDataCount();
	for (int picNo = 0; picNo < DATA_COUNT; picNo++) {
		const int HOTSPOT_COUNT = result->getHotspotCount(picNo);
		for (int ptNo = 0; ptNo < HOTSPOT_COUNT; ptNo++) {
			if (result->isDisabledHotspot(picNo, ptNo)) {
				return true;
			}
		}
	}
	return false;
}

/**
 * 無効化されたホットスポットを削除する
 */
void removeDisabledHotspots(void) {
	const int DATA_COUNT = result->getDataCount();
	for (int picNo = 0; picNo < DATA_COUNT; picNo++) {
		const int HOTSPOT_COUNT = result->getHotspotCount(picNo);
		for (int ptNo = HOTSPOT_COUNT - 1; ptNo >= 0; ptNo--) {
			if (result->isDisabledHotspot(picNo, ptNo)) {
				// 無効化したホットスポットを自動的に削除する
				hotspotLinker->removeHotspot(picNo, ptNo);
			}
		}
	}

	// 保存する
	result->saveHotspot();
	hotspotLinker->saveTo(result->getHotspotLinkFileName());
}

//------------------------------------------------------------------------------
// Message Handler
//------------------------------------------------------------------------------

/**
 * メインウィンドウのイベントを処理するためのコールバック関数
 */
LRESULT CALLBACK MainWindow::handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam) {
	MainWindow *target;
	if (uMsg == WM_CREATE) {
		// ウィンドウ生成時にウィンドウハンドルとウィンドウオブジェクトへのポインタを関連付ける
		target = (MainWindow*) ((LPCREATESTRUCT) lParam)->lpCreateParams;
		Window::registration(hWnd, target);
	} else {
		target = (MainWindow*) fromHandle(hWnd);
	}

	if (target != NULL) {
		LRESULT ret;
		RECT rect;
		switch (uMsg) {
		case WM_CREATE:
			// CreateWindow呼び出し直後
			break;

		case WM_DESTROY:
			// ウィンドウが破棄されたときの処理
			target->saveAll();
			PostQuitMessage(0);
			break;

		case WM_SIZE:
			// CreateWindow呼び出し直後にも送られてくるため要注意
			target->getWindowRect(&rect);
			target->onResize(wParam | MainWindow::FLAG_RESIZED, &rect);
			break;

		case WM_SIZING:
			target->onResize(wParam, (RECT*) lParam);
			return TRUE;

		case WM_CTLCOLORSTATIC:
			// 背景・文字色などの変更処理
			ret = target->onColorCtrl((HDC) wParam, (HWND) lParam);
			if (ret != 0) {
				return ret;
			}
			// 戻り値が0の場合はデフォルト処理
			break;

		case WM_COMMAND:
			// ボタンクリックなどの動作
			target->onCommand(uMsg, wParam, (HWND) lParam);
			break;
		case WM_KEYDOWN:
		case WM_KEYUP:
			// キー入力処理
			target->onKey(uMsg, wParam);
			break;
		case WM_DRAWITEM:
			// オーナードロー型コントロールの再描画
			target->onDrawItem(uMsg, wParam, (LPDRAWITEMSTRUCT) lParam);
			break;
		case WM_GETMINMAXINFO:
			// CreateWindowの処理中にも送られてくるため要注意
			// サイズ制限情報
			((MINMAXINFO*) lParam)->ptMinTrackSize.x = target->getMinimalSize()
					+ 340; // 最小幅
			((MINMAXINFO*) lParam)->ptMinTrackSize.y = target->getMinimalSize(); // 最小高
			return 0; // 処理したら0を返す
		case WM_MOUSEWHEEL:
			return target->onMouseWheel(uMsg, wParam, lParam);
		}
	} else {
		// WM_GETMINMAXINFO 36
		// WM_NCCREATE 129
		// WM_NCCALCSIZE 131
		debug("Unhandled event: %d", uMsg);
	}
	// デフォルトのイベント処理
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// キャンバスのイベントを処理するためのコールバック関数
LRESULT CALLBACK CanvasProc(HWND hWnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam) {
	MainWindow *parent = (MainWindow*) Window::fromHandle(GetParent(hWnd));
	if (parent != NULL) {
		return parent->CanvasProc(hWnd, uMsg, wParam, lParam);
	}

	// デフォルトのイベント処理
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// MainWindow Class
//------------------------------------------------------------------------------

/**
 * メインウィンドウを作成する。
 */
MainWindow::MainWindow(void) :
		Window() {
	mainWindow = this;
	hUIFont = NULL;
	selectedPictureId = NOT_SELECTED;
	selectedHotspotPictureId = NOT_SELECTED;
	selectedHotspotPointId = NOT_SELECTED;
	controlMode = MainWindow::CONTROL_PANEL_MODE_DEFAULT;
	blMouse = FALSE;
	HotSpotAreas.clear();
	HotSpotAreaRgn = NULL;
	isAreaMove = false;
	isAreaComplete = false;
	resetView();
	csvData = new CsvData();
	panelSettingData = new PanelSettingData();
	areaSettingData = NULL;
	isHotspotUpdated = false;
	hotspotShowMode = true;
	hotspotCandidateShowMode = true;
	disabledHotspotShowMode = false;
	keypointShowMode = true;
	borderSize = 0;
	// (2017/6/1YM)閾値データ初期化
	thresholdtemp = -1;
	//----------------------------------------

	// ウィンドウクラスを登録
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC) handleEvent;
	wcex.hInstance = GetModuleHandle(NULL);
	wcex.hIcon = Resource::getIcon(IDI_APP_ICON);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName = MainWndClassName;
	wcex.hIconSm = Resource::getIcon(IDI_APP_ICON);
	wcex.hbrBackground = (HBRUSH) COLOR_WINDOW;
	RegisterClassEx(&wcex);

	//----------------------------------------

	// ウィンドウを仮サイズで作成
	int width = 320, height = 240;
	if (getActivationResult()) {
		CreateWindowEx(0, MainWndClassName, Resource::getString(IDS_APP_TITLE),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height,
		NULL, NULL, GetModuleHandle(NULL), this);
	} else {
		CreateWindowEx(0, MainWndClassName,
				Resource::getString(IDS_APP_TITLE_TRIAL),
				WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width,
				height,
				NULL, NULL, GetModuleHandle(NULL), this);
	}

	//----------------------------------------

	// 画面にあわせてウィンドウサイズを変更する

	// デスクトップサイズを取得する
	RECT desktopRect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &desktopRect, 0);

#ifdef EMULATE_LOWRESOLUTION_DISPLAY
	//----------------------------------------
	// 擬似低解像度ディスプレイ(1366x768)
	//----------------------------------------
#define EMULATION_DISPLAY_WIDTH  1366
#define EMULATION_DISPLAY_HEIGHT 768
	RECT currentDesktopRect;
	GetClientRect(GetDesktopWindow(), &currentDesktopRect);
	desktopRect.right -= currentDesktopRect.right - EMULATION_DISPLAY_WIDTH;
	desktopRect.bottom -= currentDesktopRect.bottom - EMULATION_DISPLAY_HEIGHT;
	//----------------------------------------
#endif // EMULATE_LOWRESOLUTION_DISPLAY

	// ウィンドウサイズを取得
	RECT windowRect, clientRect;
	this->getWindowRect(&windowRect);
	this->getClientRect(&clientRect);

	// ウィンドウの幅と高さを計算
	const int windowWidth = windowRect.right - windowRect.left;
	const int windowHeight = windowRect.bottom - windowRect.top;

	// ウィンドウ内部の幅と高さを計算
	const int clientWidth = clientRect.right - clientRect.left;
	const int clientHeight = clientRect.bottom - clientRect.top;

	// ウィンドウ枠の幅と高さを計算
	const int borderWidth = windowWidth - clientWidth;
	const int borderHeight = windowHeight - clientHeight;

	// ウィンドウ枠サイズを取得
	borderSize = (borderWidth > borderHeight) ? borderWidth : borderHeight;

	// メインキャンバスの最大サイズを計算
	// 最大幅は画面右の操作パネル幅分減らしておく
	int canvasWidth = desktopRect.right - desktopRect.left - borderWidth
			- PANEL_SIZE_WIDTH;
	int canvasHeight = desktopRect.bottom - desktopRect.top - borderHeight;

	// 幅か高さが960pxを超えていたら960pxにする
	if (canvasWidth > 960) {
		canvasWidth = 960;
	}
	if (canvasHeight > 960) {
		canvasHeight = 960;
	}

	// 縦横の小さいほうにあわせる
	if (canvasHeight > canvasWidth) {
		canvasHeight = canvasWidth;
	} else {
		canvasWidth = canvasHeight;
	}

	// 940px未満の場合はミニモードに切り替える
	if (canvasHeight < 940) {
		miniPanelMode = true;
	} else {
		miniPanelMode = false;
	}
	// ウィンドウサイズを変更する
	width = borderWidth + canvasWidth + PANEL_SIZE_WIDTH;
	height = borderHeight + canvasHeight;
	this->move(0, 0, width, height);

	//----------------------------------------

	// ここでコールしないと静的解析で警告が出る
	onCreate();

	if (miniPanelMode == true) {
		this->hide();
		this->move(0, 0, width + 100, height);
		this->move(0, 0, width, height);
		this->show();
	}

	// ラインセンスキー登録済み＆アクティベーション失敗時->体験版として動作（警告画面表示）
	if (getLicenseKeyRegistered() && !getActivationResult()) {
		showMessageBox(IDS_ACTIVATION_ERROR, IDS_ACTIVATION_ERROR_TITLE,
		MB_OK | MB_ICONWARNING);
	}
}

/**
 * ハンドルを取得する。
 * @return ウィンドウハンドル
 */
HWND MainWindow::getHandle(void) {
	return Window::getHandle();
}

/**
 * 最小ウィンドウサイズを取得する。
 * @return 最小のウィンドウサイズ
 */
int MainWindow::getMinimalSize(void) {
	return miniPanelMode ? 760 : 940 + borderSize;
}

/**
 * ミニパネル(低解像度ディスプレイ用)モードの取得
 * @return true:有効/false:無効
 */
bool MainWindow::IsMiniPanelMode() {
	return miniPanelMode;
}

/**
 * デストラクタ
 */
MainWindow::~MainWindow(void) {
	DeleteObject(hUIFont);
	DeleteObject(hCanvasFont);

	delete canvas_Main;
	delete canvas_MiniMap;
	delete controlPanel;
	delete csvData;
	if (this->HotSpotAreaRgn != NULL) {
		DeleteObject(this->HotSpotAreaRgn);
	}
	delete this->panelSettingData;
	if (this->areaSettingData) {
		// 解析範囲設定情報を削除
		delete this->areaSettingData;
	}
}

/**
 * ホットスポットを検出する
 */
void MainWindow::detectHotspot(bool force) {
	// 結果データが読み込めるか確認する
	if ((result == NULL) || (result->isAvailable() == false)) {
		return;
	}

	if (force == false) {
		// ホットスポット情報を読み込む
		if (result->loadHotspot() == true) {
			// 読み込みに成功した場合は検出しない
			canvasUpdate();
			return;
		}
	}

	double ret = DetectSettingDialog::create(getHandle());

	if (ret == 0) {
		return;
	}

	// (2017/5/31YM)閾値温度を格納

	thresholdtemp = (ret * (100 - 0) / 200);

	class {
	public:
		static DWORD proc(ProgressBar::Parameters *param) {
			// (2019/09/25LEE)  int threshold =>　double threshold, (int*)param => (double*)param  に変更
			double threshold = *((double*) param->data);
			int completed = 0;
			int count = result->getDataCount();
#			ifdef _OPENMP
#			pragma omp parallel for
#			endif
			for (int i = 0; i < count; i++) {
				if (param->forceStop == 0) {
					result->detectHotspot(i, threshold);
					completed++;
					param->progress = 100 * completed / count;
				}
			}
			return 0;
		}

		static void succeeded(void *arg) {
			// 指定範囲外のホットスポットを除外する
			MainWindow::getInstance()->excludeHotspot();
			// 完了している場合はホットスポットを保存する
			result->saveHotspot();
		}
	} detectHotspot;

	// ダイアログボックスを開く
	ret = ProgressBar::create(this, ProgressBar::HOTSPOT_DETECTION,
			detectHotspot.proc, detectHotspot.succeeded, &ret);

	if (ret == IDCANCEL) {
		// キャンセルが選択された場合は元に戻す
		result->clearHotspot();
		result->loadHotspot();
		hotspotLinker->loadFrom(result->getHotspotLinkFileName());
	} else {
		// (2017/8/23YM)パネル平均温度に関する処理を処理完了後に移動
		//　パネル平均温度を保存する
		result->savePanelTemp(result->getTempDataFileName());
		//　パネル温度データをセット
		result->setTemp();
		// (2017/6/2YM)報告書出力データをクリア
		// 報告書出力をクリアする
		clearHotspotIdList();
		saveHotspotIdList();
		// リンクはすべて削除する
		hotspotLinker->clear();
		// ホットスポット/ホットスポットリンク情報を保存する
		result->saveHotspot();
		hotspotLinker->saveTo(result->getHotspotLinkFileName());
		// 全体俯瞰画像上のホットスポット位置を更新する
		getHotspotOverallPosition();
		// コントロールパネルの表示を更新
		updateInfo();
	}

	// (2016/12/28YM)ホットスポットが存在したらホットスポット自動リンクボタン有効化
	// (2017/3/2YM)特徴点検出ボタンを常時有効化に変更のため処理削除
	// 画面表示を更新する
	canvasUpdate();
}

void MainWindow::loadPositionInfo(void) {
	// 内部位置情報データを読み込む
	if (!result->loadPositionInfo()) {
		// 無名関数を定義する
		class {
		public:
			static DWORD proc(ProgressBar::Parameters *param) {
				int count = result->getDataCount();
				int completed = 0;
				// 読み込みに失敗した場合、GPSデータから内部位置情報データを生成する
#				ifdef _OPENMP
#				pragma omp parallel for
#				endif
				for (int i = 0; i < count; i++) {
					result->setDefaultInternalPositionInfo(i);
					if (param->forceStop == 0) {
						result->setDefaultDirection(i);
						completed++;
						param->progress = 100 * completed / count;
					}
				}
				return 0;
			}
		} loadDefault;

		// ダイアログボックスを開く
		ProgressBar::create(this, ProgressBar::LOADING, loadDefault.proc, NULL);
	}
}

void MainWindow::createWholePictures(void) {
#ifdef USE_WHOLE_PICTURE
	// 無名関数を定義する
	class 
	{
		public:
			static DWORD proc(ProgressBar::Parameters* param) 
			{
				MainWindow::getInstance()->saveWholePictureSplit(&param->progress);
				return 0;
			}
	}savePicture;
	
	// ダイアログボックスを開く
	ProgressBar::create(this, ProgressBar::CREATING_WHOLE_PICTURE, savePicture.proc, NULL);
#endif
}

void MainWindow::createWholePicturesReport(void) {
	// 無名関数を定義する
	class {
	public:
		static DWORD proc(ProgressBar::Parameters *param) {
			int count = 2;
			int completed = 0;
			/*
			 *  全体俯瞰画像のファイルが存在するか確認する
			 *  pictureType = 0 → VB
			 *  pictureType = 1 → IR
			 */
			for (int pictureType = 0; pictureType < count; pictureType++) {
				// (2017/5/2YM)毎回俯瞰図を作成しなおすように変更
				// 存在しなければ全体俯瞰画像を保存する
				TCHAR *path = result->getWholePictureFileName(pictureType);
				MainWindow::getInstance()->saveWholePicture(path, pictureType);
				completed++;
				param->progress = 100 * completed / count;
			}
			return 0;
		}
	} savePicture;

	// ダイアログボックスを開く
	ProgressBar::create(this, ProgressBar::CREATING_WHOLE_PICTURE,
			savePicture.proc, NULL);
}

//　(2017/2/9YM)2016/10/4田崎さん修正内容を反映
void MainWindow::deleteWholePictures(void) {
	// 俯瞰画像が保存されている場合は削除する
	for (int pictureType = 0; pictureType < 2; pictureType++) {
		if (result->existWholePicture(pictureType) == true) {
			DeleteFile(result->getWholePictureFileName(pictureType));
		}
	}
}

void MainWindow::getHotspotOverallPosition() {
	result->initHotspotOverallPosition();

	// 無名関数を定義する
	class {
	public:
		static DWORD proc(ProgressBar::Parameters *param) {
			int completed = 0;
			int count = result->getDataCount();
			for (int picNo = count - 1; picNo >= 0; picNo--) {
				if (param->forceStop == 0) {
					int num = result->getHotspotCount(picNo);
					for (int ptNo = 0; ptNo < num; ptNo++) {
						result->getHotspotOverallPosition(picNo, ptNo);
					}
					completed++;
					param->progress = 100 * completed / count;
				}
			}
			return 0;
		}
	} getHotspotOverallPositionDialog;

	// ダイアログボックスを開く
	ProgressBar::create(this, ProgressBar::UPDATING_HOTSPOT_INFO,
			getHotspotOverallPositionDialog.proc, NULL);
}

void MainWindow::onCreate(void) {
	// ウィンドウ内部の幅と高さを計算
	const int canvasHeight = this->clientHeight();
	const int canvasWidth = canvasHeight;
	// キャンバスの配置
	canvas_Main = new MainCanvas(WS_VISIBLE | SS_NOTIFY, 0, 0, canvasWidth,
			canvasHeight, this);
	canvas_MiniMap = new Canvas(WS_VISIBLE | SS_NOTIFY, canvasWidth + 10, 620,
			320, 320, this);
	// (2017/4/6YM)ミニマップの座標を少し下へ移動
	// コントロールの配置
	controlPanel = new ControlPanel(canvasWidth, canvasHeight);

	const int fontSize = -13;				// フォントサイズは固定とする

	// フォントの設定
	const TCHAR *fontFamily = NULL;
	fontFamily = TEXT("Meiryo UI");
	hUIFont = CreateFont(fontSize, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE,
	DEFAULT_CHARSET,
	OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	DEFAULT_PITCH, fontFamily);

	// (2020/01/06LEE) Programの画面で表示する部分　-20から-10で変更
	hCanvasFont = CreateFont(-10, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE,
	DEFAULT_CHARSET,
	OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	DEFAULT_PITCH, fontFamily);

	// 作成したフォントを各コントロールにセットする
	controlPanel->setFont(hUIFont);
	controlPanel->init();
	canvas_Main->setFont(hCanvasFont);

	// ミニマップのイベントハンドラをセットする
	canvas_Main->setHandler(::CanvasProc);
	canvas_MiniMap->setHandler(::CanvasProc);

	// キャンバスを更新する
	canvasUpdate();

	UpdateWindow(getHandle());
	this->show();
}

LRESULT MainWindow::onCommand(UINT uMsg, WPARAM wParam, HWND hwndControl) {
	controlPanel->onCommand(uMsg, wParam, hwndControl);
	switch (HIWORD(wParam)) {
	case BN_CLICKED:
		if (hwndControl == *canvas_Main) {
			canvas_MainOnClicked();
		} else if (hwndControl == *canvas_MiniMap) {
			canvas_MiniMapOnClicked();
		}
		return 1;

	case STN_DBLCLK:
		if (hwndControl == *canvas_MiniMap) {
			canvas_MiniMapOnDblClicked();
		} else if (hwndControl == *canvas_Main) {
			canvas_MainOnDblClicked();
		}
		return 1;

	case CBN_SELENDOK:
		return 1;

	case EN_CHANGE:
		return 1;
	}
	return 0;
}

/**
 * キー入力時の処理を行う。
 * @param uMsg メッセージID
 * @param wParam 1つ目のパラメータ
 */
LRESULT MainWindow::onKey(UINT uMsg, WPARAM wParam) {
	return controlPanel->onKey(uMsg, wParam);
}

LRESULT MainWindow::onDrawItem(UINT uMsg, WPARAM wParam,
		LPDRAWITEMSTRUCT drawItem) {
	if (drawItem->hwndItem == *canvas_MiniMap) {
		canvas_MiniMap->transfer(drawItem->hDC);
	} else if (drawItem->hwndItem == *canvas_Main) {
		canvas_Main->transfer(drawItem->hDC);
	}
	return TRUE;
}
//TODO
LRESULT MainWindow::onMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	// メッセージパラメータ
	int fwKeys = LOWORD(wParam);
	int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	int xPos = GET_X_LPARAM(lParam);
	int yPos = GET_Y_LPARAM(lParam);

	// wParamの上位ワード(WHEEL_DELTA)を120で割れば、
	// 何回「カクッ、カクッ」と回されたかがわかるらしい
	int wheelCount = zDelta / 120;

	// 処理
	TCHAR title[128];
	stprintf(title, TEXT("fwKeys: %04x, zDelta:%d, x:%d, y:%d"), fwKeys, zDelta,
			xPos, yPos);
	switch (controlMode) {
	case MainWindow::CONTROL_PANEL_MODE_DEFAULT:
		zoom(wheelCount, xPos, yPos);
		break;

	case MainWindow::CONTROL_PANEL_MODE_PANELSETTING:
		// 拡大/縮小すると選択範囲をリセットする
		this->panelSettingData->clearPointCount();
		zoom(wheelCount, xPos, yPos);
		break;

	case MainWindow::CONTROL_PANEL_MODE_SHOOTINGDATA:
		// データが読み込まれる前は処理しない
		if (result == NULL) {
			return FALSE;
		}

		if (zDelta < 0) {
			this->controlPanel->zoomOut();
		} else {
			this->controlPanel->zoomIn();
		}
		// メインキャンバスの更新
		canvas_MainUpdate();
		immediatelyRedraw();
		break;

	case MainWindow::CONTROL_PANEL_MODE_HOTSPOT:
		this->changeHotspotShowMode(-wheelCount);
		break;
	}

	// 終わり
	return TRUE;
}

LRESULT MainWindow::CanvasProc(HWND hWnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam) {
	if (hWnd == *canvas_MiniMap) {
		switch (uMsg) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEMOVE:
			minimapX = GET_X_LPARAM(lParam);
			minimapY = GET_Y_LPARAM(lParam);
			break;
		}
		return canvas_MiniMap->callDefaultProc(uMsg, wParam, lParam);
	} else if (hWnd == *canvas_Main) {

		switch (uMsg) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEMOVE:
			mainX = GET_X_LPARAM(lParam);
			mainY = GET_Y_LPARAM(lParam);
			break;
		}

		switch (this->controlMode) {
		case MainWindow::CONTROL_PANEL_MODE_DEFAULT:
			this->canvas_Main_DefaultProc(hWnd, uMsg, wParam, lParam);
			break;
		case MainWindow::CONTROL_PANEL_MODE_CAMERA:
			this->canvas_Main_CameraAdjustmentProc(hWnd, uMsg, wParam, lParam);
			break;
		case MainWindow::CONTROL_PANEL_MODE_SHOOTINGDATA:
			this->canvas_Main_ShootingDataAdjustmentProc(hWnd, uMsg, wParam,
					lParam);
			break;
		case MainWindow::CONTROL_PANEL_MODE_AREA:
			this->canvas_Main_AreaSettingProc(hWnd, uMsg, wParam, lParam);
			break;
		case MainWindow::CONTROL_PANEL_MODE_HOTSPOT:
			this->canvas_Main_HotspotAdjustmentProc(hWnd, uMsg, wParam, lParam);
			break;
		case MainWindow::CONTROL_PANEL_MODE_PANELSETTING:
			this->canvas_Main_PanelSettingProc(hWnd, uMsg, wParam, lParam);
			break;
		default:
			break;
		}
		return canvas_Main->callDefaultProc(uMsg, wParam, lParam);
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT MainWindow::onColorCtrl(HDC hDC, HWND hWnd) {
	return controlPanel->onColorCtrl(hDC, hWnd);
}

LRESULT MainWindow::onResize(WPARAM sizingEdge, RECT *windowRect) {
	// コントロールの配置が完了するまでは処理を行わない
	// ※ウィンドウ作成直後にも実行されるため要注意
	if (hUIFont == NULL) {
		return 0;
	}

	// ウィンドウサイズを取得
	RECT clientRect;
	this->getClientRect(&clientRect);

	// ウィンドウ内部の幅と高さを計算
	int canvasWidth, canvasHeight;
	canvasWidth = clientRect.right - clientRect.left - PANEL_SIZE_WIDTH;
	canvasHeight = clientRect.bottom - clientRect.top;

	// キャンバスをリサイズ
	canvas_Main->resize(canvasWidth, canvasHeight);

	if (miniPanelMode == false) {
		canvas_MiniMap->move(canvasWidth + 10, 620);
	} else {
		/**
		 * 高さが940px未満の場合、表示領域が足りない
		 */
		int remainHeight = canvasHeight - 400;
		if (remainHeight >= 320) {
			// MiniMapの表示領域が足りる場合は、最上位へ移動し、サイズは固定
			canvas_MiniMap->move(canvasWidth + 10, 430);	// (2017/8/8YM)座標変更
			canvas_MiniMap->resize(320, 320);
		} else {
			// MiniMapの表示領域が足りない場合は、最上位へ移動し、サイズを縮小
			canvas_MiniMap->move(canvasWidth + 10, 430);	// (2017/8/8YM)座標変更
			canvas_MiniMap->resize(320, remainHeight);
		}
	}

	// リサイズ完了の場合
	if (sizingEdge & FLAG_RESIZED) {
		if (this->controlMode == CONTROL_PANEL_MODE_AREA) {
			// 解析範囲設定の場合は俯瞰画像表示を画面にあわせる
			resetView();

			// 解析範囲設定が完了している場合は範囲も更新する
			if (this->isAreaComplete == true) {
				// リージョンを破棄する
				DeleteRgn(this->HotSpotAreaRgn);
				this->HotSpotAreaRgn = NULL;

				// キャンバス上の座標に変換
				std::vector<POINT> HotSpotAreasOnCanvas;
				HotSpotAreasOnCanvas.clear();
				for (HotSpotArea::iterator item = this->HotSpotAreas.begin();
						item != this->HotSpotAreas.end(); item++) {
					int x, y;
					gpsPosToPixel(item->x, item->y, &x, &y);
					POINT posCanvas = { x, y };
					HotSpotAreasOnCanvas.push_back(posCanvas);
				}

				// リージョン作成(vectorは、整列されている)
				this->HotSpotAreaRgn = CreatePolygonRgn(
						&HotSpotAreasOnCanvas.front(),
						HotSpotAreasOnCanvas.size(), WINDING);
			}

		}
		canvas_Main->resizeBackBuffer();
		canvas_MainUpdate();

		// コントロールパネルをリサイズ
		controlPanel->Resize(canvasWidth, canvasHeight);
	}
	return 1;
}

void MainWindow::canvas_MainOnClicked(void) {
	bool retVal = false;

	// モードごとに処理を分ける
	switch (controlMode) {
	case CONTROL_PANEL_MODE_CAMERA:
		retVal = canvas_Main_CameraAdjustmentOnClick();
		break;
	case CONTROL_PANEL_MODE_SHOOTINGDATA:
		retVal = canvas_Main_ShootingDataAdjustmentOnClick();
		break;
	case CONTROL_PANEL_MODE_AREA:
		retVal = canvas_Main_AreaSettingOnClick();
		break;
	case CONTROL_PANEL_MODE_HOTSPOT:
		retVal = canvas_Main_HotspotAdjustmentOnClick();
		break;
	case CONTROL_PANEL_MODE_PANELSETTING:
		retVal = canvas_Main_PanelSettingOnClick();
		break;
	}

	// 各関数の戻り値がfalseの場合はデフォルト動作
	if (retVal == false) {
		canvas_Main_DefaultOnClick();

		// ボタンの有効化/無効化
		if (selectedPictureId != -1) {
			controlPanel->enableButton();
			controlPanel->updateMode();
		} else {
			controlPanel->disableButton();
			controlPanel->hidehotspotcount();
		}

	}

}

/**
 * フォルダ選択ボタンをクリックした場合の処理
 */
void MainWindow::button_SetFolderOnClicked(void) {
	LPTSTR path = showSelectFolder(getHandle(),
			Resource::getString(IDS_SELECT_FOLDER));

	// キャンセルが選択された場合
	if (path == NULL) {
		return;
	}

	if ((result != NULL) && (lstrcmp(result->getDataPath(), path) == 0)) {
		// 現在開いているフォルダと同じフォルダの場合は何もしない
		free(path);
		return;
	}

	if (lstrlen(path) > 120) {
		// エラーメッセージを表示
		showMessageBox(IDS_FILEPATH_SIZEOVER, IDS_FILE_NOT_FOUND_TITLE,
		MB_OK | MB_ICONINFORMATION);
		return;
	}

	// 体験版動作中(ラインセンスキー情報登録なしorアクティベーション失敗)：cache有りフォルダは開かない
	if (!getActivationResult()) {
		TCHAR pathtmp[MAX_PATH];
		ZeroMemory(pathtmp, sizeof(pathtmp));
		stprintf(pathtmp, TEXT("%s\\%s"), path,
				Resource::getString(IDS_CACHE_PATH));
		if (PathFileExists(pathtmp)) {
			// エラーメッセージを表示
			showMessageBox(IDS_TRIAL_SELERR_MESSAGE, IDS_TRIAL_SELERR_TITLE,
			MB_OK | MB_ICONERROR);
			return;
		}
	}

	// データが読み込めなかった場合はリソースを解放
	class {
	public:
		struct Error {
			int id;
			int errorNumber;
		};

		struct Parameters {
			MainWindow *window;
			LPTSTR path;
			Error *errors;
			int errorCount;
			int errorCode;
		} params;

		static DWORD proc(ProgressBar::Parameters *param) {
			Parameters *params = (Parameters*) param->data;
			ResultData *resultTemp = new ResultData(params->path);

			// 指定したデータフォルダが利用可能かを確認
			if (!resultTemp->isAvailable()) {
				// 確保したメモリを解放する
				delete resultTemp;
				int result = TiffConverter::convert(params->path, &param->progress);
				if (result != TIFF_CONVERTER_SUCCESS) {
					// 変換するデータに何らかの問題がある場合
					params->errorCode = result;
					return FALSE;
				}

				// リロードする
				resultTemp = new ResultData(params->path);
			}

			// 結果データが有効かどうかを確認し、無効の場合はエラーにする。
			if (!resultTemp->isAvailable()) {
				delete resultTemp;
				params->errorCode = 102;
				return FALSE;
			}

			// GPSデータエラーをチェックする。
			if (!resultTemp->getErrorcheck()) {
				int piccount = resultTemp->getpiccount();
				for (int i = 0; i < piccount; i++) {
					// latitude=緯度、longitude=経度、height=高度
					int picNumber = resultTemp->getpicnum(i);
					int errorNumber = resultTemp->getErrornumber(picNumber);
					if (params->errorCount == 0) {
						params->errorCount = 1;
						params->errors = (Error*) malloc(sizeof(Error));
					} else {
						params->errorCount++;
						params->errors = (Error*) realloc(params->errors,
								sizeof(Error) * params->errorCount);
					}
					Error *error = &params->errors[params->errorCount - 1];
					error->id = picNumber;
					error->errorNumber = errorNumber;
				}
				delete resultTemp;
				params->errorCode = 101;
				return FALSE;
			}

			// すでにデータが読み込まれている場合
			if (result != NULL) {
				// 破棄する前に保存する
				params->window->saveAll();
				// 前のデータを破棄する
				delete result;
			}

			result = resultTemp;

			params->window->controlPanel->updateMode();
			// 表示温度範囲を読み込む
			params->window->loadTempRange();
			// 温度画像を生成する
			params->window->createDefaultTempPictures();

			// すでにホットスポットリンク生成器が生成されている場合
			if (hotspotLinker != NULL) {
				// 前のデータを破棄する
				delete hotspotLinker;
			}

			// ホットスポットリンク生成器を生成する
			hotspotLinker = new HotspotLinker(result);
			// すでにpanelSetting生成器が生成されている場合
			if (panelData != NULL) {
				// 前のデータを破棄する
				delete panelData;
			}

			// panelSetting生成器を生成する
			panelData = new PanelData();

			params->errorCode = 0;
			return TRUE;
		}
	} checkData;

	checkData.params.window = this;
	checkData.params.path = path;
	checkData.params.errors = NULL;
	checkData.params.errorCount = 0;

	// ダイアログボックスを開く
	if (ProgressBar::create(this, ProgressBar::CHANGING, checkData.proc, NULL,
			&checkData.params) != IDOK) {
		checkData.params.errorCode = -1;
	}

	int messageId = IDS_CHANGEDATACHECK;
	int titleId = IDS_ERROR;
	switch (checkData.params.errorCode) {
	case TIFF_CONVERTER_OPEN_FOLDER_ERROR:
		messageId = IDS_ERROR_CANT_OPEN_FOLDER;
		titleId = IDS_CHANGEDATA_PICERROR_TITLE;
		break;
	case TIFF_CONVERTER_NO_INFRARED_ERROR:
		messageId = IDS_ERROR_NO_INFRARED;
		titleId = IDS_CHANGEDATA_PICERROR_TITLE;
		break;
	case TIFF_CONVERTER_TOO_MANY_INFRARED_ERROR:
		messageId = IDS_ERROR_TOO_MANY_INFRARED;
		titleId = IDS_CHANGEDATA_PICERROR_TITLE;
		break;
	case TIFF_CONVERTER_VISIBLE_COUNT_ERROR:
		messageId = IDS_ERROR_VISIBLE_COUNT;
		titleId = IDS_CHANGEDATA_PICERROR_TITLE;
		break;
	case TIFF_CONVERTER_INVALID_INFRARED_ERROR:
		messageId = IDS_ERROR_INVALID_INFRARED;
		titleId = IDS_CHANGEDATA_PICERROR_TITLE;
		break;
	case TIFF_CONVERTER_INVALID_VISIBLE_ERROR:
		messageId = IDS_ERROR_INVALID_VISIBLE;
		titleId = IDS_CHANGEDATA_PICERROR_TITLE;
		break;
	case TIFF_CONVERTER_CREATE_FLIGHT_DATA_ERROR:
		// 変換するデータが間違っている場合
		messageId = IDS_ERROR_CREATE_FLIGHT_DATA;
		titleId = IDS_CHANGEDATA_PICERROR_TITLE;
		break;
	case 101:
		for (int i = 0; i < checkData.params.errorCount; i++) {
			GpsErrorMessageBox(checkData.params.errors[i].id,
					checkData.params.errors[i].errorNumber,
					IDS_GPS_NOT_FOUNT_TITLE,
					MB_OK | MB_ICONINFORMATION);
		}
		break;
	case 102:
		// ファイルフォーマットが無効な場合
		messageId = IDS_FILE_NOT_FOUND;
		titleId = IDS_FILE_NOT_FOUND_TITLE;
		break;
	}

	if (checkData.params.errors != NULL) {
		free(checkData.params.errors);
		checkData.params.errors = NULL;
	}

	if (checkData.params.errorCode != 0) {
		// 変換するデータに何らかの問題がある場合
		showMessageBox(messageId, titleId, MB_OK | MB_ICONINFORMATION);
		return;
	}

	loadPositionInfo();

	// 画像選択を解除する
	selectedPictureId = NOT_SELECTED;
	selectedHotspotPictureId = NOT_SELECTED;
	selectedHotspotPointId = NOT_SELECTED;
	// 画像選択解除されるのでボタンを無効にする
	controlPanel->disableButton();
	// ホットスポット検出ボタンを有効化する
	controlPanel->enableDetectionButton();
	// 解析範囲設定ボタンを有効化する
	controlPanel->enableAreaButton();
	// panelSettingモードボタンを有効化する
	controlPanel->enableNoButton();
	// 報告書出力ボタンを有効化する
	controlPanel->enableCSVButton();
	// 案件情報ボタンを有効化する
	controlPanel->enableItemButton();

	//(2021/10/22YM)テスト　高温部検出ボタンを有効化する
	controlPanel->enableHighTempButton();

	// 画面右の日付および枚数表示を更新
	controlPanel->setInfoFirstDate(result->getFirstDateString());
	controlPanel->setInfoDataCount(result->getDataCountString());

	// 表示範囲をリセットする
	resetView();

	// 解析範囲設定情報を取得
	if (this->areaSettingData) {
		// 既に取得されている場合、削除する
		delete this->areaSettingData;
	}
	this->areaSettingData = new AreaSettingData(result->getCachePath());
	// 解析範囲設定情報を初期化
	this->AreaSettingInit();
	// 解析範囲設定情報を読み込み
	HotSpotArea HotSpotAreasTemp;
	if (this->areaSettingData->loadAreaSetting(&HotSpotAreasTemp)) {
		// 読み込みに成功した場合
		this->HotSpotAreas = HotSpotAreasTemp;
		// キャンバス上の座標に変換
		std::vector<POINT> HotSpotAreasOnCanvas;
		for (HotSpotArea::iterator item = this->HotSpotAreas.begin();
				item != this->HotSpotAreas.end(); item++) {
			int x, y;
			gpsPosToPixel(item->x, item->y, &x, &y);
			POINT projective = { x, y };
			HotSpotAreasOnCanvas.push_back(projective);
		}
		this->HotSpotAreaRgn = CreatePolygonRgn(&HotSpotAreasOnCanvas.front(),
				HotSpotAreasOnCanvas.size(), WINDING);
		this->isAreaComplete = true;
		this->controlPanel->enableAreaClearButton();
	}

	// ホットスポット情報を読み込む
	if (result->loadHotspot() == true) {
		// 読み込みに成功した場合
		// ホットスポットリンクデータを読み込む
		hotspotLinker->loadFrom(result->getHotspotLinkFileName());
		// 全体のホットスポット情報を更新する
		this->getHotspotOverallPosition();
		// 報告書出力の設定を読み込む
		this->readHotspotIdList();
		//　パネル情報を読み込む
		panelData->loadPanelInfo();
	}

	// 案件情報を初期化
	if (this->csvData != NULL) {
		delete this->csvData;
	}
	this->csvData = new CsvData();

	// (2016/12/28YM)ホットスポットが存在したらホットスポット自動リンクボタン有効化
	controlPanel->enableAutoLinkButton();

	// (2017/5/25YM)パネル平均温度算出処理を追加
	result->loadTempdata();

	//　パネル温度データをセット
	result->setTemp();

	// ホットスポット総数、最高温度、平均温度を更新
	updateInfo();

	// 俯瞰合成画像を作成する
	createWholePictures();

	// 画面を更新する
	canvasUpdate();

	// ウィンドウタイトルを変更する
	TCHAR titleText[Resource::MAX_LOADSTRING + MAX_PATH];
	if (getActivationResult()) {
		stprintf(titleText, Resource::getString(IDS_APP_TITLE_FORMAT),
				result->getDataPath());
	} else {
		stprintf(titleText, Resource::getString(IDS_APP_TITLE_FORMAT_TRIAL),
				result->getDataPath());
	}

	this->setText(titleText);

	// Navi更新
	this->controlPanel->setNavigatorText(Resource::getString(IDS_NAVI_HOME));
}

void MainWindow::updateInfo(void) {
	// ホットスポット総数、最高温度、平均温度を更新
	controlPanel->setInfoHotspotMaxCount(result->getHotspotSumString());

	// (2020/01/08LEE) 
	controlPanel->sethotspotcount();

	// (2017/6/1YM)閾値温度表示
	StringStream thretemp;
	if (thresholdtemp < 0) {
		thretemp << TEXT("---") << Resource::getString(IDS_STATUS_TEMP_SUFFIX);
	} else {

		TCHAR text[128];                                  // (2019/09/25LEE) 追加。
		stprintf(text, TEXT("%.1lf"), thresholdtemp);     // (2019/09/25LEE) 追加。
		thretemp << text << Resource::getString(IDS_STATUS_TEMP_SUFFIX);
	}
	controlPanel->setInfoThresholdTemp((LPTSTR) thretemp.str().data());

	StringStream tempMax, tempAve;
	double max = (floor(result->getPanelTempMax() * 100 + 0.5) / 100);
	double ave = (floor(result->getPanelTempAverage() * 100 + 0.5) / 100);
	// (2017/5/29YM)データチェック
	if (isnan(max)) {
		tempMax << TEXT("---") << Resource::getString(IDS_STATUS_TEMP_SUFFIX);
	} else {
		tempMax << max << Resource::getString(IDS_STATUS_TEMP_SUFFIX);
	}
	if (isnan(ave)) {
		tempAve << TEXT("---") << Resource::getString(IDS_STATUS_TEMP_SUFFIX);
	} else {
		tempAve << ave << Resource::getString(IDS_STATUS_TEMP_SUFFIX);
	}
	controlPanel->setInfoTempMax((LPTSTR) tempMax.str().data());
	controlPanel->setInfoTempAve((LPTSTR) tempAve.str().data());
	controlPanel->updateMode();
}

void MainWindow::canvas_MainUpdate(void) {
	bool retVal = false;
	// メインキャンバスの描画
	canvas_Main->clear((HBRUSH) GetStockObject(BLACK_BRUSH));

	// モードごとに処理を分ける
	switch (controlMode) {
	case CONTROL_PANEL_MODE_CAMERA:

		retVal = canvas_Main_CameraAdjustmentUpdate();
		break;
	case CONTROL_PANEL_MODE_SHOOTINGDATA:

		retVal = canvas_Main_ShootingDataAdjustmentUpdate();
		break;
	case CONTROL_PANEL_MODE_AREA:

		retVal = canvas_Main_AreaSettingUpdate();
		break;
	case CONTROL_PANEL_MODE_HOTSPOT:
		controlPanel->sethotspotcount();
		retVal = canvas_Main_HotspotAdjustmentUpdate();
		break;
	case CONTROL_PANEL_MODE_PANELSETTING:

		retVal = canvas_Main_PanelSettingUpdate();
		break;
	}

	// 各関数の戻り値がfalseの場合はデフォルト動作
	if (retVal == false) {
		canvas_Main_DefaultUpdate();
	}
}

int MainWindow::getSelectedId(void) {
	return selectedPictureId;
}

// (2017/2/16YM)selectedPictureIDセット関数追加
void MainWindow::setSelectedId(int id) {
	selectedPictureId = id;
}

void MainWindow::selectNext(void) {
	if (result != NULL) {
		if (selectedPictureId < result->getDataCount() - 1) {
			selectedPictureId++;
		} else {
			selectedPictureId = 0;
		}
	}
}

void MainWindow::zoomIn(int zoom, int x, int y) {
	// マウスを動かしたときだけ中心点を表示の中心座標にずらす
	//現在の中心点とマウス座標の距離を計算する
	const int canvasWidth = canvas_Main->clientWidth();
	const int canvasHeight = canvas_Main->clientHeight();

	// ピクセル数→メートル数に変換する
	double dx = (x - canvasWidth / 2) / viewPixelPerMeter;
	double dy = (y - canvasHeight / 2) / viewPixelPerMeter;

	//中心点をマウス座標にずらす
	viewX = viewX + dx;
	viewY = viewY - dy;

	// 範囲チェック
	double xMax;
	double yMin;
	if (result != NULL) {
		double north, south, east, west;
		result->getDataAreaSize2(&west, &south, &east, &north);
		xMax = east - west;
		yMin = south - north;
		if (viewX < 0) {
			viewX = 0;
		} else if (viewX > xMax) {
			viewX = xMax;
		}
		if (viewY > 0) {
			viewY = 0;
		} else if (viewY < yMin) {
			viewY = yMin;
		}
	} else {
		// 最小値を0.01倍にする
		xMax = canvasWidth * 100.0;
		yMin = canvasHeight * 100.0;
	}

	// 表示倍率を取得する
	double viewRatio = getViewRatio();

	//ホイールを１つ回転させるごとに倍率をあげる
	if (viewRatio <= 0) {
		viewRatio = 1;
	}
	viewRatio = viewRatio * pow(1.1, zoom);
	// 撮影データ全体がキャンバス内に収まる倍率までしか下げられないようにする
	const double vppm = canvasWidth / xMax;
	const double hppm = canvasHeight / (-yMin);
	const double minRatio = ((hppm < vppm) ? hppm : vppm) / 10.0;
	if (viewRatio > 10) {
		viewRatio = 10;
	} else if (viewRatio < minRatio) {
		viewRatio = minRatio;
	}

	viewPixelPerMeter = viewRatio * 10.0;

}

void MainWindow::scroll(void) {
	zoomIn(0, (prevMainPos.x - mainX) + canvas_Main->clientWidth() / 2,
			(prevMainPos.y - mainY) + canvas_Main->clientHeight() / 2);
	prevMainPos.x = mainX;
	prevMainPos.y = mainY;
	canvasUpdate();
	immediatelyRedraw();
}

void MainWindow::zoom(int wheelCount, int xPos, int yPos) {
	// 現在のマウス位置がメインキャンバス内かどうかを調べる
	RECT rect;
	POINT mouse = { xPos, yPos };
	canvas_Main->getWindowRect(&rect);

	// 現在の表示倍率、表示位置を取得する
	const double pr = getViewRatio();
	const double px = viewX;
	const double py = viewY;

	// マウスが移動したかどうかを取得する
	const bool mouseMoved = ((mainX != prevMainPos.x)
			|| (mainY != prevMainPos.y));

	if ((PtInRect(&rect, mouse) == TRUE)
			&& (mouseMoved && (GetKeyState('L') & 0x8000))) {
		// Lキーを押しながらの場合は
		// マウスカーソルの位置が中心になるように移動しながら拡大・縮小する
		if (wheelCount < 0) {
			this->zoomIn(wheelCount, mainX, mainY);		//縮小させたい
		} else {
			this->zoomIn(wheelCount, mainX, mainY);		//拡大させたい
		}
	} else {
		// 拡大・縮小のみ
		this->zoomIn(wheelCount, canvas_Main->clientWidth() / 2,
				canvas_Main->clientHeight() / 2);
	}

	showHotspotDetail();

	prevMainPos.x = mainX;
	prevMainPos.y = mainY;

	// 表示が変化したかどうかを確認する
	const bool changed = (pr != getViewRatio()) || (px != viewX)
			|| (py != viewY);

	// メインキャンバスの更新(表示が変化した場合のみ更新する)
	if (changed == true) {
		canvas_MainUpdate();
		immediatelyRedraw();
	}
}

double MainWindow::getViewRatio() {
	return viewPixelPerMeter / 10;
}

void MainWindow::selectPrev() {
	if (result != NULL) {
		if (selectedPictureId > 0) {
			selectedPictureId--;
		} else {
			selectedPictureId = result->getDataCount() - 1;
		}
	}
}

void MainWindow::canvasUpdate(void) {

	// 座標マップの更新
	canvas_MiniMapUpdate();
	canvas_MiniMap->update();

	// メインキャンバスの更新
	canvas_MainUpdate();
	canvas_Main->update();
}

void MainWindow::immediatelyRedraw(void) {
	HDC hdc = GetDC(canvas_Main->getHandle());
	canvas_Main->transfer(hdc);
	ReleaseDC(canvas_Main->getHandle(), hdc);
}

void MainWindow::resultDataSave(void) {
	// コントロールパネルの値を保存
	int x, y;			//横軸、縦軸移動
	double ratio;		//縮尺
	double direction;	//回転
	int st;
	// モードごとに処理を分ける
	switch (controlMode) {
	case CONTROL_PANEL_MODE_CAMERA:
		x = controlPanel->getMoveH();
		y = controlPanel->getMoveV();
		ratio = controlPanel->getZoom();
		direction = controlPanel->getTurn();	// (2016/12/20YM)回転補正データを追加

		if (ratio < 0) {
			this->showMessageBox(IDS_PANEL_DATASETTING, IDS_ERR_ZOOM, MB_OK);
			ratio = 1; // (2019/11/05LEE) 0.1=>1 変更
		} else if (ratio > 5) {
			this->showMessageBox(IDS_PANEL_DATASETTING, IDS_ERR_ZOOM, MB_OK);
			ratio = 5;
		}

		// st!=0 場合は全体
		st = controlPanel->getsetingtype();
		if (st == 0) {

			result->setalldata(selectedPictureId, x, y, ratio, ratio,
					direction);
		}

		result->setCameraOffset2(selectedPictureId, x, y); // (2019/11/05LEE) 変更。
		result->setCameraRatio2(selectedPictureId, ratio, ratio); //(2019/11/05LEE) 変更。
		result->setCameraDirection2(selectedPictureId, direction); //(2019/11/05LEE) 変更。
		result->setCameraDatatype(selectedPictureId, st); //(2019/11/08LEE) 追加。
		result->saveCameraInfo(result->getCameraInfoFileName()); //(2019/11/05LEE) 変更。

		break;
	case CONTROL_PANEL_MODE_SHOOTINGDATA:
		direction = toRad(controlPanel->getTurn());
		canvas_Main_ShootingDataAdjustmentResultSet();
		result->setDirection(selectedPictureId, direction);
		result->savePositionInfo();
		break;
	case CONTROL_PANEL_MODE_AREA:
		//処理なし
		break;
	case CONTROL_PANEL_MODE_HOTSPOT:
		//処理なし
		break;
	case CONTROL_PANEL_MODE_PANELSETTING:
		//処理なし
		break;
	}
}

void MainWindow::resultDataWrite(void) {
	CameraInfo cameraInfo;
	InternalPositionInfo posData;

	// モードごとに処理を分ける
	switch (controlMode) {
	case CONTROL_PANEL_MODE_CAMERA:
		// カメラ情報を読み込む
		if (selectedPictureId == NOT_SELECTED) {
			// 選択されていない場合は最初の画像を選択する。
			selectedPictureId = 0;
		}
		result->getCameraInfo(selectedPictureId, &cameraInfo);
		// コントロールパネルに読み込んだ値を表示する
		controlPanel->setMoveH(cameraInfo.offset.x);
		controlPanel->setMoveV(cameraInfo.offset.y);
		controlPanel->setZoom(cameraInfo.ratio.x);
		controlPanel->setTurn(cameraInfo.direction); //　(2016/12/20YM)回転補正データセットを追加
		controlPanel->setdatatype(cameraInfo.datatype); //　(2019/11/08LEE)
		break;
	case CONTROL_PANEL_MODE_SHOOTINGDATA:
		// 選択した画像の位置情報を読み込む
		if (selectedPictureId == NOT_SELECTED) {
			// 選択されていない場合は最初の画像を選択する。
			selectedPictureId = 0;
		}
		result->getPosition(selectedPictureId, &posData);
		// コントロールパネルに読み込んだ値を表示する
		canvas_Main_ShootingDataAdjustmentResultGet();
		controlPanel->setTurn(toDeg(posData.direction));
		break;
	case CONTROL_PANEL_MODE_AREA:
		// 処理なし
		break;
	case CONTROL_PANEL_MODE_HOTSPOT:
		// 選択の解除
		selectedHotspotPictureId = NOT_SELECTED;
		selectedHotspotPointId = NOT_SELECTED;
		break;
	case CONTROL_PANEL_MODE_PANELSETTING:
		// 選択の解除
		if (panelData->getSelectedPanelId() == -1) {
			controlPanel->disableControl();
		} else {
			controlPanel->enableControl();
		}
		break;
	}
}

void MainWindow::canvasUpdateNext(void) {
	resultDataSave();
	// 次の画像ID取得
	selectNext();
	resultDataWrite();
	canvasUpdate();

	HWND window = canvas_Main->getHandle();
	PictureWindow::Windowupdate2(window);
}

void MainWindow::canvasUpdatePrev(void) {
	resultDataSave();
	// 前の画像ID取得
	selectPrev();
	resultDataWrite();
	canvasUpdate();

	HWND window = canvas_Main->getHandle();
	PictureWindow::Windowupdate2(window);

}

double MainWindow::getDirection(int id) {
	double direction = result->getBaseDirection() + result->getDirection(id);
	return direction;
}

void MainWindow::canvas_MainOnDblClicked(void) {
	// フォルダを指定する前は何もしない
	if (result == NULL) {
		return;
	}

	bool retVal = false;

	// モードごとに処理を分ける
	switch (controlMode) {
	case CONTROL_PANEL_MODE_CAMERA:
		retVal = canvas_Main_CameraAdjustmentOnDblClick();
		break;
	case CONTROL_PANEL_MODE_SHOOTINGDATA:
		retVal = canvas_Main_ShootingDataAdjustmentOnDblClick();
		break;
	case CONTROL_PANEL_MODE_HOTSPOT:
		retVal = canvas_Main_HotspotAdjustmentOnDblClick();
		break;
	case CONTROL_PANEL_MODE_AREA:
		retVal = canvas_Main_AreaSettingOnDblClick();
		break;
	case CONTROL_PANEL_MODE_PANELSETTING:
		retVal = canvas_Main_PanelSettingOnDblClick();
		break;
	}

	// 各関数の戻り値がfalseの場合はデフォルト動作
	if (retVal == false) {
		canvas_Main_DefaultOnDblClick();
	}
}

double MainWindow::getHeight(int id) {
	return result->getHeight(id);
}

void MainWindow::setMode(int mode) {
	// モード変更後の処理
	switch (mode) {
	case MainWindow::CONTROL_PANEL_MODE_DEFAULT:
		// 表示位置のリセット
		this->resetView();
		break;
	case MainWindow::CONTROL_PANEL_MODE_PANELSETTING:
		// 表示位置のリセット
		this->resetView();
		break;
	case MainWindow::CONTROL_PANEL_MODE_AREA:
		// 表示位置のリセット
		this->resetView();
		break;
	case MainWindow::CONTROL_PANEL_MODE_HOTSPOT:
		// 選択の解除
		blMouse = false;
		// 表示モードをリセット
		changeHotspotShowMode(-getHotspotShowMode());
		break;
	}

	controlMode = mode;
}

int MainWindow::getMode(void) {
	return controlMode;
}

void MainWindow::setControlPanel(int mode) {
	// モード変更前の処理
	switch (controlPanel->getMode()) {
	case ControlPanel::CONTROL_PANEL_MODE_HOTSPOT:

		// 選択の解除
		blMouse = false;
		// ホットスポットが変更されているか確認する
		if (isHotspotUpdated == true) {
			if (existsDisabledHotspot()) {
				int ret = showMessageBox(
				IDS_CONFIRM_DELETE_HOTSPOT,
				IDS_HOTSPOT_ADJUST_TITLE,
				MB_YESNO | MB_ICONQUESTION);
				if (ret == IDYES) {
					// 無効化されたホットスポットがある場合は削除する
					removeDisabledHotspots();
				}
			}
			// すべての画像がリンクされているか確認する
			if (hotspotLinker->checkAllLinked()) {
				int ret = showMessageBox(
				IDS_CONFIRM_ADJUST_POSITION,
				IDS_HOTSPOT_ADJUST_TITLE,
				MB_YESNO | MB_ICONQUESTION);
				if (ret == IDYES) {
					// 位置情報を補正して保存する
					result->adjustPosition(0);
					result->savePositionInfo();
					// (2017/2/9YM)2016/10/4田崎さん修正内容を反映
					// 位置補正が行われた場合はレポート用俯瞰合成画像を更新する
					deleteWholePictures();
				}
			} else {
				// (2017/9/4YM)ダイアログをYESNO選択に変更
				int ret = showMessageBox(
				IDS_ALERT_ADJUST_POSITION,
				IDS_HOTSPOT_ADJUST_TITLE,
				MB_YESNO | MB_ICONEXCLAMATION);
				if (ret == IDYES) {
					bool *checked = hotspotLinker->checkAllLinkedData();
					for (int i = 0; i < result->getDataCount(); i++) {
						if (checked[i] == false) {
							selectedPictureId = i;
							break;
						}
					}
					// メインウィンドウ：モード設定
					setMode(MainWindow::CONTROL_PANEL_MODE_HOTSPOT);
					// ホットスポット更新フラグをfalseにする
					// (2017/9/13YM)ホットスポットが更新されない不具合発生確認のため、注釈化
					//　キャンバスを更新
					this->canvasUpdate();
					return;
				}
			}
			// ホットスポット補正モードから別のモードに移行する際に
			// 全体のホットスポット情報を更新する
			this->getHotspotOverallPosition();
			// (2017/10/26YM)報告書出力をクリアしないように変更
			// 報告書出力をクリアする
			// ホットスポット総数、最高温度、平均温度の表示を更新
			updateInfo();
			// パネルごとのホットスポット数を再計算
			for (int panelId = 0; panelId < panelData->getPanelNameCountMax();
					panelId++) {
				this->updateHotspotCount(panelId);
			}
			// ホットスポット更新フラグをfalseにする
			isHotspotUpdated = false;
		}
		break;
	case ControlPanel::CONTROL_PANEL_MODE_PANELSETTING:

		if ((result != NULL) && (panelData != NULL)) {
			panelData->savePanelInfo();
		}
		break;
	case ControlPanel::CONTROL_PANEL_MODE_CAMERA:

		if (result != NULL) {

			result->saveCameraInfo(result->getCameraInfoFileName());
			// (2017/2/9YM)2016/10/4田崎さん修正内容を反映
			// カメラ設定が行われた場合、レポート用俯瞰合成画像を更新する
			deleteWholePictures();
		}
		break;
	case ControlPanel::CONTROL_PANEL_MODE_SHOOTINGDATA:

		if (result != NULL) {
			result->savePositionInfo();
			// (2017/2/9YM)2016/10/4田崎さん修正内容を反映
			// 回転・サイズ変更が行われた場合、レポート用俯瞰合成画像を更新する
			deleteWholePictures();
		}
		break;
	case ControlPanel::CONTROL_PANEL_MODE_AREA:

		if (this->areaSettingData) {
			// 解析範囲設定情報を保存
			this->areaSettingData->saveAreaSetting(&this->HotSpotAreas);
		}
	}
	controlPanel->setMode(mode);

	// モード変更後の処理
	this->resultDataWrite();
	this->canvasUpdate();

	if (selectedPictureId != NOT_SELECTED) {
		this->controlPanel->enableButton();
		this->controlPanel->updateMode();
	} else {
		this->controlPanel->disableButton();
	}
}

void MainWindow::getPanelSize(double *width, double *height) {
	TCHAR value[Resource::MAX_LOADSTRING];
	Resource::getString(IDS_PANEL_SIZE_DEFAULT_X, value);
	*width = toDouble(value);
	Resource::getString(IDS_PANEL_SIZE_DEFAULT_Y, value);
	*height = toDouble(value);
}

void MainWindow::selectHotspot(int pictureNo, int pointNo) {
	selectedHotspotPictureId = pictureNo;
	selectedHotspotPointId = pointNo;
}

/**
 * ホットスポット総数のメッセージボックスを表示する
 */
void MainWindow::hotspotSumMessage(void) {
	MessageBox(getHandle(), result->getHotspotSumString(),
			Resource::getString(IDS_HOTSPOT_SUM_COUNT), MB_OK);
}

/**
 * XLSMファイルを保存するダイアログを表示する
 */
TCHAR* MainWindow::showSaveXLSMFileDialog(void) {
	TCHAR filter[256];
	TCHAR xlsmFile[Resource::MAX_LOADSTRING];
	Resource::getString(IDS_XLSM_FILE, xlsmFile);
	stprintf(filter, TEXT("%s(*.xlsm)%c*.xlsm%c"), xlsmFile, TEXT('\0'),
			TEXT('\0'));
	return showOpenFile(getHandle(), filter, TEXT("xlsm"));
}

/**
 * CSVファイルを保存するダイアログを表示する
 */
TCHAR* MainWindow::showSaveCSVFileDialog(void) {
	TCHAR filter[256];
	TCHAR csvFile[Resource::MAX_LOADSTRING];
	Resource::getString(IDS_CSV_FILE, csvFile);
	stprintf(filter, TEXT("%s(*.csv)%c*.csv%c"), csvFile, TEXT('\0'),
			TEXT('\0'));
	return showOpenFile(getHandle(), filter, TEXT("csv"));
}

/**
 * CSVファイルを読み込むダイアログを表示する
 */
TCHAR* MainWindow::showLoadCSVFileDialog(void) {
	TCHAR filter[256];
	TCHAR csvFile[Resource::MAX_LOADSTRING];
	Resource::getString(IDS_CSV_FILE, csvFile);
	stprintf(filter, TEXT("%s(*.csv)%c*.csv%c"), csvFile, TEXT('\0'),
			TEXT('\0'));
	return showSelectFile(getHandle(), filter, TEXT("csv"));
}

/**
 * CSV/XLSMファイルを保存するダイアログを表示する
 */
TCHAR* MainWindow::showSaveCSVXLSMFileDialog(void) {
	TCHAR filter[256];
	TCHAR csvFile[Resource::MAX_LOADSTRING];
	Resource::getString(IDS_CSV_FILE, csvFile);
	TCHAR xlsmFile[Resource::MAX_LOADSTRING];
	Resource::getString(IDS_XLSM_FILE, xlsmFile);
	stprintf(filter, TEXT("%s(*.xlsm)%c*.xlsm%c%s(*.csv)%c*.csv%c"), xlsmFile,
			TEXT('\0'), TEXT('\0'), csvFile, TEXT('\0'), TEXT('\0'));
	return showOpenFile(getHandle(), filter, TEXT("xlsm"));
}

int MainWindow::showMessageBox(int textId, int titleId, int type) {
	TCHAR *title = new TCHAR[Resource::MAX_LOADSTRING];
	TCHAR *message = new TCHAR[Resource::MAX_LOADSTRING];
	Resource::getString(textId, message);
	Resource::getString(titleId, title);
	int ret = MessageBox(getHandle(), message, title, type);
	delete title;
	delete message;
	return ret;
}

//(2020/01/15LEE) GPS ERROR MESSAGE BOX
int MainWindow::GpsErrorMessageBox(int picnum, int errornum, int titleId,
		int type) {
	TCHAR *title2 = new TCHAR[Resource::MAX_LOADSTRING];
	Resource::getString(titleId, title2);
	TCHAR *message2 = new wchar_t[Resource::MAX_LOADSTRING];

	switch (errornum) {
	case GPS_ERROR_LONGITUDE:
		stprintf(message2, TEXT("%d番目の画像：GPSデータ「経度」に異常があります。\rアプリケーションを終了します。"),
				picnum);
		break;

	case GPS_ERROR_LATITUDE:
		stprintf(message2, TEXT("%d番目の画像：GPSデータ「緯度」に異常があります。\rアプリケーションを終了します。"),
				picnum);
		break;

	case GPS_ERROR_HEIGHT:
		stprintf(message2, TEXT("%d番目の画像：GPSデータ「高度」に異常があります。\rアプリケーションを終了します。"),
				picnum);
		break;

	case GPS_ERROR_LONGITUDE_LATITUDE:
		stprintf(message2,
				TEXT("%d番目の画像：GPSデータ「経度、緯度」に異常があります。\rアプリケーションを終了します。"),
				picnum);
		break;

	case GPS_ERROR_LONGITUDE_HEIGHT:
		stprintf(message2,
				TEXT("%d番目の画像：GPSデータ「経度、高度」に異常があります。\rアプリケーションを終了します。"),
				picnum);
		break;

	case GPS_ERROR_LATITUDE_HEIGHT:
		stprintf(message2,
				TEXT("%d番目の画像：GPSデータ「緯度、高度」に異常があります。\rアプリケーションを終了します。"),
				picnum);
		break;

	case GPS_ERROR_ALL:
		stprintf(message2,
				TEXT("%d番目の画像：GPSデータ「経度、緯度、高度」に異常があります。\rアプリケーションを終了します。"),
				picnum);
		break;
	}

	int ret2 = MessageBox(getHandle(), message2, title2, type);
	delete title2;
	delete message2;
	return ret2;
}

bool MainWindow::outputCsv(void) {
	int ret = 0;

	// ファイル名用ダイアログ表示
	ret = OutputCsvInfoDialog::create(this->getHandle());
	if (ret == IDOK) {
		TCHAR *pFilePath;

		// ファイルパスが存在するかチェック
		int check = csvData->checkExistFilePath();
		if (check == false) {
			// エラーメッセージ表示
			showMessageBox(IDS_OUTPUT_ERROR_MESSAGE, IDS_OUTPUT_TITLE,
			MB_OK | MB_ICONERROR);
			return false;
		}

		// 顧客情報入力ダイアログを表示
		pFilePath = csvData->getCustomerInfoPath();
		if (pFilePath != NULL) {
			CustomerInfoDialog::loadCustomerInfo(pFilePath);
			ret = CustomerInfoDialog::create(getHandle());
			if (ret == IDOK) {
				CustomerInfoDialog::saveCustomerInfo(pFilePath);
			} else {
				return false;
			}
		} else {
			return false;
		}

		// 装置情報入力ダイアログを表示
		pFilePath = csvData->getMachineInfoPath();
		if (pFilePath != NULL) {
			MachineInfoDialog::loadMachineInfo(pFilePath);
			ret = MachineInfoDialog::create(getHandle());
			if (ret == IDOK) {
				MachineInfoDialog::saveMachineInfo(pFilePath);
			} else {
				return false;
			}
		} else {
			return false;
		}

		// 報告書1用全体俯瞰画像出力
		createWholePicturesReport();

		// 内部データ出力
		result->saveHotspotDetail();

		// 報告書2用画像出力
		class {
		public:
			static DWORD proc(ProgressBar::Parameters *param) {
				int completed = 0;
				int count = MainWindow::getInstance()->getHotspotIdListSize();
				// @todo OpenMP対応(スレッドセーフ化)
				//#ifdef _OPENMP
				//#pragma omp parallel for
				//#endif
				for (int i = 0; i < count; i++) {
					if (param->forceStop == 0) {
#ifdef USE_WHOLE_PICTURE
						MainWindow::getInstance()->trimConstitutionPicture(
#else
						/*
						 *	MainWindow::getInstance()->saveHotspotPicture( 
						 *   (2020/01/21LEE) Detailのホットスポットの写真を保存する事
						 *   (2020/01/21LEE) Homeで報告書に保存する写真のSIZEを探して赤の線表示する事をここで全部処理
						 */
						MainWindow::getInstance()->saveHotspotPictureAlgorithm(
#endif
								MainWindow::getInstance()->getHotspotIdListId(
										i),
								MainWindow::getInstance()->getHotspotIdListViewRatio(
										i), 1);
						completed++;
						param->progress = 100 * completed / count;
					}
				}
				return 0;
			}
		} outputCsvProgress;

		// ダイアログボックスを開く
		ProgressBar::create(this, ProgressBar::WRITING_HOTSPOT_DETAIL,
				outputCsvProgress.proc, NULL);

		// (2017/6/2YM)追加データ出力
		pFilePath = csvData->getAddDataPath();
		result->saveAddData(pFilePath);

		// 内部データ出力
		pFilePath = csvData->getReport2DataPath();
		result->saveHotspotDetail(pFilePath);

		// パス情報を保存するファイルを開く
		// パス情報を保存するファイル名
		{
			pFilePath = new TCHAR[MAX_PATH];
			// 報告書を保存するフォルダを取得する
			TCHAR *filePath = clone(csvData->getReportPath());
			toBaseName(filePath);
			stprintf(pFilePath, TEXT("%s\\FilePath.txt"), filePath);
			delete filePath;
		}
		saveFilePathList(pFilePath);

		// 報告書のコピーを作成する
		pFilePath = clone(csvData->getReportPath());
		makeReport(pFilePath);

		//　報告書の起動
		openReport(pFilePath);
		delete pFilePath;

		return true;
	}
	return false;
}

//報告書の作成
bool MainWindow::makeReport(TCHAR *path) {
	FILE *file = fileOpen(path, TEXT("wb"));
	if (file == NULL) {
		return false;
	}

	// リソースを開く
	HRSRC res;
	if (getActivationResult()) {
		res = Resource::getObject(IDR_REPORT_TEMPLATE, (int) RT_RCDATA);
	} else {
		res = Resource::getObject(IDR_REPORT_TEMPLATE_TRIAL, (int) RT_RCDATA);
	}
	HGLOBAL mem = LoadResource(GetModuleHandle(NULL), res);
	size_t size = SizeofResource(GetModuleHandle(NULL), res);
	char *data = (char*) LockResource(mem);

	// ファイルに書き込む
	for (size_t i = 0; i < size; i++) {
		fputc(data[i], file);
	}

	// 後片付け
	FreeResource(mem);

	fclose(file);
	return true;
}
// 4点指定方式四角形判定関数
bool MainWindow::squareJudgment(POINT pt1, POINT pt2, POINT pt3, POINT pt4) {
	double rad[4];

	rad[0] = acos((angle(pt2, pt4, pt1)));
	rad[1] = acos((angle(pt1, pt3, pt2)));
	rad[2] = acos((angle(pt2, pt4, pt3)));
	rad[3] = acos((angle(pt3, pt1, pt4)));

	double total = rad[0] + rad[1] + rad[2] + rad[3];

	// 四角形の内角の和360度
	if (fabs(total - 2 * M_PI) < 1e-14) {
		return true;
	}
	return false;
}

// 4点指定方式四角形追加関数
int MainWindow::squareAdd(POINT pt1, POINT pt2, POINT pt3, POINT pt4) {
	double x[4], y[4];

	// @todo 仮
	pixelToGPSPos(pt1.x, pt1.y, &x[0], &y[0]);
	pixelToGPSPos(pt2.x, pt2.y, &x[1], &y[1]);
	pixelToGPSPos(pt3.x, pt3.y, &x[2], &y[2]);
	pixelToGPSPos(pt4.x, pt4.y, &x[3], &y[3]);

	// 四角形登録
	Vector2D square[4];
	for (int i = 0; i < 4; i++) {
		square[i].x = x[i];
		square[i].y = y[i];
	}
	int panelId = panelData->panelSettingAdd(square);

	return panelId;
}

/**
 * 表示を元に戻す
 */
void MainWindow::resetView(void) {
	if (result != NULL) {
		// 撮影データ全体がキャンバス内に収まるようにする
		double north, south, east, west;
		result->getDataAreaSize2(&west, &south, &east, &north);
		const double dataWidth = east - west;
		const double dataHeight = north - south;
		// 撮影データの中央を画面の中央にあわせる
		viewX = dataWidth / 2;
		viewY = -dataHeight / 2;
		// 撮影データ全体がキャンバスに収まるように画面を調整する
		const double hppm = canvas_Main->clientWidth() / dataWidth;
		const double vppm = canvas_Main->clientHeight() / dataHeight;
		viewPixelPerMeter = ((hppm < vppm) ? hppm : vppm);
	} else {
		viewX = 0;
		viewY = 0;
		viewPixelPerMeter = 10.0;
	}
}

void MainWindow::pixelToGPSPos(int x, int y, double *gpsX, double *gpsY) {
	// @todo 仮
	const int width = canvas_Main->clientWidth();
	const int height = canvas_Main->clientHeight();

	result->pixelToGPSPos(x - width / 2, y - height / 2, gpsX, gpsY,
			viewPixelPerMeter);

	*gpsX += viewX;
	*gpsY += viewY;
}

int MainWindow::getHotspotId(int x, int y) {
	const int radius = (getViewRatio() < 1 ? 2 : getViewRatio() * 2) + 2;
	const int radius_2 = radius * radius;
	for (int i = 0; i < (int) result->getAllHotspotCount(); i++) {
		int _x, _y;
		Vector2D pos = result->getAllHotspotPos(i);
		gpsPosToPixel(pos.x, pos.y, &_x, &_y);
		const int dx = _x - mainX;
		const int dy = _y - mainY;
		if ((dx * dx) + (dy * dy) < radius_2) {
			return i;
		}
	}
	// 見つからなかった場合
	return -1;
}

void MainWindow::excludeHotspot(void) {
	if (HotSpotAreas.size() < 3) {
		// 範囲が指定されていない場合は除外しない
		return;
	}

	// キャンバス上の座標に変換
	std::vector<POINT> HotSpotAreasOnCanvas;
	for (HotSpotArea::iterator item = this->HotSpotAreas.begin();
			item != this->HotSpotAreas.end(); item++) {
		int x, y;
		gpsPosToPixel(item->x, item->y, &x, &y);
		POINT projective = { x, y };
		HotSpotAreasOnCanvas.push_back(projective);
	}

	const int dataCount = result->getDataCount();
	// 熱画像サイズを取得する
	const int infraredWidth = result->getInfraredWidth();
	const int infraredHeight = result->getInfraredHeight();
	for (int picNo = 0; picNo < dataCount; picNo++) {
		const int hotspotCount = result->getHotspotCount(picNo);
		// 取得した高度から画像の縮尺を計算する
		const double ratio = getPictureRatio(picNo);
		// 撮影方位を取得する
		const double direction = getDirection(picNo);
		int cx, cy;
		// 画像の表示位置を計算する
		gpsPosToPixel(picNo, &cx, &cy);
		for (int ptNo = hotspotCount - 1; ptNo >= 0; ptNo--) {
			if (result->isHotspot(picNo, ptNo)) {
				POINT p;
				result->getHotspot(picNo, ptNo, &p);
				const double fx = (p.x - infraredWidth / 2) * ratio;
				const double fy = (p.y - infraredHeight / 2) * ratio;
				int x = fx * cos(direction) - fy * sin(direction);
				int y = fx * sin(direction) + fy * cos(direction);
				p.x = x + cx;
				p.y = y + cy;
				if (!isPointInPolygon(p, HotSpotAreasOnCanvas.size(),
						HotSpotAreasOnCanvas.data())) {
					result->delHotspot(picNo, ptNo);
				}
			}
		}
	}
}

void MainWindow::showHotspotDetail(void) {
	static int selectedHotspotId = -1;

	if (result == NULL) {
		return;
	}

	// ホットスポットにマウスカーソルを当てると温度が表示されるようにする
	int selHotspotNum = getHotspotId(mainX, mainY);
	if (selHotspotNum != -1) {

		if (selHotspotNum != selectedHotspotId) {

			StringStream popupStr;
			HotspotDetail detail = result->getHotspotDetail(selHotspotNum);
			TCHAR str[64];
			// 詳細番号
			int detailNum = -1;
			for (int i = this->getHotspotIdListSize() - 1; i >= 0; i--) {
				if (this->getHotspotIdListId(i) == selHotspotNum) {
					detailNum = i;
					break;
				}
			}
			if (detailNum != -1) {
				// 詳細報告書に出力する番号を表示する
				popupStr << Resource::getString(IDS_HOTSPOT_DETAIL)
						<< (detailNum + 1) << std::endl;
			}
			// 管理番号の表示
			popupStr << Resource::getString(IDS_PANEL_INFO_NAME);
			if (detail.panelName == NULL) {
				popupStr << Resource::getString(IDS_OUT_OF_PANELS);
			} else {
				popupStr << detail.panelName;
			}
			popupStr << std::endl;
			// 緯度の表示
			stprintf(str, TEXT("%.8g%s\n"), detail.latitude,
					Resource::getString(IDS_DEGREES_SUFFIX));
			popupStr << Resource::getString(IDS_LATITUDE) << str;
			// 経度の表示
			stprintf(str, TEXT("%.8g%s\n"), detail.longitude,
					Resource::getString(IDS_DEGREES_SUFFIX));
			popupStr << Resource::getString(IDS_LONGITUDE) << str;
			// 温度の表示
			popupStr << Resource::getString(IDS_TEMPERATURE)
					<< (floor(detail.temperature * 100 + 0.5) / 100);
			popupStr << Resource::getString(IDS_STATUS_TEMP_SUFFIX);
			popupStr << std::endl;
			// 平均温度の表示
			double average = floor(detail.temperatureAverage * 100 + 0.5) / 100;
			if (!isnan(average)) {
				popupStr << Resource::getString(IDS_PANEL_TEMPERATURE)
						<< average;
				popupStr << Resource::getString(IDS_STATUS_TEMP_SUFFIX);
			} else {
				popupStr << Resource::getString(IDS_PANEL_TEMPERATURE);
				popupStr << Resource::getString(IDS_UNCALCULATED);
			}
			// 後片付け
			delete detail.fileNameIr;
			delete detail.fileNameVb;
			// ポップアップ表示
			canvas_Main->addBalloonToolTipForRect(
					(LPTSTR) popupStr.str().data());
			canvas_Main->setToolTipPopupTime(0x7fff);
		}
	} else {
		canvas_Main->deleteToolTip();
	}

	selectedHotspotId = selHotspotNum;
}

bool MainWindow::saveFilePathList(TCHAR *fileName) {
	//　ファイルを開く
	FILE *file = fileOpen(fileName, TEXT("w"));

	if (file == NULL) {
		return false;
	}

	TCHAR *pFilePath;
	char *utf8String;

	//　顧客情報ファイルパス
	pFilePath = csvData->getCustomerInfoPath();
	utf8String = toUTF8String(pFilePath);
	fprintf(file, "%s\n", utf8String);
	delete utf8String;

	// 機器情報ファイルパス
	pFilePath = csvData->getMachineInfoPath();
	utf8String = toUTF8String(pFilePath);
	fprintf(file, "%s\n", utf8String);
	delete utf8String;

	// 俯瞰画像ファイルパス
	for (int pictureType = 0; pictureType <= 1; pictureType++) {
		pFilePath = result->getWholePictureFileName(pictureType);
		utf8String = toUTF8String(pFilePath);
		fprintf(file, "%s\n", utf8String);
		delete utf8String;
	}

	// 飛行記録パス
	pFilePath = result->getFlightDataFileName();
	utf8String = toUTF8String(pFilePath);
	fprintf(file, "%s\n", utf8String);
	delete utf8String;

	// 内部データ出力パス
	pFilePath = csvData->getReport2DataPath();
	utf8String = toUTF8String(pFilePath);
	fprintf(file, "%s\n", utf8String);
	delete utf8String;

	// (2017/6/2YM)追加データ出力パス
	pFilePath = csvData->getAddDataPath();
	utf8String = toUTF8String(pFilePath);
	fprintf(file, "%s\n", utf8String);
	delete utf8String;

	fclose(file);
	return true;
}

bool MainWindow::saveFilePath2List(TCHAR *fileName, TCHAR *hotspotFileName) {
	//　ファイルを開く
	FILE *file = fileOpen(fileName, TEXT("w"));

	if (file == NULL) {
		return false;
	}

	TCHAR *pFilePath;
	TCHAR *path;
	char *utf8String;

	pFilePath = new TCHAR[MAX_PATH];
	path = result->getCachePath();
	// 熱画像
	stprintf(pFilePath, TEXT("%s\\infrared2.png"), path);
	utf8String = toUTF8String(pFilePath);
	fprintf(file, "%s\n", utf8String);
	delete utf8String;

	// 可視画像
	stprintf(pFilePath, TEXT("%s\\visible2.png"), path);
	utf8String = toUTF8String(pFilePath);
	fprintf(file, "%s\n", utf8String);
	delete utf8String;

	// パネル管理データ
	utf8String = toUTF8String(hotspotFileName);
	fprintf(file, "%s\n", utf8String);
	delete utf8String;

	fclose(file);

	delete pFilePath;
	return true;
}

void MainWindow::createTempPictures(void) {
	if (result == NULL) {
		return;
	}
	if (result->getDataVersion() >= 1) {
		double minTemp, maxTemp;
		minTemp = controlPanel->getHueTempMin();
		maxTemp = controlPanel->getHueTempMax();
		createTempPictures(minTemp, maxTemp, true);
		canvasUpdate();
	}
}

void MainWindow::createDefaultTempPictures(void) {
	double minTemp, maxTemp;
	minTemp = result->getTempMin();
	maxTemp = result->getTempMax();
	createTempPictures(minTemp, maxTemp, false);
}

void MainWindow::createTempPictures(double minTemp, double maxTemp,
		bool force) {
	double tempRange[3] = { minTemp, maxTemp, 0 };
	if (force == true) {
		tempRange[2] = 1;
	}

	class {
	public:
		static DWORD proc(ProgressBar::Parameters *param) {
			int completed = 0;
			int count = result->getDataCount();
			double min = ((double*) param->data)[0];
			double max = ((double*) param->data)[1];
			bool force = (((double*) param->data)[2] != 0);
#			ifdef _OPENMP
#			pragma omp parallel for
#			endif
			for (int i = 0; i < result->getDataCount(); i++) {
				TCHAR *fileName;
				result->getFilePath(i, ResultData::INFRARED_IMAGE, &fileName);

				if (force || (!existFile(fileName))) {
					// 熱画像がない場合のみ実行(強制実行は可)
					result->saveTemperaturePicture(i, min, max);
				}
				delete fileName;

				if (result->isPanelDetected()) {
					// パネル検出後はパネル画像も更新する。
					result->getFilePath(i, ResultData::PANEL_IMAGE, &fileName);
					if (force || (!existFile(fileName))) {
						result->savePanelPicture(i);
					}
					delete fileName;
				}
				completed++;
				param->progress = 100 * completed / count;
			}
			return 0;
		}
	} createTempPicture;

	// (2017/9/12YM)赤外線画像作成フラグを追加
	ProgressBar::create(this, ProgressBar::DEFAULT, createTempPicture.proc,
	NULL, tempRange);

	saveTempRange(force);

	// 赤外線画像/輪郭画像なら全画像再読み込みフラグをセット
	int type = controlPanel->getPictureType();
	if (type == ResultData::INFRARED_IMAGE || type == ResultData::PANEL_IMAGE) {
		this->canvas_Main->PicTypReset();
	}
}

/**
 * ホットスポット表示モードを取得する
 */
bool MainWindow::getEnabledHotspotShowMode(void) {
	return hotspotShowMode;
}

/**
 * ホットスポット表示モードを変更する
 */
void MainWindow::setEnabledHotspotShowMode(bool mode) {
	if (mode != hotspotShowMode) {
		hotspotShowMode = mode;
		canvasUpdate();
	}
}

/**
 * ホットスポット表示モードを変更する
 */
void MainWindow::toggleEnabledHotspotShowMode(void) {
	hotspotShowMode = !hotspotShowMode;
	canvasUpdate();
}

/**
 * ホットスポット表示モードを取得する
 */
bool MainWindow::getHotspotCandidateShowMode(void) {
	return hotspotCandidateShowMode;
}

/**
 * ホットスポット表示モードを変更する
 */
void MainWindow::setHotspotCandidateShowMode(bool mode) {
	if (mode != hotspotCandidateShowMode) {
		hotspotCandidateShowMode = mode;
		canvasUpdate();
	}
}

/**
 * ホットスポット表示モードを変更する
 */
void MainWindow::toggleHotspotCandidateShowMode(void) {
	hotspotCandidateShowMode = !hotspotCandidateShowMode;
	canvasUpdate();
}

/**
 * ホットスポット表示モードを取得する
 */
bool MainWindow::getDisabledHotspotShowMode(void) {
	return disabledHotspotShowMode;
}

/**
 * ホットスポット表示モードを変更する
 */
void MainWindow::setDisabledHotspotShowMode(bool mode) {
	if (mode != disabledHotspotShowMode) {
		disabledHotspotShowMode = mode;
		canvasUpdate();
	}
}

/**
 * ホットスポット表示モードを変更する
 */
void MainWindow::toggleDisabledHotspotShowMode(void) {
	disabledHotspotShowMode = !disabledHotspotShowMode;
	canvasUpdate();
}

/**
 * 特徴点表示モードを取得する
 */
bool MainWindow::getKeypointShowMode(void) {
	return keypointShowMode;
}

/**
 * 特徴点表示モードを変更する
 */
void MainWindow::setKeypointShowMode(bool mode) {
	if (mode != keypointShowMode) {
		keypointShowMode = mode;
		canvasUpdate();
	}
}

/**
 * 特徴点表示モードを変更する
 */
void MainWindow::toggleKeypointShowMode(void) {
	keypointShowMode = !keypointShowMode;
	canvasUpdate();
}

int MainWindow::getHotspotShowMode(void) {
	int showFlag = 0;

	const int DISABLED_HOTSPOT = 1;
	const int HOTSPOT_CANDIDATE = 2;
	const int ENABLED_HOTSPOT = 4;
	const int KEYPOINT = 8;

	// 削除候補ホットスポット表示モード(0ビット目)
	if (disabledHotspotShowMode == true) {
		showFlag += DISABLED_HOTSPOT;
	}
	// ホットスポット候補表示モード(1ビット目)
	if (hotspotCandidateShowMode == true) {
		showFlag += HOTSPOT_CANDIDATE;
	}
	// ホットスポット表示モード(2ビット目)
	if (hotspotShowMode == true) {
		showFlag += ENABLED_HOTSPOT;
	}
	// 特徴点表示モード(3ビット目)
	if (keypointShowMode == true) {
		showFlag += KEYPOINT;
	}

	switch (showFlag) {
	case 0:
		return 0;
	case ENABLED_HOTSPOT | KEYPOINT:
		return 1;
	default:
		return 2;
	}
}

bool MainWindow::saveWholePictureWithPanelData(const TCHAR *path,
		int pictureType) {
	// ファイル名を保存する領域を確保
	wchar_t *fileName;
#ifndef UNICODE
	// パス長の最大まで確保する
	fileName = new wchar_t[MAX_PATH];
	MultiByteToWideChar(CP_OEMCP, MB_COMPOSITE,
			path, -1, fileName, MAX_PATH);
#else
	// UNICODE版の場合はそのまま使える
	fileName = new wchar_t[MAX_PATH];
	wcscpy(fileName, path);
#endif // UNICODE

	// 俯瞰画像全体サイズを設定（単位pxあたり1cm）
	double left, right, top, bottom;
	result->getDataAreaSize2(&left, &bottom, &right, &top);
	int width = (right - left) * PIXELS_PER_METER;
	int height = (top - bottom) * PIXELS_PER_METER;

	Gdiplus::Bitmap *destImage = new Gdiplus::Bitmap(width, height,
	PixelFormat32bppARGB);
	Gdiplus::Graphics *graphics = new Gdiplus::Graphics(destImage);

	for (int i = 0; i < result->getDataCount(); i++) {

		int id = i;
		if (selectedPictureId != NOT_SELECTED) {
			if (i == result->getDataCount() - 1) {
				id = selectedPictureId;
			} else if (i >= selectedPictureId) {
				id = i + 1;
			}
		}

		// (2017/2/9YM)2016/10/4田崎さん修正内容を反映
		// Bitmapイメージを読み込む
		Gdiplus::Bitmap *img = (Gdiplus::Bitmap*) loadPicture(id, pictureType,
				PIXELS_PER_METER);

		// 画像を読み込んだBitmapを描画
		int x, y;
		result->gpsPosToPixel(id, &x, &y, PIXELS_PER_METER);
		graphics->DrawImage(img, x - (int) img->GetWidth() / 2,
				y - (int) img->GetHeight() / 2);

		// Bitmapオブジェクトを破棄
		delete img;

	}
	//図形を描画する処理

	// 管理番号とホットスポットの表示倍率
	double objectRatio = height / 1000.0;

	// 管理パネルデータがない場合は、処理なし
	if (panelData != NULL) {
		HDC bitmapDC = graphics->GetHDC();

		const int panelCount = panelData->getPanelNameCountMax();
		HPEN whitePen = CreatePen(PS_SOLID, 3, RGB(255, 255, 255));
		HPEN blackPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
		HBRUSH oldBrush = (HBRUSH) SelectObject(bitmapDC,
				(HBRUSH) GetStockObject(NULL_BRUSH));

		// ホットスポット一覧の俯瞰画像に使用するフォント
		// フォントサイズは高さ1000pxで20pxに設定
		int fontSize = fmax(20 * objectRatio, 10);
		HFONT reportFont = CreateFont(-fontSize, 0, 0, 0, FW_REGULAR, FALSE,
		FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Meiryo UI"));
		HFONT sysFont = (HFONT) SelectObject(bitmapDC, reportFont);

		for (int id = 0; id < panelCount; id++) {
			POINT pt[4];
			// 4点の実世界座標をキャンバス座標に変換
			for (int i = 0; i < 4; i++) {
				int x, y;
				Vector2D realPt = panelData->getPoint(id, i);
				result->gpsPosToPixel(realPt.x, realPt.y, &x, &y,
						PIXELS_PER_METER);
				pt[i].x = x;
				pt[i].y = y;
			}
			HPEN oldPen = (HPEN) SelectObject(bitmapDC, whitePen);
			Polygon(bitmapDC, pt, 4);
			SelectObject(bitmapDC, blackPen);
			Polygon(bitmapDC, pt, 4);
			SelectObject(bitmapDC, oldPen);

			// パネル名を描画
			{
				double x = 0, y = 0;
				// 4点の実世界座標をキャンバス座標に変換
				for (int i = 0; i < 4; i++) {
					Vector2D realPt = panelData->getPoint(id, i);
					x += realPt.x;
					y += realPt.y;
				}
				x /= 4;
				y /= 4;
				TCHAR *text = panelData->getPanelName(id);
				int length = lstrlen(text);
				int cx, cy;
				result->gpsPosToPixel(x, y, &cx, &cy, PIXELS_PER_METER);
				COLORREF defTextColor = GetTextColor(bitmapDC);
				int defBkMode = SetBkMode(bitmapDC, TRANSPARENT);
				RECT rect;
				ZeroMemory(&rect, sizeof(RECT));
				DrawText(bitmapDC, text, -1, &rect,
				DT_CALCRECT | DT_LEFT | DT_TOP);
				cx -= rect.right / 2;
				cy -= rect.bottom / 2;

				SetTextColor(bitmapDC, RGB(255, 255, 255));
				for (int j = -1; j <= 1; j++) {
					for (int i = -1; i <= 1; i++) {
						TextOut(bitmapDC, cx + i, cy + j, text, length);
					}
				}

				SetTextColor(bitmapDC, RGB(192, 0, 64));
				TextOut(bitmapDC, cx, cy, text, length);

				SetBkMode(bitmapDC, defBkMode);
				SetTextColor(bitmapDC, defTextColor);
			}
		}

		SelectObject(bitmapDC, sysFont);
		DeleteObject(reportFont);
		SelectObject(bitmapDC, oldBrush);
		DeleteObject(whitePen);
		DeleteObject(blackPen);

		graphics->ReleaseHDC(bitmapDC);
	}

	// ホットスポット表示円の直径(ピクセル数)
	int diameter = fmax(10 * objectRatio, 8);

	// (2017/5/2YM)ホットスポット表示処理追加
	//　※ここの処理は毎回俯瞰画像ファイルを作成しなおす
	//　ペン作成
	// CSVのファイルに入れる赤丸のSIZE
	Gdiplus::Pen *red = new Gdiplus::Pen(Gdiplus::Color::Red, 2.5);
	Gdiplus::Pen *frm = new Gdiplus::Pen(Gdiplus::Color::White, 3);
	// 点の表示位置を計算し、点を表示する
	for (int i = 0; i < (int) result->getAllHotspotCount(); i++) {
		int x, y;
		Vector2D pos = result->getAllHotspotPos(i);
		result->gpsPosToPixel(pos.x, pos.y, &x, &y, PIXELS_PER_METER);
		// 出力するホットスポットリストのidと一致すればオレンジペン
		bool reportOut = false;
		// 出力対象のホットスポットかどうかを確認する
		std::vector<HotspotIdList> hotspotIdList =
				MainWindow::getInstance()->getHotspotIdList();
		for (std::vector<HotspotIdList>::iterator item = hotspotIdList.begin();
				item != hotspotIdList.end(); item++) {
			if (item->id == i) {
				reportOut = true;
				break;
			}
		}
		if (reportOut == true) {
			// ホットスポットの点を描画
			graphics->DrawEllipse(frm, x - (diameter / 2), y - (diameter / 2),
					diameter, diameter);
			graphics->DrawEllipse(red, x - (diameter / 2), y - (diameter / 2),
					diameter, diameter);
		} else {
			// 出力対象外のホットスポットも描画
			graphics->DrawEllipse(frm, x - (diameter / 2), y - (diameter / 2),
					diameter, diameter);
			graphics->DrawEllipse(red, x - (diameter / 2), y - (diameter / 2),
					diameter, diameter);
		}
	}
	delete red;
	delete frm;

	// Save the altered image.
	CLSID pngClsid;
	Graphics::GetEncoderClsid(L"image/png", &pngClsid);

	// (2019/12/13LEE) 管理番号一覧保存に対応 、(2020/01/06LEE) XTMODEに対応
	if (!result->hasVisible()) {
		if (pictureType == 0) {
			cv::Mat img(1200, 1600, CV_8UC1);
			Graphics::saveCVImage(fileName, img);
		} else {
			destImage->Save(fileName, &pngClsid, NULL);
		}
	} else {
		destImage->Save(fileName, &pngClsid, NULL);
	}

	delete fileName;
	fileName = NULL;
	delete destImage;
	destImage = NULL;
	delete graphics;
	graphics = NULL;

	return true;
}

/**
 * すべてのデータを保存する
 */
void MainWindow::saveAll(void) {
	if (result != NULL) {
		// データを保存する
		if (panelData != NULL) {
			panelData->savePanelInfo();

		}
		result->saveCameraInfo(result->getCameraInfoFileName());
		result->savePositionInfo();
		// ホットスポット情報も保存する
		if (hotspotLinker != NULL) {
			hotspotLinker->saveTo(result->getHotspotLinkFileName());
		}
		result->saveHotspot();
	}
	if (this->areaSettingData) {
		// 解析範囲設定情報を保存
		this->areaSettingData->saveAreaSetting(&this->HotSpotAreas);
	}
}

void MainWindow::showOutOfRangeError(int titleId, double min, double max,
		int decimalCount) {
	// タイトルを取得
	TCHAR title[Resource::MAX_LOADSTRING];
	Resource::getString(titleId, title);

	// フォーマット文字列を生成
	TCHAR formatBase[Resource::MAX_LOADSTRING];
	Resource::getString(IDS_ERR_INPUT_OUT_OF_RANGE, formatBase);
	TCHAR format[Resource::MAX_LOADSTRING + 20];
	stprintf(format, formatBase, decimalCount, decimalCount);

	// メッセージを生成
	TCHAR message[Resource::MAX_LOADSTRING + 40];
	stprintf(message, format, min, max);

	MessageBox(getHandle(), message, title, MB_OK | MB_ICONERROR);
}

void MainWindow::saveTempRange(bool force) {
	StringStream fileName;
	fileName << result->getCachePath() << TEXT("\\TempRange.txt");
	if ((force == false) && (existFile(fileName.str().data()) == true)) {
		// 既にファイルが作成されている場合は作成しない
		return;
	}
	FILE *file = fileOpen(fileName.str().data(), TEXT("w"));
	if (file == NULL) {
		return;
	}
	// (2017/4/4YM)赤外線タイプデータを追加
	fprintf(file, "min:%.6g,max:%.6g,typ:%d\n",
			this->controlPanel->getHueTempMin(),
			this->controlPanel->getHueTempMax(),
			this->controlPanel->getInfraredType());
	fclose(file);
}

void MainWindow::loadTempRange(void) {
	StringStream fileName;
	fileName << result->getCachePath() << TEXT("\\TempRange.txt");
	FILE *file = fileOpen(fileName.str().data(), TEXT("r"));
	if (file == NULL) {
		this->controlPanel->setHueTempMax(result->getTempMax());
		this->controlPanel->setHueTempMin(result->getTempMin());
		return;
	}
	double min, max;
	// (2017/4/4YM)赤外線タイプデータを追加
	int InfraredType = -1;
	fscanf(file, "min:%lg,max:%lg,typ:%d\n", &min, &max, &InfraredType);
	fclose(file);
	this->controlPanel->setHueTempMin(min);
	this->controlPanel->setHueTempMax(max);
	// (2017/4/6YM)データが存在しなかった時の処理を追加
	if (InfraredType == -1) {
		//　デフォルトを白黒に
		InfraredType = 0;
	}
	// (2017/4/4YM)赤外線タイプデータを反映
	this->controlPanel->setInfraredType(InfraredType);

	// (2017/8/16YM)画面更新処理追加
	InvalidateRect(0, 0, false);

}

// (2017/2/2YM)ホットスポット自動リンク処理追加
void MainWindow::HotspotAutoLink(void) {

	// 特徴点自動検出用
	class {
	public:
		static DWORD proc(ProgressBar::Parameters *param) {
			int completed = 0;
			//　撮影画像の枚数を取得
			int count = result->getDataCount();
			// (2017/4/6YM)処理の分岐暫定追加
			int KeyFlag = ((int*) param->data)[0];
			if (KeyFlag == 0) {
				count--;
			}
			//　全枚数分ループ
			for (int i = 0; i < count; i++) {
				if (param->forceStop == 0) {
					// (2017/4/6YM)処理の分岐暫定追加
					if (KeyFlag == 1) {
						//　全方位について特徴点検出
						MainWindow::getInstance()->KeyPointAutoDetect2(i);
					} else {
						MainWindow::getInstance()->KeyPointAutoDetect(i);
					}
					completed++;
					param->progress = 100 * completed / count;
				}
			}
			return 0;
		}
	} KeyPointProgress;

	// (2017/4/6YM)処理分岐用変数暫定追加
	int KEYPOINTTYPE[1];

	//　(2017/4/6YM)確認メッセージ表示
	int ret = this->showMessageBox(IDS_DETECTING_KEYPOINT_CONF,
	IDS_HOTSPOT_AUTOLINK, MB_YESNO);
	if (ret == IDYES) {
		//　Shiftキー確認
		if (isKeyDown(VK_SHIFT)) {
			KEYPOINTTYPE[0] = 1;
		} else {
			KEYPOINTTYPE[0] = 0;
		}
		// ダイアログボックスを開く
		ProgressBar::create(this, ProgressBar::DETECTING_KEYPOINT,
				KeyPointProgress.proc, NULL, KEYPOINTTYPE);
		//　ホットスポットリンクを保存
		hotspotLinker->saveTo(result->getHotspotLinkFileName());
		//　ホットスポット変更フラグセット（モード変更時位置補正実行）
		isHotspotUpdated = true;
		//　キャンバスを更新
		canvasUpdate();
	}
}

// (2017/2/6YM)特徴点検出処理別関数化
void MainWindow::KeyPointAutoDetect(int id) {

	//　ORBアルゴリズムの変数宣言
	cv::Ptr<cv::BRISK> ptrBrisk = cv::BRISK::create();

	//　比較元画像Noをセット
	int i = id;

	//　対象となる赤外線画像を読み込み
	TCHAR *fileName;
	result->getFilePath(i, 1, &fileName);
	cv::Mat cvImageVbY = Graphics::loadCVImage(fileName);
	cv::Mat cvImageVb;
	cv::GaussianBlur(cvImageVbY, cvImageVb, cv::Size(5, 5), 0);
	//　特徴点を検出
	std::vector<cv::KeyPoint> KP1;
	ptrBrisk->detect(cvImageVb, KP1);
	//　特徴記述
	cv::Mat DS1;
	ptrBrisk->compute(cvImageVb, KP1, DS1);

	// 画像番号を取得する
	int j = i + 1;

	//　比較対象画像を読み込み
	TCHAR *fileName2;
	result->getFilePath(j, 1, &fileName2);
	cv::Mat cvImageVb2Y = Graphics::loadCVImage(fileName2);
	cv::Mat cvImageVb2;
	cv::GaussianBlur(cvImageVb2Y, cvImageVb2, cv::Size(5, 5), 0);
	//　特徴点を検出
	std::vector<cv::KeyPoint> KP2;
	ptrBrisk->detect(cvImageVb2, KP2);
	//　特徴記述
	cv::Mat DS2;
	ptrBrisk->compute(cvImageVb2, KP2, DS2);
	//　マッチング処理
	std::vector<cv::DMatch> matches;
	cv::BFMatcher matcher(cv::NORM_HAMMING, true);
	if ((!DS1.empty()) && (!DS2.empty())) {		// NULLチェック（NULLの場合エラーとなるため追加）
		matcher.match(DS1, DS2, matches);
	}
	//　精度の良い順に並び替え
	sort(matches.begin(), matches.end());
	// 良いペアのみ残す
	const double threshold = 40.0;
	std::vector<cv::DMatch> matches_good;
	for (int ii = 0; ii < (int) matches.size(); ii++) {
		if (matches[ii].distance < threshold) {
			matches_good.push_back(matches[ii]);
			if (matches_good.size() >= MAXKP) {		// (2017/6/13YM)特徴点最大検出数を可変に
				ii = (int) matches.size() + 1;
			}
		}
	}
	//　特徴点設定
	float x[2], y[2];
	int TP1, TP2, IdN[2], HSID[2];
	for (int ii = 0; ii < (int) matches_good.size(); ii++) {
		TP1 = matches_good[ii].queryIdx;
		TP2 = matches_good[ii].trainIdx;
		x[0] = KP1[TP1].pt.x;
		y[0] = KP1[TP1].pt.y;
		x[1] = KP2[TP2].pt.x;
		y[1] = KP2[TP2].pt.y;
		IdN[0] = i;
		IdN[1] = j;
		for (int k = 0; k < 2; k++) {
			// 撮影方位を取得する
			// (2017/2/22YM)特徴点座標の計算に画像回転角を考慮する必要がなかったので修正
			const double direction = 0;
			// 特徴点座標計算
			int px, py;
			px = x[k];
			py = y[k];
			int lx, ly;
			lx = px * cos(direction) - py * sin(direction);
			ly = px * sin(direction) + py * cos(direction);
			//　近くに他の特徴点がないかチェック
			bool sameHotspot = false;
			for (int hscnt = 0; hscnt < result->getHotspotCount(IdN[k]);
					hscnt++) {
				POINT hspt;
				result->getHotspot(IdN[k], hscnt, &hspt);
				int dx = hspt.x - lx;
				int dy = hspt.y - ly;
				if (dx * dx + dy * dy < 64) {
					sameHotspot = true;
					HSID[k] = hscnt;
					break;
				}
			}
			// 近くに特徴点がない場合のみ追加する
			if (sameHotspot == false) {
				result->addHotspot(IdN[k], lx, ly, HOTSPOT_TYPE_KEYPOINT);
				HSID[k] = result->getHotspotCount(IdN[k]) - 1;
			}
		}
		//　ホットスポットリンク追加
		hotspotLinker->add(i, HSID[0], j, HSID[1]);
	}
}

// (2017/2/6YM)特徴点検出処理別関数化
// (2017/4/6YM)全方位特徴点を検出する仕様追加
void MainWindow::KeyPointAutoDetect2(int id) {

	//　ORBアルゴリズムの変数宣言
	cv::Ptr<cv::BRISK> ptrBrisk = cv::BRISK::create();

	//　比較元画像Noをセット
	int i = id;

	//　対象となる赤外線画像を読み込み
	TCHAR *fileName;
	result->getFilePath(i, 1, &fileName);
	cv::Mat cvImageVbY = Graphics::loadCVImage(fileName);
	cv::Mat cvImageVb;
	cv::GaussianBlur(cvImageVbY, cvImageVb, cv::Size(5, 5), 0);
	//　特徴点を検出
	std::vector<cv::KeyPoint> KP1;
	ptrBrisk->detect(cvImageVb, KP1);
	//　特徴記述
	cv::Mat DS1;
	ptrBrisk->compute(cvImageVb, KP1, DS1);

	// (2017/4/6YM)画像の東西南北方向の画像について処理
	for (int jj = 0; jj < PICTURE_POS_COUNT; jj++) {
		// 画像番号を取得する
		int j = getAroundPictureId(i, jj);
		//　画像が存在するかチェック

		if ((j != -1) && (j != i)) {
			//　比較対象画像を読み込み
			TCHAR *fileName2;
			result->getFilePath(j, 1, &fileName2);
			cv::Mat cvImageVb2Y = Graphics::loadCVImage(fileName2);
			cv::Mat cvImageVb2;
			cv::GaussianBlur(cvImageVb2Y, cvImageVb2, cv::Size(5, 5), 0);
			//　特徴点を検出
			std::vector<cv::KeyPoint> KP2;
			ptrBrisk->detect(cvImageVb2, KP2);
			//　特徴記述
			cv::Mat DS2;
			ptrBrisk->compute(cvImageVb2, KP2, DS2);
			//　マッチング処理
			std::vector<cv::DMatch> matches;
			cv::BFMatcher matcher(cv::NORM_HAMMING, true);
			if ((!DS1.empty()) && (!DS2.empty())) {	// NULLチェック（NULLの場合エラーとなるため追加）
				matcher.match(DS1, DS2, matches);
			}
			//　精度の良い順に並び替え
			sort(matches.begin(), matches.end());
			// 良いペアのみ残す
			const double threshold = 40.0;
			std::vector<cv::DMatch> matches_good;
			for (int ii = 0; ii < (int) matches.size(); ii++) {
				if (matches[ii].distance < threshold) {
					matches_good.push_back(matches[ii]);
					if (matches_good.size() >= MAXKP) {	// (2017/6/13YM)特徴点最大検出数を可変に
						ii = (int) matches.size() + 1;
					}
				}
			}
			//　特徴点設定
			float x[2], y[2];
			int TP1, TP2, IdN[2], HSID[2];
			for (int ii = 0; ii < (int) matches_good.size(); ii++) {
				TP1 = matches_good[ii].queryIdx;
				TP2 = matches_good[ii].trainIdx;
				x[0] = KP1[TP1].pt.x;
				y[0] = KP1[TP1].pt.y;
				x[1] = KP2[TP2].pt.x;
				y[1] = KP2[TP2].pt.y;
				IdN[0] = i;
				IdN[1] = j;
				for (int k = 0; k < 2; k++) {
					// 撮影方位を取得する
					// (2017/2/22YM)特徴点座標の計算に画像回転角を考慮する必要がなかったので修正
					const double direction = 0;
					// 特徴点座標計算
					int px, py;
					px = x[k];
					py = y[k];
					int lx, ly;
					lx = px * cos(direction) - py * sin(direction);
					ly = px * sin(direction) + py * cos(direction);
					//　近くに他の特徴点がないかチェック
					bool sameHotspot = false;
					for (int hscnt = 0; hscnt < result->getHotspotCount(IdN[k]);
							hscnt++) {
						POINT hspt;
						result->getHotspot(IdN[k], hscnt, &hspt);
						int dx = hspt.x - lx;
						int dy = hspt.y - ly;
						if (dx * dx + dy * dy < 64) {
							sameHotspot = true;
							HSID[k] = hscnt;
							break;
						}
					}
					// 近くに特徴点がない場合のみ追加する
					if (sameHotspot == false) {
						result->addHotspot(IdN[k], lx, ly,
								HOTSPOT_TYPE_KEYPOINT);
						HSID[k] = result->getHotspotCount(IdN[k]) - 1;
					}
				}
				//　ホットスポットリンク追加
				hotspotLinker->add(i, HSID[0], j, HSID[1]);
			}
		}
	}
}

// (2017/4/4YM)画像位置初期化ボタンを追加
void MainWindow::button_InitPictureOnClicked(void) {

	// フォルダを指定する前は何もしない
	if (result == NULL) {
		return;
	}

	// データがない場合も何もしない
	if (result->getDataCount() == 0) {
		return;
	}

	//　確認メッセージ表示
	int ret = this->showMessageBox(IDS_INIT_PICTURE_CONF, IDS_INIT_PICTURE,
	MB_YESNO);

	if (ret == IDYES) {
		//　ポジションデータファイルのパスを取得
		TCHAR *str = result->getStringBufferPublic();
		stprintf(str, TEXT("%s\\Result_Position_Data.txt"),
				result->getCachePath());
		//　ポジションファイル削除
		DeleteFile(str);
		//　データを更新
		if (isKeyDown(VK_SHIFT)) {
			// Shiftキーを入力していた場合、高度・回転情報も初期化
			loadPositionInfo();
		} else {
			//　画像のXY座標のみ初期化
			ReloadPositionInfo();
		}
		// 全体のホットスポット情報を更新する
		this->getHotspotOverallPosition();
		//　キャンバスを更新
		canvasUpdate();
	}
}

// (2017/4/4YM)画像のXY座標のみ初期化する関数追加
void MainWindow::ReloadPositionInfo(void) {
	// 無名関数を定義する
	class {
	public:
		static DWORD proc(ProgressBar::Parameters *param) {
			int count = result->getDataCount();
			int completed = 0;
			// 読み込みに失敗した場合、GPSデータから内部位置情報データを生成する
#			ifdef _OPENMP
#			pragma omp parallel for
#			endif
			for (int i = 0; i < count; i++) {
				result->ResetDefaultInternalPositionInfo(i);
				if (param->forceStop == 0) {
					completed++;
					param->progress = 100 * completed / count;
				}
			}
			return 0;
		}
	} loadDefault;

	// ダイアログボックスを開く
	ProgressBar::create(this, ProgressBar::LOADING, loadDefault.proc, NULL);
}

// (2017/4/4YM)赤外線画像のタイプ選択関数追加
void MainWindow::InfraredTypeSelect(int Type) {

	// (2017/8/16YM)画面更新処理追加
	InvalidateRect(0, 0, false);

	//　データ存在チェック
	if (result == NULL) {
		return;
	}

	//　赤外線画像を再作成
	createTempPictures();

	//　報告書出力用全体俯瞰図を削除
	deleteWholePictures();

}

// (2017/4/4YM)赤外線画像タイプの取得関数追加
int MainWindow::getInfraredType(void) {
	return controlPanel->getInfraredType();
}

// (2017/5/31YM)閾値温度を取得する関数を追加
double MainWindow::getThresholdTemp(void) {
	return thresholdtemp;
}

// (2017/6/1YM)閾値温度をセットする関数を追加
void MainWindow::setThresholdTemp(float threshold) {
	thresholdtemp = threshold;
}

// (2017/6/14YM)指定したタイプのホットスポットを削除する関数を追加
void MainWindow::removeTypeHotspots(int chktype) {

	const int DATA_COUNT = result->getDataCount();
	for (int picNo = 0; picNo < DATA_COUNT; picNo++) {
		const int HOTSPOT_COUNT = result->getHotspotCount(picNo);
		for (int ptNo = HOTSPOT_COUNT - 1; ptNo >= 0; ptNo--) {
			int type = result->getHotspotType(picNo, ptNo);
			if (type == chktype) {
				// 指定したホットスポットを自動的に削除する
				hotspotLinker->removeHotspot(picNo, ptNo);
			}
		}
	}

	//　データ全クリアチェック
	result->CheckHSClearData();
	hotspotLinker->CheckLinkClearData();

	// 保存する
	result->saveHotspot();
	hotspotLinker->saveTo(result->getHotspotLinkFileName());
}

// (2017/6/20YM)ホットスポット全削除処理関数追加
void MainWindow::removeAllHotspots(void) {

	//　確認メッセージ表示
	int ret = this->showMessageBox(IDS_HOTSPOT_ALLDEL_CONF, IDS_HOTSPOT_ALLDEL,
	MB_YESNO);

	if (ret == IDYES) {
		//　ホットスポット削除
		clearHotspotIdList(); //(2020/01/10LEE) 
		removeHotspotALLIdList(); //(2020/01/10LEE) 
		removeTypeHotspots(HOTSPOT_TYPE_UNKNOWN);
		removeTypeHotspots(HOTSPOT_TYPE_DISABLED);
		removeTypeHotspots(HOTSPOT_TYPE_CANDIDATE);
		removeTypeHotspots(HOTSPOT_TYPE_ENABLED);
		updateHotspot();
		//　キャンバスを更新
		canvasUpdate();
	}

}

// (2017/6/20YM)特徴点全削除処理関数追加
void MainWindow::removeAllKeypoints(void) {

	//　確認メッセージ表示
	int ret = this->showMessageBox(IDS_KEYPOINT_ALLDEL_CONF,
	IDS_KEYPOINT_ALLDEL, MB_YESNO);

	if (ret == IDYES) {
		//　特徴点削除
		removeTypeHotspots(HOTSPOT_TYPE_KEYPOINT);
		//　キャンバスを更新
		canvasUpdate();
	}

}
