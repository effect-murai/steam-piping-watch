/*
 * ControlPanelArea.cpp
 *
 *  Created on: 2016/02/29
 *      Author: PC-EFFECT-011
 */

#include "ControlPanelArea.h"
#include "MainWindow.h"
#include "resource.h"

ControlPanelArea::ControlPanelArea(GroupBox *pWnd) {
	this->m_pWnd = pWnd;

	this->button_Clear = new PushButton(Resource::getString(IDS_SET_AREA_CLEAR),
	WS_VISIBLE, 10, 150, 320, 36, this->m_pWnd);

	this->label_Title = new Label(Resource::getString(IDS_PANEL_INFO_TITLE),
	WS_VISIBLE, 30, 230, 300, 24, this->m_pWnd);
	this->label_Info = new Label(Resource::getString(IDS_PANEL_INFO_AREA),
	WS_VISIBLE, 40, 260, 300, 156, this->m_pWnd);

}

ControlPanelArea::~ControlPanelArea() {
	delete this->button_Clear;
	delete this->label_Title;
	delete this->label_Info;
}

/**
 * 初期設定
 */
void ControlPanelArea::init() {
	this->button_Clear->disable();

	/**
	 * 高さが940px未満の場合、MiniPanel用の配置に変更
	 */
	if (MainWindow::getInstance()->IsMiniPanelMode()) {
		// 操作パネルのコントロールを移動
		const int fontHeight = 21;
		const int Height = 38;
		const int ButtonTop = 120;
		this->button_Clear->move(10, ButtonTop);
		const int InfoTop = ButtonTop + Height + 10;
		this->label_Title->move(30, InfoTop);
		this->label_Info->move(40, InfoTop + fontHeight);
	}
}

/**
 * フォントを設定する。
 *
 * @param hFont フォント
 */
void ControlPanelArea::setFont(HFONT hFont) {
	this->button_Clear->setFont(hFont);
	this->label_Title->setFont(hFont);
	this->label_Info->setFont(hFont);
}

/**
 * 全てのコントロールの表示
 */
void ControlPanelArea::allShow() {
	this->button_Clear->show();
	this->label_Title->show();
	this->label_Info->show();
}

/**
 * 全てのコントロールの非表示
 */
void ControlPanelArea::allHide() {
	this->button_Clear->hide();
	this->label_Title->hide();
	this->label_Info->hide();
}

/**
 * クリアボタンの有効化
 */
void ControlPanelArea::enableClearButton() {
	this->button_Clear->enable();
}

/**
 * ボタンの有効化
 */
void ControlPanelArea::enableButton() {
	this->button_Clear->enable();
}

/**
 * ボタンの無効化
 */
void ControlPanelArea::disableButton() {
	this->button_Clear->disable();

}

/**
 * コマンド発行時の処理を行う。
 * @param uMsg メッセージID(WM_COMMAND)
 * @param wParam 1つ目のパラメータ
 * @param hwndControl コマンドが発生したコントロールのハンドル
 */
LRESULT ControlPanelArea::onCommand(UINT uMsg, WPARAM wParam,
		HWND hwndControl) {
	switch (HIWORD(wParam)) {
	case BN_CLICKED:
		if (hwndControl == *this->button_Clear) {
			MainWindow::getInstance()->AreaSettingClear();
		}
		break;

	default:
		break;
	}
	return 1;
}
