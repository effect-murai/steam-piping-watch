/*
 * PanelSettingData.h
 *
 *  Created on: 2016/03/10
 *      Author: PC-EFFECT-011
 */

#ifndef PANELSETTINGDATA_H_
#define PANELSETTINGDATA_H_

class PanelSettingData {
public:
	PanelSettingData();
	~PanelSettingData();

	void setRectLeft(LONG pos);
	LONG getRectLeft();

	void setRectTop(LONG pos);
	LONG getRectTop();

	void setRectRight(LONG pos);
	LONG getRectRight();

	void setRectBottom(LONG pos);
	LONG getRectBottom();

	void setPoint(int index, LONG x, LONG y);
	POINT* getPoint(int index);

	void clearPointCount();
	void setPointCount(int count);
	int getPointCount();

	// パネル設定モード設定
	void setMode(int mode);
	// パネル設定モード取得
	int getMode();

	// パネル選択中のID管理配列 設定
	void setCopyId(int index, int data);
	// パネル選択中のID管理配列 追加
	void addCopyId(int data);
	// パネル選択中のID管理配列 取得
	int getCopyId(int index);

	// パネル選択中のID管理配列数 クリア
	void ClearCopyIdSize();
	// パネル選択中のID管理配列数 取得
	int getCopyIdSize();

	// パネル設定ダイアログデータ関連
	void initDialogData();

	void setLine1(TCHAR *line1);
	TCHAR* getLine1();

	void setStart1(int start);
	int getStart1();

	void setLine2(TCHAR *line2);
	TCHAR* getLine2();

	void setStart2(int start);
	int getStart2();

	void setHeight(int height);
	int getHeight();

	void setWidth(int width);
	int getWidth();

	void clearPoints(void);

	// パネル選択中のID管理配列最大数
	static const int panelCopyMax = 100000;

	// パネル設定モード
	enum {
		PANEL_SETTING_MODE_2SET = 0,
		PANEL_SETTING_MODE_4SET,
		PANEL_SETTING_MODE_COPY,
		PANEL_SETTING_MODE_MOVE
	};

private:
	RECT m_Rect;
	POINT m_pt[4];
	int m_pointCount;

	// パネル設定モード
	int m_panelSettingMode;

	// パネル選択中のID管理配列
	int m_selectedPanelCopyIds[panelCopyMax];
	// パネル選択中のID管理配列数
	int m_selectedPanelCopyIdSize;

	// パネル設定ダイアログデータ
	TCHAR *m_line1;
	int m_start1;
	TCHAR *m_line2;
	int m_start2;
	int m_height;
	int m_width;

};

#endif /* PANELSETTINGDATA_H_ */
