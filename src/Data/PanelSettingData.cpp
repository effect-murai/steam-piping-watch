/*
 * PanelSettingData.cpp
 *
 *  Created on: 2016/03/10
 *      Author: PC-EFFECT-011
 */

#include <windows.h>
#include "PanelSettingData.h"

PanelSettingData::PanelSettingData() {
	ZeroMemory(&this->m_Rect, sizeof(RECT));
	ZeroMemory(&this->m_pt, sizeof(POINT) * 4);
	this->m_pointCount = 0;

	// パネル設定モード
	this->m_panelSettingMode = PANEL_SETTING_MODE_2SET;

	// パネル選択中のID管理配列
	ZeroMemory(this->m_selectedPanelCopyIds, sizeof(int) * panelCopyMax);
	this->m_selectedPanelCopyIdSize = 0;

	// パネル設定ダイアログデータ
	initDialogData();
}

PanelSettingData::~PanelSettingData() {
	if (this->m_line1 != NULL) {
		delete this->m_line1;
	}
	if (this->m_line2 != NULL) {
		delete this->m_line2;
	}
}

void PanelSettingData::setRectLeft(LONG pos) {
	this->m_Rect.left = pos;
}
LONG PanelSettingData::getRectLeft() {
	return this->m_Rect.left;
}

void PanelSettingData::setRectTop(LONG pos) {
	this->m_Rect.top = pos;
}
LONG PanelSettingData::getRectTop() {
	return this->m_Rect.top;
}

void PanelSettingData::setRectRight(LONG pos) {
	this->m_Rect.right = pos;
}
LONG PanelSettingData::getRectRight() {
	return this->m_Rect.right;
}

void PanelSettingData::setRectBottom(LONG pos) {
	this->m_Rect.bottom = pos;
}
LONG PanelSettingData::getRectBottom() {
	return this->m_Rect.bottom;
}

void PanelSettingData::setPoint(int index, LONG x, LONG y) {
	this->m_pt[index].x = x;
	this->m_pt[index].y = y;
}
POINT* PanelSettingData::getPoint(int index) {
	return &this->m_pt[index];
}

void PanelSettingData::clearPointCount() {
	this->m_pointCount = 0;
}
void PanelSettingData::setPointCount(int count) {
	this->m_pointCount = count;
}
int PanelSettingData::getPointCount() {
	return this->m_pointCount;
}

// パネル設定モード設定
void PanelSettingData::setMode(int mode) {
	this->m_panelSettingMode = mode;
}
// パネル設定モード取得
int PanelSettingData::getMode() {
	return this->m_panelSettingMode;
}

// パネル選択中のID管理配列 設定
void PanelSettingData::setCopyId(int index, int data) {
	this->m_selectedPanelCopyIds[index] = data;
}
// パネル選択中のID管理配列 追加
void PanelSettingData::addCopyId(int data) {
	this->m_selectedPanelCopyIds[this->m_selectedPanelCopyIdSize] = data;
	this->m_selectedPanelCopyIdSize++;
}
// パネル選択中のID管理配列 取得
int PanelSettingData::getCopyId(int index) {
	return this->m_selectedPanelCopyIds[index];
}

// パネル選択中のID管理配列数 クリア
void PanelSettingData::ClearCopyIdSize() {
	this->m_selectedPanelCopyIdSize = 0;
}
// パネル選択中のID管理配列数 取得
int PanelSettingData::getCopyIdSize() {
	return this->m_selectedPanelCopyIdSize;
}

void PanelSettingData::initDialogData() {
	this->m_line1 = NULL;
	this->m_start1 = 1;		 // (2020/01/06LEE) start data は0から1に変更
	this->m_line2 = NULL;	 // (2020/01/06LEE) start data は0から1に変更
	this->m_start2 = 1;
	this->m_height = 6;
	this->m_width = 7;

	this->setLine1((TCHAR*) TEXT("A"));
	this->setLine2((TCHAR*) TEXT("A"));
}

void PanelSettingData::setLine1(TCHAR *line1) {
	TCHAR *oldLine = this->m_line1;
	int len = lstrlen(line1);
	if (len > 0) {
		this->m_line1 = new TCHAR[len + 1];
		ZeroMemory(this->m_line1, (len + 1) * sizeof(TCHAR));
		lstrcpy(this->m_line1, line1);
	} else {
		this->m_line1 = NULL;
	}
	// すでに確保している場合は解放
	if (oldLine != NULL) {
		delete oldLine;
	}
}
TCHAR* PanelSettingData::getLine1(void) {
	return this->m_line1;
}
void PanelSettingData::setStart1(int start1) {
	this->m_start1 = start1;
}
int PanelSettingData::getStart1(void) {
	return this->m_start1;
}
void PanelSettingData::setLine2(TCHAR *line2) {
	TCHAR *oldLine = this->m_line2;
	int len = lstrlen(line2);
	if (len > 0) {
		this->m_line2 = new TCHAR[len + 1];
		ZeroMemory(this->m_line2, (len + 1) * sizeof(TCHAR));
		lstrcpy(this->m_line2, line2);
	} else {
		this->m_line2 = NULL;
	}
	// すでに確保している場合は解放
	if (oldLine != NULL) {
		delete oldLine;
	}
}
TCHAR* PanelSettingData::getLine2(void) {
	return this->m_line2;
}
void PanelSettingData::setStart2(int start2) {
	this->m_start2 = start2;
}
int PanelSettingData::getStart2(void) {
	return this->m_start2;
}
void PanelSettingData::setHeight(int height) {
	this->m_height = height;
}
int PanelSettingData::getHeight(void) {
	return this->m_height;
}

void PanelSettingData::setWidth(int width) {
	this->m_width = width;
}
int PanelSettingData::getWidth(void) {
	return this->m_width;
}

void PanelSettingData::clearPoints(void) {
	for (int i = 0; i < 4; i++) {
		this->setPoint(i, 0, 0);
	}
	this->clearPointCount();
}
