/*
 * ControlPanelPanelSetting.cpp
 *
 *  Created on: 2016/01/29
 *      Author: PC-EFFECT-002
 */

#include "ControlPanelPanelSetting.h"
#include "MainWindow.h"
#include "PanelInfo.h"
#include "resource.h"

extern PanelData *panelData;

#define parent (MainWindow::getInstance())

ControlPanelPanelSetting::ControlPanelPanelSetting(GroupBox *pWnd) {
	this->m_pWnd = pWnd;
	this->m_Enabled = true;

	this->panelSettingGroupBox = new GroupBox(
			Resource::getString(IDS_PANEL_INFO_BASE), WS_VISIBLE | BS_NOTIFY,
			10, 150, 320, 90, this->m_pWnd);
	this->panel_Name = new Label(Resource::getString(IDS_PANEL_INFO_NAME),
	WS_VISIBLE, 35, 30, 150, 24, panelSettingGroupBox);
	this->edit_PanelName = new EditBox(Resource::getString(IDS_NULL),
	WS_VISIBLE, 170, 30, 90, 24, panelSettingGroupBox);
	this->Label_Hotspot_Count_Name = new Label(
			Resource::getString(IDS_PANEL_INFO_HOTSPOT_COUNT), WS_VISIBLE, 35,
			60, 150, 24, panelSettingGroupBox);
	this->Label_HotspotCount = new Label(Resource::getString(IDS_NULL),
	WS_VISIBLE, 170, 60, 90, 24, panelSettingGroupBox);

	this->settingType = new GroupBox(Resource::getString(IDS_SETTINGTYPE),
	WS_VISIBLE | BS_NOTIFY, 10, 250, 320, 90, this->m_pWnd);
	this->point2Set = new RadioButton(Resource::getString(IDS_POINT2SET),
	WS_VISIBLE | BS_NOTIFY | WS_GROUP, 10, 30, 140, 24, settingType);
	this->point4Set = new RadioButton(Resource::getString(IDS_POINT4SET),
	WS_VISIBLE | BS_NOTIFY, 170, 30, 140, 24, settingType);
	this->copySelect = new RadioButton(Resource::getString(IDS_COPYSELECT),
	WS_VISIBLE | BS_NOTIFY, 10, 60, 140, 24, settingType);
	this->moveSelect = new RadioButton(Resource::getString(IDS_MOVESELECT),
	WS_VISIBLE | BS_NOTIFY, 170, 60, 140, 24, settingType);

	this->button_Delete = new PushButton(Resource::getString(IDS_PANEL_DELETE),
	WS_VISIBLE | BS_FLAT, 10, 350, 320, 36, this->m_pWnd);
	this->button_SaveHotspot = new PushButton(
			Resource::getString(IDS_SAVE_HOTSPOT_INFO), WS_VISIBLE | BS_FLAT,
			10, 400, 320, 36, this->m_pWnd);

	this->label_Title = new Label(Resource::getString(IDS_PANEL_INFO_TITLE),
	WS_VISIBLE, 30, 500, 300, 24, this->m_pWnd);
	this->label_Info = new Label(
			Resource::getString(IDS_PANEL_INFO_PANELSETTING), WS_VISIBLE, 40,
			530, 300, 150, this->m_pWnd);
}

ControlPanelPanelSetting::~ControlPanelPanelSetting() {
	delete panelSettingGroupBox;
	delete panel_Name;
	delete edit_PanelName;
	delete Label_Hotspot_Count_Name;
	delete Label_HotspotCount;
	delete button_Delete;
	delete button_SaveHotspot;
	delete settingType;
	delete point2Set;
	delete point4Set;
	delete copySelect;
	delete moveSelect;
	delete label_Title;
	delete label_Info;
}

/**
 * 初期設定
 */
void ControlPanelPanelSetting::init() {
	point2Set->setCheck();
	/**
	 * 高さが940px未満の場合、MiniPanel用の配置に変更
	 */
	if (MainWindow::getInstance()->IsMiniPanelMode()) {
		// 操作パネルのコントロールを移動
		const int fontHeight = 21;
		const int GroupBoxTop = 120;
		this->panelSettingGroupBox->move(10, GroupBoxTop, 320, fontHeight * 3);
		this->panel_Name->move(10, fontHeight, 150, fontHeight);
		this->edit_PanelName->move(150, fontHeight, 90, fontHeight);
		this->Label_Hotspot_Count_Name->move(10, fontHeight * 2, 150,
				fontHeight);
		this->Label_HotspotCount->move(150, fontHeight * 2, 90, fontHeight);

		const int SettingTypeTop = GroupBoxTop + fontHeight * 3 + 3;
		this->settingType->move(10, SettingTypeTop, 320, fontHeight * 3);
		this->point2Set->move(10, fontHeight, 140, fontHeight);
		this->point4Set->move(170, fontHeight, 140, fontHeight);
		this->copySelect->move(10, fontHeight * 2, 140, fontHeight);
		this->moveSelect->move(170, fontHeight * 2, 140, fontHeight);

		const int ButtonTop = SettingTypeTop + fontHeight * 3 + 3;
		this->button_Delete->move(10, ButtonTop, 320, fontHeight);
		this->button_SaveHotspot->move(10, ButtonTop + fontHeight, 320,
				fontHeight);

		const int InfoTop = ButtonTop + fontHeight * 3 + 3;
		this->label_Title->move(30, InfoTop, 300, fontHeight);
		this->label_Info->move(40, InfoTop + fontHeight, 300, fontHeight * 5);
	}
}

/**
 * フォントを設定する。
 * @param hFont フォント
 */
void ControlPanelPanelSetting::setFont(HFONT hFont) {
	this->panelSettingGroupBox->setFont(hFont);
	this->panel_Name->setFont(hFont);
	this->edit_PanelName->setFont(hFont);
	this->Label_Hotspot_Count_Name->setFont(hFont);
	this->Label_HotspotCount->setFont(hFont);
	this->button_Delete->setFont(hFont);
	this->button_SaveHotspot->setFont(hFont);
	this->settingType->setFont(hFont);
	this->point2Set->setFont(hFont);
	this->point4Set->setFont(hFont);
	this->copySelect->setFont(hFont);
	this->moveSelect->setFont(hFont);
	this->label_Title->setFont(hFont);
	this->label_Info->setFont(hFont);
}

/**
 * 全てのコントロールの表示
 */
void ControlPanelPanelSetting::allShow() {
	this->panelSettingGroupBox->show();
	this->panel_Name->show();
	this->edit_PanelName->show();
	this->Label_Hotspot_Count_Name->show();
	this->Label_HotspotCount->show();
	this->button_Delete->show();
	this->button_SaveHotspot->show();
	this->settingType->show();
	this->point2Set->show();
	this->point4Set->show();
	this->copySelect->show();
	this->moveSelect->show();
	this->label_Title->show();
	this->label_Info->show();

	SetFocus(MainWindow::getInstance()->getHandle());
}

/**
 * 全てのコントロールの非表示
 */
void ControlPanelPanelSetting::allHide() {
	this->panelSettingGroupBox->hide();
	this->panel_Name->hide();
	this->edit_PanelName->hide();
	this->Label_Hotspot_Count_Name->hide();
	this->Label_HotspotCount->hide();
	this->button_Delete->hide();
	this->button_SaveHotspot->hide();
	this->settingType->hide();
	this->point2Set->hide();
	this->point4Set->hide();
	this->copySelect->hide();
	this->moveSelect->hide();
	this->label_Title->hide();
	this->label_Info->hide();
}

/**
 * ボタンの有効化
 */
void ControlPanelPanelSetting::enableButton() {
	this->button_Delete->enable();
	this->button_SaveHotspot->enable();
	this->edit_PanelName->enable();
	this->point2Set->enable();
	this->point4Set->enable();
	this->copySelect->enable();
	this->moveSelect->enable();
	this->m_Enabled = true;
}

/**
 * ボタンの無効化
 */
void ControlPanelPanelSetting::disableButton() {
	this->edit_PanelName->disable();
	this->m_Enabled = false;
}

/**
 * コマンド発行時の処理を行う。
 * @param uMsg メッセージID(WM_COMMAND)
 * @param wParam 1つ目のパラメータ
 * @param hwndControl コマンドが発生したコントロールのハンドル
 */
LRESULT ControlPanelPanelSetting::onCommand(UINT uMsg, WPARAM wParam,
		HWND hwndControl) {
	switch (HIWORD(wParam)) {
	case BN_CLICKED:
		if (hwndControl == *this->button_Delete) {
			SendMessage(parent->getHandle(), WM_KEYDOWN, (WPARAM) VK_DELETE, 0);
		} else if (hwndControl == *this->button_SaveHotspot) {
			parent->saveHotspotNumInfo();
		} else if (hwndControl == *point2Set) {
			parent->panelSettingData->setMode(
					PanelSettingData::PANEL_SETTING_MODE_2SET);
		} else if (hwndControl == *point4Set) {
			parent->panelSettingData->setMode(
					PanelSettingData::PANEL_SETTING_MODE_4SET);
		} else if (hwndControl == *copySelect) {
			parent->panelSettingData->setMode(
					PanelSettingData::PANEL_SETTING_MODE_COPY);
		} else if (hwndControl == *moveSelect) {
			parent->panelSettingData->setMode(
					PanelSettingData::PANEL_SETTING_MODE_MOVE);
		} else {
			break;
		}

		parent->panelSettingData->clearPointCount();
		parent->panelSettingData->ClearCopyIdSize();
		parent->canvasUpdate();
		SetFocus(parent->getHandle());
		break;

	case EN_CHANGE: {
		int length = 0;
		length = this->edit_PanelName->getTextLength();
		// TODO 関数化
		int sel = LOWORD(Edit_GetSel(this->edit_PanelName->getHandle()));
		TCHAR text[length + 1];
		this->edit_PanelName->getText(text, length + 1);
		TCHAR space = TEXT(' ');
		TCHAR zenSpace = TEXT('　');
		bool changed = false;
		for (int i = 0; i < length; i++) {
			if (text[i] == space) {
				text[i] = zenSpace;
				changed = true;
			}
		}
		if (changed == true) {
			this->edit_PanelName->setText(text);
			Edit_SetSel(this->edit_PanelName->getHandle(), sel, sel);
		}
	}
		break;

	case EN_KILLFOCUS:
		if (hwndControl == *this->edit_PanelName) {
			int id = panelData->getSelectedPanelId();
			if (id != -1) {
				int bufferLength = this->edit_PanelName->getTextLength() + 1;
				TCHAR *buffer = new TCHAR[bufferLength];
				ZeroMemory(buffer, bufferLength * sizeof(TCHAR));
				this->edit_PanelName->getText(buffer, bufferLength);
				if (lstrlen(buffer) > 0) {
					panelData->setPanelName(id, buffer);
				} else {
					parent->showMessageBox(
					IDS_PANELSETTING_ERROR,
					IDS_ERROR,
					MB_OK | MB_ICONEXCLAMATION);
					this->setFocus();
				}
				delete buffer;
			}
		} else {
			break;
		}
		parent->canvasUpdate();
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
LRESULT ControlPanelPanelSetting::onKey(UINT uMsg, WPARAM wParam) {
	int id = panelData->getSelectedPanelId();
	const int movePixel = 1;
	double move = movePixel / parent->getViewPixelPerMeter();
	int mode = parent->panelSettingData->getMode();
	int size = parent->panelSettingData->getCopyIdSize();

	switch (uMsg) {
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_UP:
			if (this->m_Enabled && (id != -1)) {
				panelData->movePanel(id, 0, move);
			}
			break;

		case VK_DOWN:
			if (this->m_Enabled && (id != -1)) {
				panelData->movePanel(id, 0, -move);
			}
			break;

		case VK_LEFT:
			if (this->m_Enabled && (id != -1)) {
				panelData->movePanel(id, -move, 0);
			}
			break;

		case VK_RIGHT:
			if (this->m_Enabled && (id != -1)) {
				panelData->movePanel(id, move, 0);
			}
			break;

		case VK_CONTROL:
			if ((mode == PanelSettingData::PANEL_SETTING_MODE_COPY)
					&& (0 < size)) {
				// カーソル位置がキャンバス内かどうか判定処理
				RECT rect;
				parent->getWindowRect(&rect);
				if (parent->mouseCursorLocationCheck(&rect)
						&& parent->panelPosCheck()) {
					parent->PanelSettingPanelCopy();
					parent->panelSettingData->ClearCopyIdSize();
				} else {
					// 処理なし
				}
			}
			break;

		case VK_DELETE:
			if (this->m_Enabled && (id != -1)) {
				// 一つ選択中の場合
				this->disableButton();
				panelData->panelSettingDelete(id);
			}
			// 複数選択中
			parent->PanelSettingPanelDelete();
			panelData->setSelectedPanelId(-1);
			break;

		default:
			break;
		}
		parent->canvasUpdate();
		break;

	default:
		break;
	}
	return 0;
}

void ControlPanelPanelSetting::setEditName(LPTSTR data) {
	this->edit_PanelName->setText(data);
}

void ControlPanelPanelSetting::setFocus(void) {
	this->edit_PanelName->setFocus();
}

void ControlPanelPanelSetting::setHotspotCount(LPTSTR data) {
	this->Label_HotspotCount->setText(data);
}
