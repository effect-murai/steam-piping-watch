/*
 * ControlPanelCamera.cpp
 *
 *  Created on: 2016/01/12
 *      Author: PC-EFFECT-011
 */

#include "ControlPanelCamera.h"
#include "MainWindow.h"
#include "resource.h"
#include "resultData.h"
#include "Canvas.h"
#include "CommonData.h"
#define PANELSHOWENABLE
#define MAX_VALUE_MOVE  9999999
#define MIN_VALUE_MOVE -9999999
#define MAX_VALUE_TURN  99999.99
#define MIN_VALUE_TURN -99999.99

inline void showOutOfRangeError(double min, double max, int decimalCount) {
	MainWindow::getInstance()->showOutOfRangeError(IDS_CAMERA_ADJUST_TITLE, min,
			max, decimalCount);
}
int prevradiocheck = 0;
extern ResultData *result;

ControlPanelCamera::ControlPanelCamera(GroupBox *pWnd) {
	this->m_pWnd = pWnd;
	this->m_MoveV = ControlPanelBase::DEFAULT_VALUE_MOVE;
	this->m_MoveH = ControlPanelBase::DEFAULT_VALUE_MOVE;
	this->m_Zoom = ControlPanelBase::DEFAULT_VALUE_ZOOM;
	this->m_Turn = ControlPanelBase::DEFAULT_VALUE_TURN;
	this->m_Enabled = true;
	this->m_datatype = ControlPanelBase::DEFAULT_VALUE_DATA;

	this->button_Prev = new PushButton(
			Resource::getString(IDS_PANEL_PICTURE_PREV), WS_VISIBLE | BS_FLAT,
			10, 180, 140, 40, this->m_pWnd);
	this->button_Next = new PushButton(
			Resource::getString(IDS_PANEL_PICTURE_NEXT), WS_VISIBLE | BS_FLAT,
			190, 180, 140, 40, this->m_pWnd);

	this->button_Up = new PushButton(Resource::getString(IDS_PANEL_MOVE_UP),
	WS_VISIBLE | BS_FLAT, 10, 225, 110, 40, this->m_pWnd);
	this->edit_MoveV = new InputBox(TEXT("0"), WS_VISIBLE | ES_RIGHT, 130, 230,
			80, 30, this->m_pWnd);
	this->button_Down = new PushButton(Resource::getString(IDS_PANEL_MOVE_DOWN),
	WS_VISIBLE | BS_FLAT, 220, 225, 110, 40, this->m_pWnd);
	this->button_Left = new PushButton(Resource::getString(IDS_PANEL_MOVE_LEFT),
	WS_VISIBLE | BS_FLAT, 10, 270, 110, 40, this->m_pWnd);
	this->edit_MoveH = new InputBox(TEXT("0"), WS_VISIBLE | ES_RIGHT, 130, 275,
			80, 30, this->m_pWnd);
	this->button_Right = new PushButton(
			Resource::getString(IDS_PANEL_MOVE_RIGHT), WS_VISIBLE | BS_FLAT,
			220, 270, 110, 40, this->m_pWnd);
	this->button_TurnLeft = new PushButton(
			Resource::getString(IDS_PANEL_TURN_LEFT), WS_VISIBLE | BS_FLAT, 10,
			315, 110, 40, this->m_pWnd);
	this->edit_Turn = new InputBox(TEXT("0.00"), WS_VISIBLE | ES_RIGHT, 130,
			320, 80, 30, this->m_pWnd);
	this->button_TurnRight = new PushButton(
			Resource::getString(IDS_PANEL_TURN_RIGHT), WS_VISIBLE | BS_FLAT,
			220, 315, 110, 40, this->m_pWnd);
	this->button_ZoomIn = new PushButton(Resource::getString(IDS_PANEL_ZOOM_IN),
	WS_VISIBLE | BS_FLAT, 10, 360, 110, 40, this->m_pWnd);
	this->edit_Zoom = new InputBox(TEXT("1.00"), WS_VISIBLE | ES_RIGHT, 130,
			365, 80, 30, this->m_pWnd);
	this->button_ZoomOut = new PushButton(
			Resource::getString(IDS_PANEL_ZOOM_OUT), WS_VISIBLE | BS_FLAT, 220,
			360, 110, 40, this->m_pWnd);
	this->button_Reset = new PushButton(
			Resource::getString(IDS_PANEL_RESET_BUTTON), WS_VISIBLE | BS_FLAT,
			10, 420, 110, 40, this->m_pWnd);

#ifdef PANELSHOWENABLE
	this->radio_Setall = new RadioButton(Resource::getString(IDS_PANEL_SETALL),
	WS_VISIBLE | BS_NOTIFY, 20, 130, 80, 24, this->m_pWnd);
	this->radio_Setone = new RadioButton(Resource::getString(IDS_PANEL_SETONE),
	WS_VISIBLE | BS_NOTIFY, 200, 130, 80, 24, this->m_pWnd);

#else
	this->radio_Setall = new RadioButton(
	Resource::getString(IDS_PANEL_SETALL), WS_VISIBLE | BS_NOTIFY,20, 130, 80, 24, this->m_pWnd);
	this->radio_Setone = new RadioButton(
	Resource::getString(IDS_PANEL_SETONE), WS_VISIBLE | BS_NOTIFY,100, 130, 80, 24, this->m_pWnd);
#endif

	//(2019/11/26LEE）位置を変更。
	this->label_Title = new Label(Resource::getString(IDS_PANEL_INFO_TITLE),
	WS_VISIBLE, 30, 510, 300, 24, this->m_pWnd);
	this->label_Info = new Label(Resource::getString(IDS_PANEL_INFO_CAMERA),
	WS_VISIBLE, 40, 540, 300, 96, this->m_pWnd);

	// (2019/11/26LEE) 全体モード案内するために追加。
	this->all_mode_Title = new Label(
			Resource::getString(IDS_PANEL_INFO_CAMERA_ALLMODE_TITLE),
			WS_VISIBLE, 140, 420, 300, 24, this->m_pWnd);
	this->all_mode_Info = new Label(
			Resource::getString(IDS_PANEL_INFO_CAMERA_ALLMODE_INFO), WS_VISIBLE,
			140, 445, 300, 20, this->m_pWnd);

}

ControlPanelCamera::~ControlPanelCamera() {
	delete this->button_Up;
	delete this->edit_MoveV;
	delete this->button_Down;

	delete this->button_Left;
	delete this->edit_MoveH;
	delete this->button_Right;

	delete this->button_ZoomIn;
	delete this->edit_Zoom;
	delete this->button_ZoomOut;

	delete this->button_TurnLeft;
	delete this->edit_Turn;
	delete this->button_TurnRight;

	delete this->label_Title;
	delete this->label_Info;

	delete this->all_mode_Title;
	delete this->all_mode_Info;

	// (2019/10/25LEE) 追加。
	delete this->radio_Setall;
	delete this->radio_Setone;
	delete this->button_Reset; //(2019/11/06LEE)追加。

	// (2020/01/23LEE) 追加
	delete this->button_Prev;
	delete this->button_Next;

}

/**
 * 初期設定
 */
void ControlPanelCamera::init() {
	/**
	 * 高さが940px未満の場合、MiniPanel用の配置に変更
	 */
	this->setall();

	if (MainWindow::getInstance()->IsMiniPanelMode()) {
		// 操作パネルのコントロールを移動
		const int fontHeight = 21;
		const int Height = 25;

		const int radio = 110;
		this->radio_Setall->move(40, radio, 80, 24);
		this->radio_Setone->move(230, radio, 80, 24);

		const int changeturnTop = radio + Height;
		this->button_Prev->move(20, changeturnTop, 140, 20);
		this->button_Next->move(186, changeturnTop, 140, 20);

		const int MoveHTop = changeturnTop + Height;
		this->button_Up->move(20, MoveHTop, 110, 20);
		this->edit_MoveV->move(133, MoveHTop, 80, 20);
		this->button_Down->move(216, MoveHTop, 110, 20);

		const int MoveVTop = MoveHTop + Height;
		this->button_Left->move(20, MoveVTop, 110, 20);
		this->edit_MoveH->move(133, MoveVTop, 80, 20);
		this->button_Right->move(216, MoveVTop, 110, 20);

		const int TurnTop = MoveVTop + Height;
		this->button_TurnLeft->move(20, TurnTop, 110, 20);
		this->edit_Turn->move(133, TurnTop, 80, 20);
		this->button_TurnRight->move(216, TurnTop, 110, 20);

		const int ZoomTop = TurnTop + Height;
		this->button_ZoomIn->move(20, ZoomTop, 110, 20);
		this->edit_Zoom->move(133, ZoomTop, 80, 20);
		this->button_ZoomOut->move(216, ZoomTop, 110, 20);

		const int ModeTop = ZoomTop + Height;
		this->all_mode_Title->move(30, ModeTop);
		this->all_mode_Info->move(30, ModeTop + fontHeight);

		const int ResetTop = ModeTop + (Height * 2) - 5;
		this->button_Reset->move(20, ResetTop, 100, 34);

		const int InfoTop = ResetTop + Height + 10;
		this->label_Title->move(30, InfoTop);
		this->label_Info->move(40, InfoTop + fontHeight);

	}
}

/**
 * フォントを設定する。
 * @param hFont フォント
 */
void ControlPanelCamera::setFont(HFONT hFont) {
	this->button_Up->setFont(hFont);
	this->edit_MoveV->setFont(hFont);
	this->button_Down->setFont(hFont);

	this->button_Left->setFont(hFont);
	this->edit_MoveH->setFont(hFont);
	this->button_Right->setFont(hFont);

	this->button_ZoomIn->setFont(hFont);
	this->edit_Zoom->setFont(hFont);
	this->button_ZoomOut->setFont(hFont);

	this->button_TurnLeft->setFont(hFont);
	this->edit_Turn->setFont(hFont);
	this->button_TurnRight->setFont(hFont);

	this->label_Title->setFont(hFont);
	this->label_Info->setFont(hFont);

	this->all_mode_Title->setFont(hFont);
	this->all_mode_Info->setFont(hFont);

	this->radio_Setone->setFont(hFont);
	this->radio_Setall->setFont(hFont);
	this->button_Reset->setFont(hFont);

	this->button_Prev->setFont(hFont);
	this->button_Next->setFont(hFont);

}

/**
 * 全てのコントロールの表示
 */
void ControlPanelCamera::allShow() {
	this->button_Prev->show();
	this->button_Next->show();

	this->button_Up->show();
	this->edit_MoveV->show();
	this->button_Down->show();

	this->button_Left->show();
	this->edit_MoveH->show();
	this->button_Right->show();

	this->button_ZoomIn->show();
	this->edit_Zoom->show();
	this->button_ZoomOut->show();

	this->button_TurnLeft->show();
	this->edit_Turn->show();
	this->button_TurnRight->show();

	this->label_Title->show();
	this->label_Info->show();

	this->all_mode_Title->show();
	this->all_mode_Info->show();

	this->radio_Setone->show();
	this->radio_Setall->show();
	this->button_Reset->show();

	//(2019/11/06LEE) 追加。
	SetFocus(MainWindow::getInstance()->getHandle());
}

/**
 * 全てのコントロールの非表示
 */
void ControlPanelCamera::allHide() {
	this->button_Prev->hide();
	this->button_Next->hide();

	this->button_Up->hide();
	this->edit_MoveV->hide();
	this->button_Down->hide();

	this->button_Left->hide();
	this->edit_MoveH->hide();
	this->button_Right->hide();

	this->button_ZoomIn->hide();
	this->edit_Zoom->hide();
	this->button_ZoomOut->hide();

	this->button_TurnLeft->hide();
	this->edit_Turn->hide();
	this->button_TurnRight->hide();

	this->label_Title->hide();
	this->label_Info->hide();

	this->all_mode_Title->hide();
	this->all_mode_Info->hide();

	this->radio_Setone->hide();
	this->radio_Setall->hide();
	this->button_Reset->hide();
}

/**
 * 移動値(垂直方向)の取得
 */
double ControlPanelCamera::getMoveV() {
	return this->m_MoveV;
}
/**
 * 移動値(垂直方向)の設定
 */
void ControlPanelCamera::setMoveV(double data) {
	this->m_MoveV = data;
	this->edit_MoveV->setTextAsFloat(data, 0);
}

/**
 * 移動値(水平方向)の取得
 */
double ControlPanelCamera::getMoveH() {
	return this->m_MoveH;
}
/**
 * 移動値(水平方向)の設定
 */
void ControlPanelCamera::setMoveH(double data) {
	this->m_MoveH = data;
	this->edit_MoveH->setTextAsFloat(data, 0);
}

/**
 * 上方向へ移動
 */
void ControlPanelCamera::moveUp(void) {
	if (this->m_MoveV < MAX_VALUE_MOVE) {
		this->m_MoveV += ControlPanelBase::SCALE_VALUE_MOVE;
		if (this->m_MoveV > MAX_VALUE_MOVE) {
			this->m_MoveV = MAX_VALUE_MOVE;
		}
		this->edit_MoveV->setTextAsFloat(this->m_MoveV, 0);
	} else {
		showOutOfRangeError(MIN_VALUE_MOVE, MAX_VALUE_MOVE, 0);
	}
}
/**
 * 下方向へ移動
 */
void ControlPanelCamera::moveDown(void) {
	if (this->m_MoveV > MIN_VALUE_MOVE) {
		this->m_MoveV -= ControlPanelBase::SCALE_VALUE_MOVE;
		if (this->m_MoveV < MIN_VALUE_MOVE) {
			this->m_MoveV = MIN_VALUE_MOVE;
		}
		this->edit_MoveV->setTextAsFloat(this->m_MoveV, 0);
	} else {
		showOutOfRangeError(MIN_VALUE_MOVE, MAX_VALUE_MOVE, 0);
	}
}
/**
 * 左方向へ移動
 */
void ControlPanelCamera::moveLeft(void) {
	if (this->m_MoveH > MIN_VALUE_MOVE) {
		this->m_MoveH -= ControlPanelBase::SCALE_VALUE_MOVE;
		if (this->m_MoveH < MIN_VALUE_MOVE) {
			this->m_MoveH = MIN_VALUE_MOVE;
		}
		this->edit_MoveH->setTextAsFloat(this->m_MoveH, 0);
	} else {
		showOutOfRangeError(MIN_VALUE_MOVE, MAX_VALUE_MOVE, 0);
	}
}
/**
 * 右方向へ移動
 */
void ControlPanelCamera::moveRight(void) {
	if (this->m_MoveH < MAX_VALUE_MOVE) {
		this->m_MoveH += ControlPanelBase::SCALE_VALUE_MOVE;
		if (this->m_MoveH > MAX_VALUE_MOVE) {
			this->m_MoveH = MAX_VALUE_MOVE;
		}
		this->edit_MoveH->setTextAsFloat(this->m_MoveH, 0);
	} else {
		showOutOfRangeError(MIN_VALUE_MOVE, MAX_VALUE_MOVE, 0);
	}
}

/**
 * 拡大・縮小値の取得
 */
double ControlPanelCamera::getZoom() {
	return this->m_Zoom;
}
/**
 * 拡大・縮小値の設定
 */
void ControlPanelCamera::setZoom(double data) {
	this->m_Zoom = data;
	this->edit_Zoom->setTextAsFloat(data, 2);
}

/**
 * ズームイン
 */
void ControlPanelCamera::zoomIn(void) {
	this->m_Zoom += ControlPanelBase::SCALE_VALUE_ZOOM;
	if (this->m_Zoom > ControlPanelBase::MAX_VALUE_ZOOM) {
		this->m_Zoom = ControlPanelBase::MAX_VALUE_ZOOM;
		// エラーメッセージ表示
		MainWindow::getInstance()->showMessageBox(IDS_ERR_ZOOM,
		IDS_ERR_ZOOM_CONF, MB_OK | MB_ICONERROR);
		this->edit_Zoom->setFocus();
	}
	this->edit_Zoom->setTextAsFloat(this->m_Zoom, 2);
}

/**
 * ズームアウト
 */
void ControlPanelCamera::zoomOut(void) {
	this->m_Zoom -= ControlPanelBase::SCALE_VALUE_ZOOM;
	if (this->m_Zoom <= ControlPanelBase::MIN_VALUE_ZOOM) {
		this->m_Zoom = ControlPanelBase::MIN_VALUE_ZOOM;
		// エラーメッセージ表示
		MainWindow::getInstance()->showMessageBox(IDS_ERR_ZOOM,
		IDS_ERR_ZOOM_CONF, MB_OK | MB_ICONERROR);
		this->edit_Zoom->setFocus();
	}
	this->edit_Zoom->setTextAsFloat(this->m_Zoom, 2);

}

/**
 * 回転角の取得
 */
double ControlPanelCamera::getTurn() {
	return this->m_Turn;
}
/**
 * 回転角の設定
 */
void ControlPanelCamera::setTurn(double data) {
	this->m_Turn = data;
	this->edit_Turn->setTextAsFloat(data, 2);
}

/**
 * 反時計回りに回転させる
 */
void ControlPanelCamera::turnLeft(void) {
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
void ControlPanelCamera::turnRight(void) {
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

// (2019/11/06LEE) 追加。
void ControlPanelCamera::setall(void) {

	radio_Setall->setCheck();
	radio_Setone->setCheck(false);

	prevradiocheck = 0;

}

void ControlPanelCamera::setone(void) {

	radio_Setone->setCheck();
	radio_Setall->setCheck(false);

	prevradiocheck = 1;
}
void ControlPanelCamera::reset(void) {
	radio_Setall->setCheck();
	radio_Setone->setCheck(false);
	prevradiocheck = 0;
	double x, y, vertical, horizontal, direction;

	// (2019/11/29LEE) 修正中
	result->getalldata(&x, &y, &vertical, &horizontal, &direction);
	this->edit_Turn->setTextAsFloat(direction, 2);
	this->m_Turn = direction;
	this->edit_MoveH->setTextAsFloat(x, 2);
	this->m_MoveH = x;
	this->edit_Zoom->setTextAsFloat(vertical, 2);
	this->m_Zoom = vertical;
	this->edit_MoveV->setTextAsFloat(y, 2);
	this->m_MoveV = y;
}
// (2019/11/06LEE) 追加。

int ControlPanelCamera::getsetingtype() {

	// (2017/5/31YM)選択したボタン別に値を返す
	int st = radio_Setall->getCheck();

	if (st == 1) {
		prevradiocheck = 0;
		return 0;		//　全体：１
	}
	st = radio_Setone->getCheck();
	if (st == 1) {
		prevradiocheck = 1;
		return 1;		//　個別：２
	}

	prevradiocheck = 0;
	return 0;
}
// (2019/11/08LEE) data typeを読んで設定。

int ControlPanelCamera::getdatatype() {
	return this->m_datatype;
}
/**
 * (2019/11/08LEE) data typeを読んで設定。
 */
void ControlPanelCamera::setdatatype(int data) {

	this->m_datatype = data;
	if (data == 0) {
		this->setall();
	} else if (data == 1) {
		this->setone();
	} else {
		this->setall();
	}
}

/**
 * ボタンの有効化
 */
void ControlPanelCamera::enableButton() {

	this->button_Prev->enable();
	this->button_Next->enable();

	this->button_Up->enable();
	this->edit_MoveV->enable();
	this->button_Down->enable();

	this->button_Left->enable();
	this->edit_MoveH->enable();
	this->button_Right->enable();

	this->button_ZoomIn->enable();
	this->edit_Zoom->enable();
	this->button_ZoomOut->enable();

	this->button_TurnLeft->enable();
	this->edit_Turn->enable();
	this->button_TurnRight->enable();
	this->button_Reset->enable();		//(2019/11/06LEE) 追加。
	this->m_Enabled = true;
}

/**
 * ボタンの無効化
 */
void ControlPanelCamera::disableButton() {

	this->button_Prev->disable();
	this->button_Next->disable();

	this->button_Up->disable();
	this->edit_MoveV->disable();
	this->button_Down->disable();

	this->button_Left->disable();
	this->edit_MoveH->disable();
	this->button_Right->disable();

	this->button_ZoomIn->disable();
	this->edit_Zoom->disable();
	this->button_ZoomOut->disable();

	this->button_TurnLeft->disable();
	this->edit_Turn->disable();
	this->button_TurnRight->disable();

	this->button_Reset->disable(); //(2019/11/06LEE) 追加。

	this->m_Enabled = false;
}

/**
 * コマンド発行時の処理を行う。
 * @param uMsg メッセージID(WM_COMMAND)
 * @param wParam 1つ目のパラメータ
 * @param hwndControl コマンドが発生したコントロールのハンドル
 */
LRESULT ControlPanelCamera::onCommand(UINT uMsg, WPARAM wParam,
		HWND hwndControl) {
	//HWND TEMP;

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
		} else if (hwndControl == *this->button_Up) {
			this->moveUp();
		} else if (hwndControl == *this->button_Down) {
			this->moveDown();
		} else if (hwndControl == *this->button_Left) {
			this->moveLeft();
		} else if (hwndControl == *this->button_Right) {
			this->moveRight();
		} else if (hwndControl == *this->button_ZoomIn) {
			this->zoomIn();
		} else if (hwndControl == *this->button_ZoomOut) {
			this->zoomOut();
		} else if (hwndControl == *this->button_TurnLeft) {
			this->turnLeft();
		} else if (hwndControl == *this->button_TurnRight) {
			this->turnRight();
		} else if (hwndControl == *this->radio_Setall) {
			// (2019/11/27LEE) 追加
			if (prevradiocheck == 1) {
				int checkallset = MainWindow::getInstance()->showMessageBox(
				IDS_PANEL_INFO_SETALL_INFO,
				IDS_PANEL_INFO_SETALL_TITLE,
				MB_YESNO);
				if (checkallset == IDYES) {
					this->reset();
				} else {
					this->setone();
				}
			}
		} else if (hwndControl == *this->radio_Setone) {
			this->setone();
		} else if (hwndControl == *this->button_Reset) {
			// (2019/11/06LEE) 追加。
			if (MainWindow::getInstance()->Reset()) {
				this->reset();
			}
		} else {
			break;
		}

		SetFocus(MainWindow::getInstance()->getHandle());
		MainWindow::getInstance()->canvasUpdate();
		break;

		//case EN_CHANGE:
	case EN_KILLFOCUS:
		if (hwndControl == *this->edit_MoveV) {
			double moveV = this->edit_MoveV->getTextAsFloat();
			if (moveV > MAX_VALUE_MOVE) {
				// エラーメッセージ表示
				showOutOfRangeError(MIN_VALUE_MOVE, MAX_VALUE_MOVE, 0);
				this->setMoveV(this->m_MoveV);
				this->edit_MoveV->setFocus();
				this->moveLeft(); // (2019/11/29LEE)
			} else if (moveV < MIN_VALUE_MOVE) {
				// エラーメッセージ表示
				showOutOfRangeError(MIN_VALUE_MOVE, MAX_VALUE_MOVE, 0);
				this->setMoveV(this->m_MoveV);
				this->edit_MoveV->setFocus();
			} else {
				this->m_MoveV = moveV;
			}
		} else if (hwndControl == *this->edit_MoveH) {
			double moveH = this->edit_MoveH->getTextAsFloat();
			if (moveH > MAX_VALUE_MOVE) {
				// エラーメッセージ表示
				showOutOfRangeError(MIN_VALUE_MOVE, MAX_VALUE_MOVE, 0);
				this->setMoveH(this->m_MoveH);
				this->edit_MoveH->setFocus();
			} else if (moveH < MIN_VALUE_MOVE) {
				// エラーメッセージ表示
				showOutOfRangeError(MIN_VALUE_MOVE, MAX_VALUE_MOVE, 0);
				this->setMoveH(this->m_MoveH);
				this->edit_MoveH->setFocus();
			} else {
				this->m_MoveH = moveH;
			}
		} else if (hwndControl == *this->edit_Zoom) {
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
LRESULT ControlPanelCamera::onKey(UINT uMsg, WPARAM wParam) {
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
			} else {
				this->moveUp();
			}
			MainWindow::getInstance()->canvasUpdate();
			break;

		case VK_DOWN:
			// Ctrlキーが押されている場合
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				this->zoomOut();
			} else {
				this->moveDown();
			}
			MainWindow::getInstance()->canvasUpdate();
			break;

		case VK_LEFT:
			// Ctrlキーが押されている場合
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				this->turnLeft();
			} else {
				this->moveLeft();
			}
			MainWindow::getInstance()->canvasUpdate();
			break;

		case VK_RIGHT:
			// Ctrlキーが押されている場合
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				this->turnRight();
			} else {
				this->moveRight();
			}
			MainWindow::getInstance()->canvasUpdate();
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
