/*
 * ControlPanelDefault.h
 *
 *  Created on: 2016/01/12
 *      Author: PC-EFFECT-011
 */

#ifndef CONTROLPANELDEFAULT_H_
#define CONTROLPANELDEFAULT_H_

#include <windows.h>

#include "ControlPanelBase.h"

class ControlPanelDefault: public ControlPanelBase {
public:
	ControlPanelDefault(GroupBox *pWnd);
	~ControlPanelDefault(void);

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
	 * 表示モードの更新
	 */
	void updateMode();

	/**
	 * 表示画像種別の取得
	 */
	int getPictureType();

	// (2017/4/4YM)赤外線画像タイプ取得関数追加
	/**
	 * 赤外線画像種別の取得
	 */
	int getInfraredType();

	// (2017/4/4YM)赤外線画像タイプセット関数追加
	/**
	 * 赤外線画像種別のセット
	 */
	void setInfraredType(int Type);

	/**
	 * 画面右の開始日付を更新
	 */
	void setInfoFirstDate(LPTSTR firstDate);

	/**
	 * 画面右の終了日付を更新
	 */
//	void setInfoLastDate(LPTSTR lastDate);		// (2017/6/1YM)撮影終了時刻を削除
// (2017/6/1YM)閾値温度を更新
	void setInfoThresholdTemp(LPTSTR threshold);

	/**
	 * 画面右の枚数表示を更新
	 */
	void setInfoDataCount(LPTSTR dataCount);

	/**
	 * 画面右のホットスポット総数表示を更新
	 */
	void setInfoHotSpotCount(LPTSTR dataCount);

	/**
	 * 画面右の最高温度表示を更新
	 */
	void setInfoTempMax(LPTSTR tempMax);

	/**
	 * 画面右の平均温度表示を更新
	 */
	void setInfoTempAve(LPTSTR tempAve);

	/**
	 * コマンド発行時の処理を行う。
	 * @param uMsg メッセージID(WM_COMMAND)
	 * @param wParam 1つ目のパラメータ
	 * @param hwndControl コマンドが発生したコントロールのハンドル
	 */
	LRESULT onCommand(UINT uMsg, WPARAM wParam, HWND hwndControl);

	// (2017/2/16YM)キー入力処理を追加
	LRESULT onKey(UINT uMsg, WPARAM wParam);

	int checkXTmode;
	int bt;
private:
	//--------------------------------------------------------
	// 操作パネル
	//--------------------------------------------------------
	/** フォルダ選択ボタン */
	PushButton *button_SetFolder;

	/** 選択データ内容グループボックス */
	GroupBox *group_PositionInfo;
	/** 文字列「監視日時(始)」表示ラベル */
	Label *label_StartDateDesc;
	/** 文字列「監視日時(終)」表示ラベル */
	//Label* label_EndDateDesc;
	/** 監視日時(始)表示ラベル */
	Label *label_StartDate;
	/** 開始日時(終)表示ラベル */
	//Label* label_EndDate;
	/** 文字列「撮影枚数」表示ラベル */
	Label *label_PictureCountDesc;
	/** 撮影枚数表示ラベル */
	Label *label_PictureCount;
	/** 文字列「ホットスポット総数」表示ラベル */
	Label *label_HotSpotCountDesc;
	/** ホットスポット総数表示ラベル */
	Label *label_HotSpotCount;
	/** 文字列「最高温度」表示ラベル */
	Label *label_TempMaxDesc;
	/** 最高温度表示ラベル */
	Label *label_TempMax;
	/** 文字列「平均温度」表示ラベル */
	Label *label_TempAveDesc;
	/** 平均温度表示ラベル */
	Label *label_TempAve;
	// (2017/6/1YM)閾値温度の項目を追加
	Label *label_ThresholdTemp;
	Label *label_ThresholdTempDesc;

	/** 表示画像選択グループボックス */
	GroupBox *group_PictureType;
	/** 可視画像表示ラジオボタン */
	RadioButton *radio_ShowVisible;
	/** 熱画像表示ラジオボタン */
	RadioButton *radio_ShowInfrared;
	// (2017/5/31YM)輪郭表示ラジオボタンを追加
	/** 輪郭画像表示ラジオボタン */
	RadioButton *radio_ShowPanel;

	// (2017/4/4YM)画像位置初期化ボタンを追加
	/** 画像位置初期化ボタン */
	PushButton *button_InitPicture;

	// (2017/4/4YM)赤外線画像タイプ選択ラジオを追加
	/** 赤外線画像選択グループボックス */
	GroupBox *group_InfraredType;
	/** 白黒ラジオボタン */
	RadioButton *radio_ShowMono;
	/** カラーラジオボタン */
	RadioButton *radio_ShowColor;
	// (2017/4/4YM)↑ここまで追加

};

#endif /* CONTROLPANELDEFAULT_H_ */
