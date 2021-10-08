/*
 * ControlPanelHotspot.cpp
 *
 *  Created on: 2016/01/12
 *      Author: PC-EFFECT-011
 */

#include "ControlPanelHotspot.h"
#include "MainWindow.h"
#include "resource.h"

ControlPanelHotspot::ControlPanelHotspot(GroupBox *pWnd) {
	this->m_pWnd = pWnd;
	this->m_Enabled = true;

	this->button_Prev = new PushButton(
			Resource::getString(IDS_PANEL_PICTURE_PREV), WS_VISIBLE | BS_FLAT,
			20, 130, 140, 36, this->m_pWnd);
	this->button_Next = new PushButton(
			Resource::getString(IDS_PANEL_PICTURE_NEXT), WS_VISIBLE | BS_FLAT,
			180, 130, 140, 36, this->m_pWnd);

	this->label_Title = new Label(Resource::getString(IDS_PANEL_INFO_TITLE),
	WS_VISIBLE, 30, 200, 300, 24, this->m_pWnd);	// (2017/6/14YM)座標変更
	this->label_Info = new Label(Resource::getString(IDS_PANEL_INFO_HOTSPOT),
	WS_VISIBLE, 40, 230, 300, 260, this->m_pWnd);	// (2017/6/14YM)座標変更
	//(2019/12/20LEE) ホットスポット補正の部分を追加
	this->label_Info2 = new Label(Resource::getString(IDS_PANEL_INFO_HOTSPOT2),
	WS_VISIBLE, 40, 450, 250, 50, this->m_pWnd);

	// (2017/6/14YM)ホットスポット削除ボタンを追加
	this->button_HSDel = new PushButton(
			Resource::getString(IDS_NAVI_HOTSPOTDEL), WS_VISIBLE | BS_FLAT, 20,
			550, 140, 36, this->m_pWnd);
	// (2017/6/14YM)特徴点削除ボタンを追加
	this->button_KPDel = new PushButton(
			Resource::getString(IDS_NAVI_KEYPOINTDEL), WS_VISIBLE | BS_FLAT,
			180, 550, 140, 36, this->m_pWnd);
}

ControlPanelHotspot::~ControlPanelHotspot() {
	delete this->button_Prev;
	delete this->button_Next;

	delete this->label_Title;
	delete this->label_Info;
	delete this->label_Info2;

	// (2017/6/14YM)ホットスポット削除ボタンを追加
	delete this->button_HSDel;
	// (2017/6/14YM)特徴点削除ボタンを追加
	delete this->button_KPDel;
}

/**
 * 初期設定
 */
void ControlPanelHotspot::init() {
	/**
	 * 高さが940px未満の場合、MiniPanel用の配置に変更
	 */
	if (MainWindow::getInstance()->IsMiniPanelMode()) {
		// 操作パネルのコントロールを移動
		const int fontHeight = 21;
		const int Height = 20;
		HFONT minilabelfont;
		minilabelfont = CreateFont(-12, 0, 0, 0, FW_REGULAR, FALSE, FALSE,
		FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH, NULL);

		const int PictureTop = 110;
		this->button_Prev->move(20, PictureTop, 140, 40); //(2020/01/06LEE) buttonのsizeも変更
		this->button_Next->move(180, PictureTop, 140, 40); //(2020/01/06LEE) buttonのsizeも変更

		const int InfoTop = PictureTop + Height + 20;
		this->label_Title->move(30, InfoTop);
		this->label_Info->setFont(minilabelfont);
		this->label_Info->move(40, InfoTop + fontHeight, 300, 160);
		this->label_Info2->setFont(minilabelfont);
		this->label_Info2->move(40, InfoTop + fontHeight + 160, 250, 50);

		// (2017/6/23YM)ホットスポット削除ボタンを追加
		this->button_HSDel->move(20, InfoTop + fontHeight + 205, 140, 40); // (2017/8/9YM)175⇒185へ、(2020/01/06LEE) 185⇒205、またbuttonのsizeも変更
		// (2017/6/23YM)特徴点削除ボタンを追加
		this->button_KPDel->move(180, InfoTop + fontHeight + 205, 140, 40);	// (2017/8/9YM)175⇒185へ、(2020/01/06LEE) 185⇒205、またbuttonのsizeも変更
	}
}

/**
 * フォントを設定する。
 * @param hFont フォント
 */
void ControlPanelHotspot::setFont(HFONT hFont) {
	this->button_Prev->setFont(hFont);
	this->button_Next->setFont(hFont);

	this->label_Title->setFont(hFont);
	this->label_Info->setFont(hFont);
	// (2019/12/20LEE) ホットスポット補正の部分を追加
	this->label_Info2->setFont(hFont);

	// (2017/6/14YM)ホットスポット削除ボタンを追加
	this->button_HSDel->setFont(hFont);
	// (2017/6/14YM)特徴点削除ボタンを追加
	this->button_KPDel->setFont(hFont);
}

/**
 * 全てのコントロールの表示
 */
void ControlPanelHotspot::allShow() {
	this->button_Prev->show();
	this->button_Next->show();

	this->label_Title->show();
	this->label_Info->show();
	this->label_Info2->show();

	// (2017/6/14YM)ホットスポット削除ボタンを追加
	this->button_HSDel->show();
	// (2017/6/14YM)特徴点削除ボタンを追加
	this->button_KPDel->show();

	SetFocus(MainWindow::getInstance()->getHandle());
}

/**
 * 全てのコントロールの非表示
 */
void ControlPanelHotspot::allHide() {
	this->button_Prev->hide();
	this->button_Next->hide();

	this->label_Title->hide();
	this->label_Info->hide();
	// (2019/12/20LEE) ホットスポット補正の部分を追加
	this->label_Info2->hide();
	// (2017/6/14YM)ホットスポット削除ボタンを追加
	this->button_HSDel->hide();
	// (2017/6/14YM)特徴点削除ボタンを追加
	this->button_KPDel->hide();
}

/**
 * ボタンの有効化
 */
void ControlPanelHotspot::enableButton() {
	this->button_Prev->enable();
	this->button_Next->enable();

	this->m_Enabled = true;

	// (2017/6/14YM)ホットスポット削除ボタンを追加
	this->button_HSDel->enable();
	// (2017/6/14YM)特徴点削除ボタンを追加
	this->button_KPDel->enable();
}

/**
 * ボタンの無効化
 */
void ControlPanelHotspot::disableButton() {
	this->button_Prev->disable();
	this->button_Next->disable();

	this->m_Enabled = false;

	// (2017/6/14YM)ホットスポット削除ボタンを追加
	this->button_HSDel->disable();
	// (2017/6/14YM)特徴点削除ボタンを追加
	this->button_KPDel->disable();
}

/**
 * コマンド発行時の処理を行う。
 * @param uMsg メッセージID(WM_COMMAND)
 * @param wParam 1つ目のパラメータ
 * @param hwndControl コマンドが発生したコントロールのハンドル
 */
LRESULT ControlPanelHotspot::onCommand(UINT uMsg, WPARAM wParam,
		HWND hwndControl) {

	switch (HIWORD(wParam)) {
	case BN_CLICKED:
		if (hwndControl == *this->button_Prev) {
			MainWindow::getInstance()->canvasUpdatePrev();
		} else if (hwndControl == *this->button_Next) {
			MainWindow::getInstance()->canvasUpdateNext();
		} else if (hwndControl == *this->button_HSDel) // (2017/6/14YM)ホットスポット削除ボタンを追加
				{
			// (2017/6/20YM)処理を関数に変更
			MainWindow::getInstance()->removeAllHotspots();
		} else if (hwndControl == *this->button_KPDel) // (2017/6/14YM)特徴点削除ボタンを追加
				{
			// (2017/6/20YM)処理を関数に変更
			MainWindow::getInstance()->removeAllKeypoints();
		} else {
			break;
		}
		SetFocus(MainWindow::getInstance()->getHandle());
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
LRESULT ControlPanelHotspot::onKey(UINT uMsg, WPARAM wParam) {
	if (this->m_Enabled != true) {
		return 0;
	}
	switch (uMsg) {
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_LEFT:
			MainWindow::getInstance()->canvasUpdatePrev();
			break;

		case VK_RIGHT:
			MainWindow::getInstance()->canvasUpdateNext();
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
