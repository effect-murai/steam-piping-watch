/*
 * ControlPanelCamera.h
 *
 *  Created on: 2016/01/12
 *      Author: PC-EFFECT-011
 */

#ifndef CONTROLPANELCAMERA_H_
#define CONTROLPANELCAMERA_H_

#include <windows.h>
#include <Canvas.h>
#include "ControlPanelBase.h"

class ControlPanelCamera: public ControlPanelBase {
public:
	ControlPanelCamera(GroupBox *pWnd);
	~ControlPanelCamera();

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
	 * 移動値(垂直方向)の取得
	 */
	double getMoveV();
	/**
	 * 移動値(垂直方向)の設定
	 */
	void setMoveV(double data);

	/**
	 * 移動値(水平方向)の取得
	 */
	double getMoveH();
	/**
	 * 移動値(水平方向)の設定
	 */
	void setMoveH(double data);

	// 各方向への移動
	void moveUp(void);
	void moveDown(void);
	void moveLeft(void);
	void moveRight(void);

	/**
	 * 拡大・縮小値の取得
	 */
	double getZoom();
	/**
	 * 拡大・縮小値の設定
	 */
	void setZoom(double data);

	// 拡大・縮小
	void zoomIn();
	void zoomOut();

	/**
	 * 回転角の取得
	 */
	double getTurn();
	/**
	 * 回転角の設定
	 */
	void setTurn(double data);

	// 左回転(反時計回り)
	void turnLeft(void);
	// 右回転(時計回り)
	void turnRight(void);

	// (2019/10/31LEE) 変換した値を全体or個別に適用。
	void setall(void);
	void setone(void);
	// (2019/11/06LEE)
	void reset(void);
	int getsetingtype(); // (2019/11/06LEE) カメラ特性で全体と個別を区分。
	int getdatatype();
	void setdatatype(int data);
	/**
	 * ボタンの有効化
	 */
	void enableButton();

	/**
	 * ボタンの無効化
	 */
	void disableButton();

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
	// (2020/01/23LEE) 前画像、次画像のボタン追加
	/** 前画像ボタン */
	PushButton *button_Prev;
	/** 次画像ボタン */
	PushButton *button_Next;

	/** 移動↑ボタン */
	PushButton *button_Up;
	/** 移動(垂直) 入力テキストボックス */
	EditBox *edit_MoveV;
	/** 移動↓ボタン */
	PushButton *button_Down;

	/** 移動←ボタン */
	PushButton *button_Left;
	/** 移動(水平) 入力テキストボックス */
	EditBox *edit_MoveH;
	/** 移動→ボタン */
	PushButton *button_Right;

	/** 拡大ボタン */
	PushButton *button_ZoomIn;
	/** 拡大・縮小 入力テキストボックス */
	EditBox *edit_Zoom;
	/** 縮小ボタン */
	PushButton *button_ZoomOut;

	/** 回転←ボタン */
	PushButton *button_TurnLeft;
	/** 回転 入力テキストボックス */
	EditBox *edit_Turn;
	/** 回転→ボタン */
	PushButton *button_TurnRight;

	//全体と個別に区分して設定。　（2019/10/30LEE）　追加。
	RadioButton *radio_Setall;
	RadioButton *radio_Setone;
	PushButton *button_Reset;

	/** タイトルラベル */
	Label *label_Title;
	/** 説明ラベル */
	Label *label_Info;

	// (2019/11/26LEE) 全体モード案内するために追加。
	Label *all_mode_Title;
	Label *all_mode_Info;

};

#endif /* CONTROLPANELCAMERA_H_ */
