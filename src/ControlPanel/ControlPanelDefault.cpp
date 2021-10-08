/*
 * ControlPanelDefault.cpp
 *
 *  Created on: 2016/01/12
 *      Author: PC-EFFECT-011
 */

#define PANELSHOWENABLE 			// (2017/7/24YM)パネル輪郭表示ラジオ有効化フラグ

#include "ControlPanelDefault.h"
#include "MainWindow.h"
#include "resource.h"
#include "resultData.h"

// (2017/6/2YM)インクルード文追加
#include "app.h"
#include <stdio.h>

// (2017/2/16YM)外部参照変数宣言
extern MainWindow *mainWindow;
extern ResultData *result;

ControlPanelDefault::ControlPanelDefault(GroupBox *pWnd) {
	this->m_pWnd = pWnd;
	this->bt = 0;
	this->checkXTmode = 0;

	button_SetFolder = new PushButton(Resource::getString(IDS_SEL_FOLDER),
	WS_VISIBLE | BS_FLAT, 10, 150, 320, 36, this->m_pWnd);

	group_PositionInfo = new GroupBox(Resource::getString(IDS_DATA_CONTENT),
	WS_VISIBLE, 10, 200, 320, 210, this->m_pWnd);
	label_StartDateDesc = new Label(Resource::getString(IDS_START_DATE),
	WS_VISIBLE, 10, 30, 120, 24, group_PositionInfo);
	label_StartDate = new Label(Resource::getString(IDS_NULL), WS_VISIBLE, 130,
			30, 180, 24, group_PositionInfo);
	label_PictureCountDesc = new Label(
			// (2017/6/1YM)撮影枚数の表示位置変更
			Resource::getString(IDS_PICTURE_COUNT), WS_VISIBLE, 10, 60, 100, 24,
			group_PositionInfo);
	label_PictureCount = new Label(
			// (2017/6/1YM)撮影枚数の表示位置変更
			Resource::getString(IDS_NULL), WS_VISIBLE, 130, 60, 100, 24,
			group_PositionInfo);
	label_HotSpotCountDesc = new Label(
			Resource::getString(IDS_HOTSPOT_SUM_COUNT), WS_VISIBLE, 10, 120,
			120, 24, group_PositionInfo);
	label_HotSpotCount = new Label(Resource::getString(IDS_NULL), WS_VISIBLE,
			130, 120, 120, 24, group_PositionInfo);
	label_TempMaxDesc = new Label(Resource::getString(IDS_TEMP_MAX), WS_VISIBLE,
			10, 150, 120, 24, group_PositionInfo);

	label_TempMax = new Label(Resource::getString(IDS_NULL), WS_VISIBLE, 130,
			150, 180, 24, group_PositionInfo);
	label_TempAveDesc = new Label(Resource::getString(IDS_TEMP_AVE), WS_VISIBLE,
			10, 180, 120, 24, group_PositionInfo);

	label_TempAve = new Label(Resource::getString(IDS_NULL), WS_VISIBLE, 130,
			180, 180, 24, group_PositionInfo);

	group_PictureType = new GroupBox(Resource::getString(IDS_VIEW_PICTURE_TYPE),
	WS_VISIBLE | BS_NOTIFY, 10, 430, 320, 60, this->m_pWnd);

	// (2017/7/24YM)パネル輪郭表示ラジオ有効の時
#ifdef PANELSHOWENABLE
	radio_ShowVisible = new RadioButton(
			Resource::getString(IDS_VISIBLE_PICTURE),
			WS_VISIBLE | BS_NOTIFY | WS_GROUP, 10, 30, 100, 24,
			group_PictureType);				// (2017/5/31YM)ボタン表示長さを変更
	radio_ShowInfrared = new RadioButton(
			Resource::getString(IDS_INFRARED_PICTURE), WS_VISIBLE | BS_NOTIFY,
			120, 30, 100, 24, group_PictureType);	// (2017/5/31YM)ボタン表示位置を変更
	// (2017/5/31YM)輪郭画像選択ラジオボタンを追加
	radio_ShowPanel = new RadioButton(Resource::getString(IDS_PANEL_PICTURE),
	WS_VISIBLE | BS_NOTIFY, 230, 30, 80, 24, group_PictureType);
#else
	// (2017/7/24YM)パネル輪郭表示ラジオ無効の時
	radio_ShowVisible = new RadioButton(
			Resource::getString(IDS_VISIBLE_PICTURE), WS_VISIBLE | BS_NOTIFY | WS_GROUP,
			10, 30, 140, 24, group_PictureType);
	radio_ShowInfrared = new RadioButton(
			Resource::getString(IDS_INFRARED_PICTURE), WS_VISIBLE | BS_NOTIFY,
			170, 30, 140, 24, group_PictureType);
	radio_ShowPanel = new RadioButton(
			Resource::getString(IDS_PANEL_PICTURE), WS_VISIBLE | BS_NOTIFY,
			230, 30, 0, 0, group_PictureType);
#endif

	// (2017/4/4YM)画像位置初期化ボタンを追加
	button_InitPicture = new PushButton(Resource::getString(IDS_INIT_PICTURE),
	WS_VISIBLE | BS_FLAT, 10, 570, 320, 36, this->m_pWnd);

	// (2017/4/4YM)赤外線画像タイプ選択ラジオを追加
	group_InfraredType = new GroupBox(
			Resource::getString(IDS_INFRARED_PICTURE_TYPE),
			WS_VISIBLE | BS_NOTIFY, 10, 500, 320, 60, this->m_pWnd);
	radio_ShowMono = new RadioButton(Resource::getString(IDS_INFRARED_MONO),
	WS_VISIBLE | BS_NOTIFY | WS_GROUP, 10, 30, 140, 24, group_InfraredType);
	radio_ShowColor = new RadioButton(Resource::getString(IDS_INFRARED_COLOR),
	WS_VISIBLE | BS_NOTIFY, 170, 30, 140, 24, group_InfraredType);
	// (2017/4/4YM)↑ここまで追加
	// (2017/6/1YM)閾値温度を追加
	label_ThresholdTemp = new Label(Resource::getString(IDS_NULL), WS_VISIBLE,
			130, 90, 180, 24, group_PositionInfo);
	label_ThresholdTempDesc = new Label(Resource::getString(IDS_THRESHOLD_TEMP),
	WS_VISIBLE, 10, 90, 120, 24, group_PositionInfo);

}

ControlPanelDefault::~ControlPanelDefault(void) {
	delete button_SetFolder;

	delete label_StartDateDesc;
	delete label_StartDate;
	delete label_PictureCountDesc;
	delete label_PictureCount;
	delete label_HotSpotCountDesc;
	delete label_HotSpotCount;
	delete label_TempMaxDesc;
	delete label_TempMax;
	delete label_TempAveDesc;
	delete label_TempAve;

	// (2017/6/1YM)閾値温度項目を追加
	delete label_ThresholdTemp;
	delete label_ThresholdTempDesc;

	delete group_PositionInfo;

	delete radio_ShowVisible;
	delete radio_ShowInfrared;
	// (2017/5/31YM)輪郭画像選択ボタンを追加
	delete radio_ShowPanel;
	delete group_PictureType;

	// (2017/4/4YM)画像位置初期化ボタンを追加
	delete button_InitPicture;

	// (2017/4/4YM)赤外線画像タイプ選択ラジオを追加
	delete radio_ShowMono;
	delete radio_ShowColor;
	delete group_InfraredType;

}

/**
 * 初期設定
 */
void ControlPanelDefault::init() {
	radio_ShowInfrared->setCheck();
	// (2017/4/4YM)赤外線白黒のラジオボタンにチェックを入れる
	radio_ShowMono->setCheck();

	if (result != NULL && result->isPanelDetected()) {
		this->label_TempMaxDesc->setText(Resource::getString(IDS_TEMP_MAX));
		this->label_TempAveDesc->setText(Resource::getString(IDS_TEMP_AVE));
	} else {
		this->label_TempMaxDesc->setText(Resource::getString(IDS_TEMP_MAX2));
		this->label_TempAveDesc->setText(Resource::getString(IDS_TEMP_AVE2));
	}

	this->label_TempMax->show();
	this->label_TempAve->show();

	/**
	 * 高さが940px未満の場合、MiniPanel用の配置に変更
	 */
	if (MainWindow::getInstance()->IsMiniPanelMode()) {
		// 操作パネルのコントロールを移動
		const int fontHeight = 21;
		const int group_Button_SelFolderTop = 120;
		button_SetFolder->move(10, group_Button_SelFolderTop, 320, fontHeight);

		const int group_PictureInfoTop = group_Button_SelFolderTop + fontHeight
				+ 6;
		group_PositionInfo->move(10, group_PictureInfoTop, 320,
				fontHeight * 7 + 6);

		label_StartDateDesc->move(10, fontHeight);
		label_StartDate->move(130, fontHeight);

		label_PictureCountDesc->move(10, fontHeight * 2);
		label_PictureCount->move(130, fontHeight * 2);

		label_HotSpotCountDesc->move(10, fontHeight * 4);
		label_HotSpotCount->move(130, fontHeight * 4);

		label_TempMaxDesc->move(10, fontHeight * 5);
		label_TempMax->move(130, fontHeight * 5);

		label_TempAveDesc->move(10, fontHeight * 6);
		label_TempAve->move(130, fontHeight * 6);

		// (2017/6//1YM)閾値温度を追加
		label_ThresholdTemp->move(130, fontHeight * 3);
		label_ThresholdTempDesc->move(10, fontHeight * 3);

		const int group_PictureTypeTop = group_PictureInfoTop + fontHeight * 7
				+ 6 + 2;
		group_PictureType->move(10, group_PictureTypeTop, 320,
				fontHeight * 2 + 6);
#ifdef PANELSHOWENABLE
		radio_ShowVisible->move(10, fontHeight);
		radio_ShowInfrared->move(120, fontHeight);		// (2017/5/31YM)表示位置を変更
		// (2017/5/31YM)輪郭画像選択ラジオボタンを追加
		radio_ShowPanel->move(230, fontHeight);
#else	// (2017/8/3YM)追加
		radio_ShowVisible->move(10, fontHeight);
		radio_ShowInfrared->move(170, fontHeight);
		// (2017/5/31YM)輪郭画像選択ラジオボタンを追加
		radio_ShowPanel->move(290, fontHeight);
#endif

		// (2017/4/4YM)画像位置初期化ボタンを追加
		const int group_Button_InitPicture = group_PictureInfoTop
				+ fontHeight * 11 + 6 + 6 + 10;
		// (2017/8/9YM)ボタンの幅を調整
		button_InitPicture->move(10, group_Button_InitPicture, 320, fontHeight);

		// (2017/4/4YM)赤外線画像タイプ選択ラジオを追加
		const int group_InfraredTypeTop = group_PictureInfoTop + fontHeight * 9
				+ 6 + 6 + 2;
		group_InfraredType->move(10, group_InfraredTypeTop, 320,
				fontHeight * 2 + 6);
		radio_ShowMono->move(10, fontHeight);
		radio_ShowColor->move(170, fontHeight);
	}
}

/**
 * フォントを設定する
 * @param[in] hFont フォントのハンドル
 */
void ControlPanelDefault::setFont(HFONT hFont) {
	button_SetFolder->setFont(hFont);

	group_PositionInfo->setFont(hFont);
	label_StartDateDesc->setFont(hFont);
	label_PictureCountDesc->setFont(hFont);
	label_HotSpotCountDesc->setFont(hFont);
	label_TempMaxDesc->setFont(hFont);
	label_TempAveDesc->setFont(hFont);

	label_StartDate->setFont(hFont);
	label_PictureCount->setFont(hFont);
	label_HotSpotCount->setFont(hFont);
	label_TempMax->setFont(hFont);
	label_TempAve->setFont(hFont);
	// (2017/6/1YM)閾値温度を追加
	label_ThresholdTemp->setFont(hFont);
	label_ThresholdTempDesc->setFont(hFont);

	group_PictureType->setFont(hFont);
	radio_ShowVisible->setFont(hFont);
	radio_ShowInfrared->setFont(hFont);
	// (2017/5/31YM)輪郭画像タイプ選択ラジオを追加
	radio_ShowPanel->setFont(hFont);

	// (2017/4/4YM)画像位置初期化ボタンを追加
	button_InitPicture->setFont(hFont);

	// (2017/4/4YM)赤外線画像タイプ選択ラジオを追加
	group_InfraredType->setFont(hFont);
	radio_ShowMono->setFont(hFont);
	radio_ShowColor->setFont(hFont);

}

/**
 * 全てのコントロールの表示
 */
void ControlPanelDefault::allShow() {

	this->button_SetFolder->show();
	this->group_PositionInfo->show();
	this->label_StartDateDesc->show();
	this->label_StartDate->show();
	this->label_PictureCountDesc->show();
	this->label_PictureCount->show();
	this->label_HotSpotCountDesc->show();
	this->label_HotSpotCount->show();

	if (result != NULL && result->isPanelDetected()) {
		this->label_TempMaxDesc->setText(Resource::getString(IDS_TEMP_MAX));
		this->label_TempAveDesc->setText(Resource::getString(IDS_TEMP_AVE));
	} else {
		this->label_TempMaxDesc->setText(Resource::getString(IDS_TEMP_MAX2));
		this->label_TempAveDesc->setText(Resource::getString(IDS_TEMP_AVE2));
	}
	this->label_TempMaxDesc->show();
	this->label_TempAveDesc->show();
	this->label_TempMax->show();
	this->label_TempAve->show();

	// (2017/6/1YM)閾値温度を追加
	this->label_ThresholdTemp->show();
	this->label_ThresholdTempDesc->show();

	this->group_PictureType->show();

	// (2019/12/13LEE) 
	this->radio_ShowVisible->show();

	updateMode();

	this->radio_ShowInfrared->show();
	// (2017/5/31YM)輪郭画像タイプ選択ラジオを追加
	this->radio_ShowPanel->show();

	// (2017/2/16YM)キー入力処理有効化のために追加
	SetFocus(MainWindow::getInstance()->getHandle());

	// (2017/4/4YM)画像位置初期化ボタンを追加
	this->button_InitPicture->show();

	// (2017/4/4YM)赤外線画像タイプ選択ラジオを追加
	this->group_InfraredType->show();
	this->radio_ShowMono->show();
	this->radio_ShowColor->show();
}

/**
 * 全てのコントロールの非表示
 */
void ControlPanelDefault::allHide() {
	this->button_SetFolder->hide();

	this->group_PositionInfo->hide();
	this->label_StartDateDesc->hide();
	this->label_StartDate->hide();
	this->label_PictureCountDesc->hide();
	this->label_PictureCount->hide();
	this->label_HotSpotCountDesc->hide();
	this->label_HotSpotCount->hide();
	this->label_TempMaxDesc->hide();
	this->label_TempMax->hide();
	this->label_TempAveDesc->hide();
	this->label_TempAve->hide();
	// (2017/6/1YM)閾値温度を追加
	this->label_ThresholdTemp->hide();
	this->label_ThresholdTempDesc->hide();

	this->group_PictureType->hide();
	this->radio_ShowVisible->hide();
	this->radio_ShowInfrared->hide();
	// (2017/5/31YM)輪郭画像タイプ選択ラジオを追加
	this->radio_ShowPanel->hide();

	// (2017/4/4YM)画像位置初期化ボタンを追加
	this->button_InitPicture->hide();

	// (2017/4/4YM)赤外線画像タイプ選択ラジオを追加
	this->group_InfraredType->hide();
	this->radio_ShowMono->hide();
	this->radio_ShowColor->hide();
}

/**
 * ボタンの有効化
 */
void ControlPanelDefault::enableButton() {
}

/**
 * ボタンの無効化
 */
void ControlPanelDefault::disableButton() {
}

void ControlPanelDefault::updateMode() {
	if (result != NULL) {
		result->updatePanelDetected();
	}
	if (result != NULL && result->hasVisible()) {
		radio_ShowVisible->enable();
	} else {
		// 輪郭画像時の対応
#		ifdef PANELSHOWENABLE
		if (!radio_ShowPanel->getCheck()) {
			radio_ShowInfrared->setCheck();
		}
#		else // PANELSHOWENABLE
		radio_ShowInfrared->setCheck();
#		endif // PANELSHOWENABLE
		radio_ShowVisible->setCheck(false);
		radio_ShowVisible->disable();
	}
	if (result != NULL && result->isPanelDetected()) {
		radio_ShowPanel->enable();
		label_TempMaxDesc->setText(Resource::getString(IDS_TEMP_MAX));
		label_TempAveDesc->setText(Resource::getString(IDS_TEMP_AVE));
	} else {
		if (radio_ShowPanel->getCheck()) {
			radio_ShowInfrared->setCheck();
		}
		radio_ShowPanel->setCheck(false);
		radio_ShowPanel->disable();
		label_TempMaxDesc->setText(Resource::getString(IDS_TEMP_MAX2));
		label_TempAveDesc->setText(Resource::getString(IDS_TEMP_AVE2));
	}
}

/**
 * 表示画像種別の取得
 */
int ControlPanelDefault::getPictureType() {
	// (2017/5/31YM)選択したボタン別に値を返す
	bt = radio_ShowVisible->getCheck();
	if (bt == 1) {
		return 0;		//　可視光画像：１
	}
	bt = radio_ShowInfrared->getCheck();
	if (bt == 1) {
		return 1;		//　赤外線画像：２
	}
	//　(2017/6/2YM)輪郭データ存在チェック
	int ret;
	TCHAR *fileName;
	result->getFilePath(0, ResultData::PANEL_IMAGE, &fileName);
	wchar_t *wcFileName;
	wcFileName = (wchar_t*) fileName;
	FILE *fp;
	if ((fp = fileOpen(wcFileName, TEXT("r"))) != NULL) {
		ret = 2;		//　輪郭画像：３
	} else {
		ret = 1;		//　輪郭画像がなければ赤外線画像
	}
	fclose(fp);
	return ret;
}

// (2017/4/4YM)赤外線画像タイプ取得関数追加
/**
 * 赤外線画像種別の取得
 */
int ControlPanelDefault::getInfraredType() {
	return radio_ShowColor->getCheck();
}

// (2017/4/4YM)赤外線画像タイプセット関数追加
/**
 * 赤外線画像種別のセット
 */
void ControlPanelDefault::setInfraredType(int Type) {

	if (Type == 0) {
		// 赤外線白黒のラジオボタンにチェックを入れる
		radio_ShowMono->setCheck(true);
		radio_ShowColor->setCheck(false);
	} else {
		// (2017/4/4YM)赤外線カラーのラジオボタンにチェックを入れる
		radio_ShowColor->setCheck(true);
		radio_ShowMono->setCheck(false);
	}
}

/**
 * 画面右の開始日付を更新
 */
void ControlPanelDefault::setInfoFirstDate(LPTSTR firstDate) {
	label_StartDate->setText(firstDate);
}

// (2017/6/1YM)閾値温度を更新
void ControlPanelDefault::setInfoThresholdTemp(LPTSTR threshold) {
	label_ThresholdTemp->setText(threshold);
}
/**
 * 画面右の枚数表示を更新
 */
void ControlPanelDefault::setInfoDataCount(LPTSTR dataCount) {
	label_PictureCount->setText(dataCount);
}

/**
 * 画面右のホットスポット総数表示を更新
 */
void ControlPanelDefault::setInfoHotSpotCount(LPTSTR dataCount) {
	this->label_HotSpotCount->setText(dataCount);
}

/**
 * 画面右の最高温度表示を更新
 */
void ControlPanelDefault::setInfoTempMax(LPTSTR tempMax) {
	this->label_TempMax->setText(tempMax);
}

/**
 * 画面右の平均温度表示を更新
 */
void ControlPanelDefault::setInfoTempAve(LPTSTR tempAve) {
	this->label_TempAve->setText(tempAve);
}

/**
 * コマンド発行時の処理を行う。
 * @param uMsg メッセージID(WM_COMMAND)
 * @param wParam 1つ目のパラメータ
 * @param hwndControl コマンドが発生したコントロールのハンドル
 */
LRESULT ControlPanelDefault::onCommand(UINT uMsg, WPARAM wParam,
		HWND hwndControl) {
	switch (HIWORD(wParam)) {
	case BN_CLICKED:
		if (hwndControl == *button_SetFolder) {
			MainWindow::getInstance()->button_SetFolderOnClicked();
		} else if (hwndControl == *radio_ShowVisible) {
			MainWindow::getInstance()->canvasUpdate();
			// フォーカスの再設定
			SetFocus(MainWindow::getInstance()->getHandle());
		} else if (hwndControl == *radio_ShowInfrared) {
			MainWindow::getInstance()->canvasUpdate();
			// フォーカスの再設定
			SetFocus(MainWindow::getInstance()->getHandle());
		} else if (hwndControl == *radio_ShowPanel) {
			MainWindow::getInstance()->canvasUpdate();
			// フォーカスの再設定
			SetFocus(MainWindow::getInstance()->getHandle());
		} else if (hwndControl == *button_InitPicture) {
			MainWindow::getInstance()->button_InitPictureOnClicked();
		} else if (hwndControl == *radio_ShowMono) {
			MainWindow::getInstance()->InfraredTypeSelect(0);
		} else if (hwndControl == *radio_ShowColor) {
			MainWindow::getInstance()->InfraredTypeSelect(1);
		}
		break;

	case EN_KILLFOCUS:
		break;

	default:
		break;
	}

	return 0;
}

// (2017/2/16YM)キー入力処理を追加
/**
 * キー入力時の処理を行う。
 * @param uMsg メッセージID
 * @param wParam 1つ目のパラメータ
 */
LRESULT ControlPanelDefault::onKey(UINT uMsg, WPARAM wParam) {
	// (2017/2/9YM)キー押された時の処理追加
	if (uMsg == WM_KEYDOWN) {
		// キーダウンメッセージの時
		if (result != NULL) {
			//　データが読まれていない場合は何もしない
			switch (wParam) {
			int id;

		case VK_LEFT:

		case VK_DOWN:
			//　←↓押したとき
			id = mainWindow->getSelectedId();
			id--;
			if (id < 0) {
				id = result->getDataCount() - 1;
			}
			//　選択中IDをセット
			mainWindow->setSelectedId(id);
			// 表示を更新する
			mainWindow->canvasUpdate();
			break;

		case VK_RIGHT:

		case VK_UP:
			//　↑→押したとき
			id = mainWindow->getSelectedId();
			id++;
			if (id > result->getDataCount() - 1) {
				id = 0;
			}
			//　選択中IDをセット
			mainWindow->setSelectedId(id);
			// 表示を更新する
			mainWindow->canvasUpdate();
			break;
			}
		}
	}
	return 0;
}
