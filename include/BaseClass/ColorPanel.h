/*
 * ColorPanel.h
 *
 *  Created on: 2016/02/09
 *      Author: PC-EFFECT-011
 */

#ifndef COLORPANEL_H_
#define COLORPANEL_H_

#include "Control.h"
#include "Canvas.h"
#include "EditBox.h"
#include "Label.h"

class ColorPanel: public Control, public WindowContainer {

public:
	static const int SIZE_WIDTH = 60;
	static const int SIZE_HEIGHT = 300;
	static const int SIZE_LABEL = 24;
	static const int SIZE_CONTROL = ColorPanel::SIZE_HEIGHT
			- (ColorPanel::SIZE_LABEL * 4);
	static const double DEFAULT_TEMP_MAX = 20.0L;
	static const double DEFAULT_TEMP_MIN = 5.0L;

public:
	ColorPanel(LPCTSTR title, int x, int y, WindowContainer *parent);
	virtual ~ColorPanel();

	/**
	 * ウィンドウハンドルを取得する。
	 */
	HWND getHandle(void);

	/**
	 * フォントを設定する。
	 * @param hFont フォント
	 */
	void setFont(HFONT hFont);

	/**
	 * 温度に対応するカラー値を取得する。
	 * @param temp 温度
	 */
	COLORREF getColor(double temp);

	/**
	 * 最高温度を取得する。
	 */
	double getTempMax() const;

	/**
	 * 最高温度を設定する。
	 * @param tempMax 最高温度
	 */
	void setTempMax(double tempMax);

	/**
	 * 最低温度を取得する。
	 */
	double getTempMin() const;

	/**
	 * 最低温度を設定する。
	 * @param tempMin 温度
	 */
	void setTempMin(double tempMin);

private:
	/**
	 * コントロールにリソースのビットマップを設定する。
	 * @param id リソースID
	 */
	void setBitmap(int id);

	/**
	 * コントロールを更新する。
	 */
	void update();

	/**
	 * グループボックスのイベントを処理する。
	 * @param hWnd グループボックスのハンドル
	 * @param uMsg メッセージID
	 * @param wParam 最初のメッセージパラメータ
	 * @param lParam 2番目のメッセージパラメータ
	 */
	static LRESULT CALLBACK handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);

	/**
	 * グループボックスのイベントを処理する。
	 * @param hWnd グループボックスのハンドル
	 * @param uMsg メッセージID
	 * @param wParam 最初のメッセージパラメータ
	 * @param lParam 2番目のメッセージパラメータ
	 */
	virtual LRESULT ControlProc(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);

private:
	// コントロール
	Canvas *m_Control;
	EditBox *m_Edit_Max;
	EditBox *m_Edit_Min;
	Label *m_Label_Name;

	// 温度範囲
	double m_Temp_Max;
	double m_Temp_Min;
};

#endif /* COLORPANEL_H_ */
