/*
 * PanelInfo.h
 *
 *  Created on: 2016/01/27
 *      Author: PC-EFFECT-002
 */

#ifndef PANELINFO_H_
#define PANELINFO_H_

#include "MainWindow.h"
#include "ResultData.h"

typedef struct {
	TCHAR *panelName;
	int HotspotCount;
	Vector2D points[4];
	int cx;
	int cy;
} PanelInfo;

class PanelData {
public:
	PanelData(void);

	void setSelectedPanelId(int id);

	void setSelectedInternalId(int x, int y);

	int getSelectedPanelId(void);

	POINT getSelectedInternalId(void);

	bool setPanelName(int id, TCHAR *name);

	TCHAR* getPanelName(int id);

	bool setPanelHotspotCount(int id, int count);

	bool movePanel(int id, double x, double y);

	Vector2D* getPanelPos(int id);

	int getPanelNameCountMax(void);

	int getHotspotPanelCount(int id);

	bool panelSettingDelete(int id);

	int panelSettingAdd(Vector2D *panelArea);

	bool savePanelInfo(TCHAR *filePath);

	bool loadPanelInfo(const TCHAR *filePath);

	bool savePanelInfo(void);

	bool loadPanelInfo(void);

	bool saveHotspotNumInfo(TCHAR *filePath);

	TCHAR* getPanelNameDetail(double x, double y);

	Vector2D getPoint(int id, int pos);

	void setHotspotCount(int id, int count);

	void getInternalSplitCount(int id, SIZE *size);

	void setInternalSplitCount(int id, int cx, int cy);

	int findPanel(double x, double y);

	bool getInternalSquareFind(double x, double y, int *id, int *row, int *col,
			Vector2D *square);

	void getInternalSquare(int id, int row, int column, Vector2D *square);

	// 定数
	static const int NOT_SELECTED = -1;

private:
	std::vector<PanelInfo> panelInfo;

	int selectedPanelId;

	POINT internalId;

};
#endif /* PANELINFO_H_ */
