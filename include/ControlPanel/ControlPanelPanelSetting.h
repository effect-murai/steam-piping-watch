#ifndef CONTROLPANELPANELSETTING_H_
#define CONTROLPANELPANELSETTING_H_

#include <windows.h>

#include "ControlPanelBase.h"

class ControlPanelPanelSetting: public ControlPanelBase {
public:
	ControlPanelPanelSetting(GroupBox *pWnd);
	~ControlPanelPanelSetting();

	/**
	 * 初期設定
	 */
	void init();

	/**
	 * フォントを設定する。
	 * @param hFont フォント
	 */
	void setFont(HFONT hFont);

	/**
	 * 全てのコントロールの表示
	 */
	void allShow();

	/**
	 * 全てのコントロールの非表示
	 */
	void allHide();

	/**
	 * ボタンの有効化
	 */
	void enableButton();

	/**
	 * ボタンの無効化
	 */
	void disableButton();

	/**
	 * パネル名の設定
	 */
	void setEditName(LPTSTR data);

	/**
	 * パネル内ホットスポット数の設定
	 */
	void setHotspotCount(LPTSTR data);

	/**
	 * フォーカスの設定
	 */
	void setFocus();

	/**
	 * コマンド発行時の処理を行う。
	 * @param uMsg メッセージID(WM_COMMAND)
	 * @param wParam 1つ目のパラメータ
	 * @param hwndControl コマンドが発生したコントロールのハンドル
	 */
	LRESULT onCommand(UINT uMsg, WPARAM wParam, HWND hwndControl);

	/**
	 * キー入力時の処理を行う。
	 * @param uMsg メッセージID
	 * @param wParam 1つ目のパラメータ
	 */
	LRESULT onKey(UINT uMsg, WPARAM wParam);

private:
	//--------------------------------------------------------
	// 操作パネル
	//--------------------------------------------------------
	/** パネル情報*/
	GroupBox *panelSettingGroupBox;
	/** パネル名ラベル*/
	Label *panel_Name;
	/** パネル名入力フォーム*/
	EditBox *edit_PanelName;
	/** ホットスポット数名ラベル*/
	Label *Label_Hotspot_Count_Name;
	/** ホットスポット数ラベル*/
	Label *Label_HotspotCount;

	/** Panel削除ボタン */
	PushButton *button_Delete;
	/** ホットスポット数一覧出力ボタン */
	PushButton *button_SaveHotspot;
	/** settingモード*/
	GroupBox *settingType;
	/** 2点指定モード*/
	RadioButton *point2Set;
	/** 4点指定モード*/
	RadioButton *point4Set;
	/** 範囲指定コピーモード*/
	RadioButton *copySelect;
	/** 選択カーソル移動モード*/
	RadioButton *moveSelect;

	/** タイトルラベル */
	Label *label_Title;
	/** 説明ラベル */
	Label *label_Info;
};

#endif /* CONTROLPANELPANELSETTING_H_ */
