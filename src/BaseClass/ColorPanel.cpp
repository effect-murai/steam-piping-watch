/*
 * ColorPanel.cpp
 *
 *  Created on: 2016/02/09
 *      Author: PC-EFFECT-011
 */

// (2017/8/10YM)カラーパターン白黒を追加
#include "ColorPanel.h"
#include "InputBox.h"
#include "resource.h"

#include "MainWindow.h"			// (2017/8/10YM)追加
extern MainWindow *mainWindow;	// (2017/8/10YM)追加

ColorPanel::ColorPanel(LPCTSTR title, int x, int y, WindowContainer *parent) :
		Control(TEXT("BUTTON"), title, BS_GROUPBOX | WS_VISIBLE, x, y,
				ColorPanel::SIZE_WIDTH, ColorPanel::SIZE_HEIGHT, parent) {
	this->m_Temp_Max = ColorPanel::DEFAULT_TEMP_MAX;
	this->m_Temp_Min = ColorPanel::DEFAULT_TEMP_MIN;

	// 最高温度
	this->m_Edit_Max = new InputBox(Resource::getString(IDS_STATUS_TEMP_MAX),
	WS_VISIBLE | SS_CENTER, 0, ColorPanel::SIZE_LABEL, ColorPanel::SIZE_WIDTH,
			ColorPanel::SIZE_LABEL, this);

	// カラーマップ
	this->m_Control = new Canvas(0, ColorPanel::SIZE_LABEL * 2,
			ColorPanel::SIZE_WIDTH, ColorPanel::SIZE_CONTROL, this, true);

	// 最低温度
	this->m_Edit_Min = new InputBox(Resource::getString(IDS_STATUS_TEMP_MIN),
	WS_VISIBLE | SS_CENTER, 0,
			ColorPanel::SIZE_LABEL * 2 + ColorPanel::SIZE_CONTROL,
			ColorPanel::SIZE_WIDTH, ColorPanel::SIZE_LABEL, this);

	// 温度単位
	this->m_Label_Name = new Label(Resource::getString(IDS_STATUS_TEMP_SUFFIX),
	WS_VISIBLE | SS_CENTER, 0,
			ColorPanel::SIZE_LABEL * 3 + ColorPanel::SIZE_CONTROL,
			ColorPanel::SIZE_WIDTH, ColorPanel::SIZE_LABEL, this);

	this->setHandler(this->handleEvent);
}

ColorPanel::~ColorPanel() {
	delete this->m_Edit_Max;
	delete this->m_Control;
	delete this->m_Edit_Min;
	delete this->m_Label_Name;
}

/**
 * ウィンドウハンドルを取得する。
 */
HWND ColorPanel::getHandle(void) {
	// 複数の親に同じ名前の関数が定義されているので
	// 再定義の必要がある(?ω?)
	return Control::getHandle();
}

/**
 * フォントを設定する。
 * @param hFont フォント
 */
void ColorPanel::setFont(HFONT hFont) {
	Control::setFont(hFont);
	m_Edit_Max->setFont(hFont);
	m_Edit_Min->setFont(hFont);
	m_Label_Name->setFont(hFont);
}

/**
 * コントロールにリソースのビットマップを設定する。
 * @param id リソースID
 */
void ColorPanel::setBitmap(int id) {
	this->m_Control->selectGdiObject(Resource::getBitmap(id));
}

/**
 * コントロールを更新する。
 */
void ColorPanel::update() {
	// 背景更新
	// (2017/8/10YM)カラーパターン白黒を追加
	int irt = mainWindow->getInfraredType();
	if (irt == 0) {
		this->setBitmap(IDB_BITMAP_COLORPATTERN2);
	} else {
		this->setBitmap(IDB_BITMAP_COLORPATTERN);
	}

	// バッファ更新
	HDC hDC = GetDC(this->m_Control->getHandle());
	this->m_Control->transfer(hDC);
	ReleaseDC(this->m_Control->getHandle(), hDC);
}

/**
 * 温度に対応するカラー値を取得する。
 * @param temp 温度
 */
COLORREF ColorPanel::getColor(double temp) {
	COLORREF color;
	BYTE red;
	BYTE green;
	BYTE blue;
	double value;
	int pos;

	// 温度を 0 - 255 に変換
	value = this->m_Temp_Max - this->m_Temp_Min;
	value = 255L / value;
	value = value * (temp - this->m_Temp_Min);
	pos = (int) value;

	// RGB値 取得
	if (pos <= 63) {
		red = 0;
		green = 255 / 63 * pos;
		blue = 255;
	} else if ((pos > 63) && (pos <= 127)) {
		red = 0;
		green = 255;
		blue = 255 / 64 * (pos - 63);
	} else if ((pos > 127) && (pos <= 191)) {
		red = 255 / 64 * (pos - 127);
		green = 255;
		blue = 0;
	} else if (pos > 191) {
		red = 255;
		green = 255 / 64 * (pos - 191);
		blue = 0;
	}

	// カラー値へ変換
	color = RGB(red, green, blue);

	return color;
}

/**
 * 最高温度を取得する。
 */
double ColorPanel::getTempMax() const {
	return this->m_Temp_Max;
}

/**
 * 最高温度を設定する。
 * @param tempMax 最高温度
 */
void ColorPanel::setTempMax(double tempMax) {
	this->m_Temp_Max = tempMax;
	this->m_Edit_Max->setTextAsFloat(tempMax, 2);
}

/**
 * 最低温度を取得する。
 */
double ColorPanel::getTempMin() const {
	return this->m_Temp_Min;
}

/**
 * 最低温度を設定する。
 * @param tempMin 温度
 */
void ColorPanel::setTempMin(double tempMin) {
	this->m_Temp_Min = tempMin;
	this->m_Edit_Min->setTextAsFloat(tempMin, 2);
}

// グループボックスのイベントを処理するためのコールバック関数
LRESULT CALLBACK ColorPanel::handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam) {
	// 固有のイベント処理
	ColorPanel *parent = dynamic_cast<ColorPanel*>(Window::fromHandle(hWnd));
	if (parent != NULL) {
		return parent->ControlProc(hWnd, uMsg, wParam, lParam);
	}
	// デフォルトのイベント処理
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/**
 * グループボックスのイベントを処理する。
 * @param hWnd グループボックスのハンドル
 * @param uMsg メッセージID
 * @param wParam 最初のメッセージパラメータ
 * @param lParam 2番目のメッセージパラメータ
 */
LRESULT ColorPanel::ControlProc(HWND hWnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam) {
	// 固有のイベント処理
	HWND hWndCtrl;
	int nCode;

	switch (uMsg) {
	case WM_ERASEBKGND:
		return 0;
	case WM_PAINT:
		this->update();
		break;
	case WM_COMMAND:
		hWndCtrl = (HWND) lParam;		// Control handle
		nCode = HIWORD(wParam);			// Notification code
		switch (nCode) {
		case STN_CLICKED:
			if (hWndCtrl == *this->m_Control) {
				// コントロール更新
				this->update();
			}
			break;
		case EN_KILLFOCUS:
			if (hWndCtrl == *this->m_Edit_Max) {
				double tempMax = this->m_Edit_Max->getTextAsFloat();
				if (tempMax != this->m_Temp_Max) {
					this->m_Temp_Max = tempMax;
					PostMessage(GetParent(this->getHandle()), WM_COMMAND,
							(WPARAM) (EN_CHANGE << 16),
							(LPARAM) this->getHandle());
				}
			} else if (hWndCtrl == *this->m_Edit_Min) {
				double tempMin = this->m_Edit_Min->getTextAsFloat();
				if (tempMin != this->m_Temp_Min) {
					this->m_Temp_Min = tempMin;
					PostMessage(GetParent(this->getHandle()), WM_COMMAND,
							(WPARAM) (EN_CHANGE << 16),
							(LPARAM) this->getHandle());
				}
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	// デフォルトのイベント処理
	return this->callDefaultProc(uMsg, wParam, lParam);
}
