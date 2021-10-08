/*
 * ControlPanelShootingData.cpp
 *
 *  Created on: 2016/01/12
 *      Author: PC-EFFECT-011
 */

#include "ControlPanelShootingData.h"
#include "MainWindow.h"
#include "resource.h"

#define MAX_VALUE_ROTATE  9999
#define MIN_VALUE_ROTATE -9999

#define MAX_VALUE_TURN  99999.99
#define MIN_VALUE_TURN -99999.99

#define MAX_VALUE_PANEL 99.999
#define MIN_VALUE_PANEL  0.001

#define DEGREES_OF_CIRCLE 360.0

inline void showOutOfRangeError(double min, double max, int decimalCount) {
	MainWindow::getInstance()->showOutOfRangeError(
	IDS_CAMERA_ADJUST_TITLE, min, max, decimalCount);
}

ControlPanelShootingData::ControlPanelShootingData(GroupBox *pWnd) {
	this->m_pWnd = pWnd;
	this->m_Zoom = ControlPanelBase::DEFAULT_VALUE_ZOOM;
	this->m_Turn = ControlPanelBase::DEFAULT_VALUE_TURN;
	this->m_Enabled = true;
	this->miniFont = NULL;

	this->button_Prev = new PushButton(
			Resource::getString(IDS_PANEL_PICTURE_PREV), WS_VISIBLE | BS_FLAT,
			10, 153, 150, 46, this->m_pWnd);
	this->button_Next = new PushButton(
			Resource::getString(IDS_PANEL_PICTURE_NEXT), WS_VISIBLE | BS_FLAT,
			180, 153, 150, 46, this->m_pWnd);

	this->button_TurnLeft = new PushButton(
			Resource::getString(IDS_PANEL_TURN_LEFT), WS_VISIBLE | BS_FLAT, 10,
			204, 110, 46, this->m_pWnd);
	this->edit_Turn = new InputBox(TEXT("0.00"), WS_VISIBLE | ES_CENTER, 130,
			209, 90, 34, this->m_pWnd);
	this->button_TurnRight = new PushButton(
			Resource::getString(IDS_PANEL_TURN_RIGHT), WS_VISIBLE | BS_FLAT,
			228, 204, 101, 46, this->m_pWnd);
	// (2017/4/3YM追加)『角度一括適用』ボタンを追加
	this->button_TurnAll = new PushButton(
			Resource::getString(IDS_PANEL_TURN_ALL), WS_VISIBLE | BS_FLAT, 10,
			305, 150, 46, this->m_pWnd);

	this->button_ZoomIn = new PushButton(Resource::getString(IDS_PANEL_ZOOM_IN),
	WS_VISIBLE | BS_FLAT, 10, 255, 110, 46, this->m_pWnd);
	this->edit_Zoom = new InputBox(TEXT("0.10"), WS_VISIBLE | ES_CENTER, 130,
			260, 90, 34, this->m_pWnd);
	this->button_ZoomOut = new PushButton(
			Resource::getString(IDS_PANEL_ZOOM_OUT), WS_VISIBLE | BS_FLAT, 228,
			255, 101, 46, this->m_pWnd);
	this->button_ZoomAll = new PushButton(
			Resource::getString(IDS_PANEL_ZOOM_ALL), WS_VISIBLE | BS_FLAT, 180,
			306, 150, 46, this->m_pWnd);

	this->label_Title = new Label(Resource::getString(IDS_PANEL_INFO_TITLE),
	WS_VISIBLE, 30, 450, 300, 24, this->m_pWnd);
	this->label_Info = new Label(
			Resource::getString(IDS_PANEL_INFO_SHOOTINGDATA), WS_VISIBLE, 40,
			480, 300, 96, this->m_pWnd);

}

ControlPanelShootingData::~ControlPanelShootingData() {

	delete this->button_Prev;
	delete this->button_Next;

	delete this->button_ZoomIn;
	delete this->edit_Zoom;
	delete this->button_ZoomOut;
	delete this->button_ZoomAll;

	delete this->button_TurnLeft;
	delete this->edit_Turn;
	delete this->button_TurnRight;
	// (2017/4/3YM追加)『角度一括適用』ボタンを追加
	delete this->button_TurnAll;

	delete this->label_Title;
	delete this->label_Info;
	DeleteObject(miniFont);
}

/**
 * 初期設定
 */
void ControlPanelShootingData::init() {
	// 高さが940px未満の場合、MiniPanel用の配置に変更
	const TCHAR *fontFamily = NULL;
	fontFamily = TEXT("Meiryo UI");

	miniFont = CreateFont(20, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE,
	DEFAULT_CHARSET,
	OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	DEFAULT_PITCH, fontFamily);

	if (MainWindow::getInstance()->IsMiniPanelMode()) {
		// 操作パネルのコントロールを移動
		const int fontHeight = 40;
		const int PictureTop = 120;

		this->edit_Turn->setFont(miniFont);
		this->edit_Zoom->setFont(miniFont);

		this->button_Prev->move(20, PictureTop, 140, fontHeight);
		this->button_Next->move(180, PictureTop, 140, fontHeight);

		const int TurnTop = PictureTop + fontHeight;
		this->button_TurnLeft->move(20, TurnTop, 100, fontHeight);

		this->edit_Turn->move(130, TurnTop + 5, 80, fontHeight - 10);
		this->button_TurnRight->move(220, TurnTop, 100, fontHeight);
		// (2017/4/3YM追加)『角度一括適用』ボタンを追加
		this->button_TurnAll->move(20, TurnTop + fontHeight * 2, 140,
				fontHeight);

		const int ZoomTop = TurnTop + fontHeight;
		this->button_ZoomIn->move(20, ZoomTop, 100, fontHeight);
		this->edit_Zoom->move(130, ZoomTop + 5, 80, fontHeight - 10);
		this->button_ZoomOut->move(220, ZoomTop, 100, fontHeight);
		this->button_ZoomAll->move(180, ZoomTop + fontHeight, 140, fontHeight);

		const int InfoTop = ZoomTop + fontHeight * 2 + 10;
		this->label_Title->move(30, InfoTop);
		this->label_Info->move(40, InfoTop + fontHeight);
	}
}

/**
 * フォントを設定する。
 * @param hFont フォント
 */
void ControlPanelShootingData::setFont(HFONT hFont) {
	this->button_Prev->setFont(hFont);
	this->button_Next->setFont(hFont);

	this->button_ZoomIn->setFont(hFont);
	this->edit_Zoom->setFont(hFont);
	this->button_ZoomOut->setFont(hFont);
	this->button_ZoomAll->setFont(hFont);

	this->button_TurnLeft->setFont(hFont);
	this->edit_Turn->setFont(hFont);
	this->button_TurnRight->setFont(hFont);
	// (2017/4/3YM追加)『角度一括適用』ボタン追加
	this->button_TurnAll->setFont(hFont);

	this->label_Title->setFont(hFont);
	this->label_Info->setFont(hFont);
}

/**
 * 全てのコントロールの表示
 */
void ControlPanelShootingData::allShow() {
	this->button_Prev->show();
	this->button_Next->show();

	this->button_ZoomIn->show();
	this->edit_Zoom->show();
	this->button_ZoomOut->show();
	this->button_ZoomAll->show();

	this->button_TurnLeft->show();
	this->edit_Turn->show();
	this->button_TurnRight->show();
	// (2017/4/3YM追加)『角度一括適用』ボタン追加
	this->button_TurnAll->show();

	this->label_Title->show();
	this->label_Info->show();

	SetFocus(MainWindow::getInstance()->getHandle());
}

/**
 * 全てのコントロールの非表示
 */
void ControlPanelShootingData::allHide() {
	this->button_Prev->hide();
	this->button_Next->hide();

	this->button_ZoomIn->hide();
	this->edit_Zoom->hide();
	this->button_ZoomOut->hide();
	this->button_ZoomAll->hide();

	this->button_TurnLeft->hide();
	this->edit_Turn->hide();
	this->button_TurnRight->hide();
	// (2017/4/3YM追加)『角度一括適用』ボタン追加
	this->button_TurnAll->hide();

	this->label_Title->hide();
	this->label_Info->hide();

}

/**
 * 拡大・縮小値の取得
 */
double ControlPanelShootingData::getZoom() {
	return this->m_Zoom;

}

/**
 * 拡大・縮小値の設定
 */
void ControlPanelShootingData::setZoom(double data) {
	this->m_Zoom = data;
	this->edit_Zoom->setTextAsFloat(data, 2);
}

/**
 * ズームイン
 */
void ControlPanelShootingData::zoomIn(void) {
	this->m_Zoom += ControlPanelBase::SCALE_VALUE_ZOOM;
	if (this->m_Zoom > ControlPanelBase::MAX_VALUE_ZOOM) {
		this->m_Zoom = ControlPanelBase::MAX_VALUE_ZOOM;
		// エラーメッセージ表示
		MainWindow::getInstance()->showMessageBox(
		IDS_ERR_ZOOM, IDS_ERR_ZOOM_CONF,
		MB_OK | MB_ICONERROR);
		this->edit_Zoom->setFocus();
	}
	this->edit_Zoom->setTextAsFloat(this->m_Zoom, 2);
}

/**
 * ズームアウト
 */
void ControlPanelShootingData::zoomOut(void) {
	this->m_Zoom -= ControlPanelBase::SCALE_VALUE_ZOOM;
	if (this->m_Zoom <= ControlPanelBase::MIN_VALUE_ZOOM) {
		this->m_Zoom = ControlPanelBase::MIN_VALUE_ZOOM;
		// エラーメッセージ表示
		MainWindow::getInstance()->showMessageBox(
		IDS_ERR_ZOOM, IDS_ERR_ZOOM_CONF,
		MB_OK | MB_ICONERROR);
		this->edit_Zoom->setFocus();
	}
	this->edit_Zoom->setTextAsFloat(this->m_Zoom, 2);
}

/**
 * 回転角の取得
 */
double ControlPanelShootingData::getTurn() {
	return this->m_Turn;
}

/**
 * 回転角の設定
 */
void ControlPanelShootingData::setTurn(double data) {
	this->m_Turn = data;
	this->edit_Turn->setTextAsFloat(data, 2);
}

/**
 * 反時計回りに回転させる
 */
void ControlPanelShootingData::turnLeft(void) {
	if (this->m_Turn > MIN_VALUE_TURN) {
		this->m_Turn -= ControlPanelBase::SCALE_VALUE_TURN;
		if (this->m_Turn < MIN_VALUE_TURN) {
			this->m_Turn = MIN_VALUE_TURN;
		}
		this->edit_Turn->setTextAsFloat(this->m_Turn, 2);
	} else {
		showOutOfRangeError(MIN_VALUE_TURN, MAX_VALUE_TURN, 2);
	}
}

/**
 * 時計回りに回転させる
 */
void ControlPanelShootingData::turnRight(void) {
	if (this->m_Turn < MAX_VALUE_TURN) {
		this->m_Turn += ControlPanelBase::SCALE_VALUE_TURN;
		if (this->m_Turn > MAX_VALUE_TURN) {
			this->m_Turn = MAX_VALUE_TURN;
		}
		this->edit_Turn->setTextAsFloat(this->m_Turn, 2);
	} else {
		showOutOfRangeError(MIN_VALUE_TURN, MAX_VALUE_TURN, 2);
	}
}

/**
 * ボタンの有効化
 */
void ControlPanelShootingData::enableButton() {
	this->button_Prev->enable();
	this->button_Next->enable();

	this->button_ZoomIn->enable();
	this->edit_Zoom->enable();
	this->button_ZoomOut->enable();
	this->button_ZoomAll->enable();

	this->button_TurnLeft->enable();
	this->edit_Turn->enable();
	this->button_TurnRight->enable();
	// (2017/4/3YM追加)『角度一括適用』ボタン追加
	this->button_TurnAll->enable();

	this->m_Enabled = true;
}

/**
 * ボタンの無効化
 */
void ControlPanelShootingData::disableButton() {
	this->button_Prev->disable();
	this->button_Next->disable();

	this->button_ZoomIn->disable();
	this->edit_Zoom->disable();
	this->button_ZoomOut->disable();
	this->button_ZoomAll->disable();

	this->button_TurnLeft->disable();
	this->edit_Turn->disable();
	this->button_TurnRight->disable();
	// (2017/4/3YM追加)『角度一括適用』ボタン追加
	this->button_TurnAll->disable();

	this->m_Enabled = false;
}

/**
 * コマンド発行時の処理を行う。
 * @param uMsg メッセージID(WM_COMMAND)
 * @param wParam 1つ目のパラメータ
 * @param hwndControl コマンドが発生したコントロールのハンドル
 */
LRESULT ControlPanelShootingData::onCommand(UINT uMsg, WPARAM wParam,
		HWND hwndControl) {
	switch (HIWORD(wParam)) {
	case BN_CLICKED:
		if (hwndControl == *this->button_Prev) {
			SetFocus(MainWindow::getInstance()->getHandle());
			MainWindow::getInstance()->canvasUpdatePrev();
			break;
		} else if (hwndControl == *this->button_Next) {
			SetFocus(MainWindow::getInstance()->getHandle());
			MainWindow::getInstance()->canvasUpdateNext();
			break;
		} else if (hwndControl == *this->button_ZoomIn) {
			this->zoomIn();
		} else if (hwndControl == *this->button_ZoomOut) {
			this->zoomOut();
		} else if (hwndControl == *this->button_ZoomAll) {
			MainWindow::getInstance()->SetHeightAll();
		} else if (hwndControl == *this->button_TurnLeft) {
			this->turnLeft();
		} else if (hwndControl == *this->button_TurnRight) {
			this->turnRight();
		} else if (hwndControl == *this->button_TurnAll) // (2017/4/3YM追加)『角度一括適用』ボタン追加
				{
			MainWindow::getInstance()->SetTurnAll();
		} else {
			break;
		}
		SetFocus(MainWindow::getInstance()->getHandle());
		MainWindow::getInstance()->canvasUpdate();
		break;

	case EN_KILLFOCUS:
		if (hwndControl == *this->edit_Zoom) {
			double data = this->edit_Zoom->getTextAsFloat();
			if (data > ControlPanelBase::MAX_VALUE_ZOOM) {
				data = ControlPanelBase::MAX_VALUE_ZOOM;
				// エラーメッセージ表示
				MainWindow::getInstance()->showMessageBox(
				IDS_ERR_ZOOM, IDS_ERR_ZOOM_CONF,
				MB_OK | MB_ICONERROR);
				this->setZoom(this->getZoom());
				this->edit_Zoom->setFocus();
			} else if (data < ControlPanelBase::MIN_VALUE_ZOOM) {
				data = ControlPanelBase::MIN_VALUE_ZOOM;
				// エラーメッセージ表示
				MainWindow::getInstance()->showMessageBox(
				IDS_ERR_ZOOM, IDS_ERR_ZOOM_CONF,
				MB_OK | MB_ICONERROR);
				this->setZoom(this->getZoom());
				this->edit_Zoom->setFocus();
			} else {
				this->setZoom(data);
			}
		} else if (hwndControl == *this->edit_Turn) {
			double turn = this->edit_Turn->getTextAsFloat();
			if (turn > MAX_VALUE_TURN) {
				// エラーメッセージ表示
				showOutOfRangeError(MIN_VALUE_TURN, MAX_VALUE_TURN, 2);
				this->setTurn(this->m_Turn);
				this->edit_Turn->setFocus();
			} else if (turn < MIN_VALUE_TURN) {
				// エラーメッセージ表示
				showOutOfRangeError(MIN_VALUE_TURN, MAX_VALUE_TURN, 2);
				this->setTurn(this->m_Turn);
				this->edit_Turn->setFocus();
			} else {
				this->m_Turn = turn;
			}
		} else {
			break;
		}
		MainWindow::getInstance()->canvasUpdate();
		break;

	default:
		break;
	}

	return 0;
}

/**
 * キー入力時の処理を行う。
 * @param uMsg メッセージID
 * @param wParam 1つ目のパラメータ
 */
LRESULT ControlPanelShootingData::onKey(UINT uMsg, WPARAM wParam) {
	if (this->m_Enabled != true) {
		return 0;
	}
	switch (uMsg) {
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_UP:
			// Ctrlキーが押されている場合
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				this->zoomIn();
				MainWindow::getInstance()->canvasUpdate();
			}
			break;

		case VK_DOWN:
			// Ctrlキーが押されている場合
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				this->zoomOut();
				MainWindow::getInstance()->canvasUpdate();
			}
			break;

		case VK_LEFT:
			// Ctrlキーが押されている場合
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				this->turnLeft();
				MainWindow::getInstance()->canvasUpdate();
			} else {
				MainWindow::getInstance()->canvasUpdatePrev();
			}
			break;

		case VK_RIGHT:
			// Ctrlキーが押されている場合
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				this->turnRight();
				MainWindow::getInstance()->canvasUpdate();
			} else {
				MainWindow::getInstance()->canvasUpdateNext();
			}
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	return 0;
}
