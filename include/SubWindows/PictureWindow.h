/*
 * PictureWindow.h
 *
 *  Created on: 2016/02/01
 *      Author: PC-EFFECT-012
 */

#ifndef PICTUREWINDOW_H_
#define PICTUREWINDOW_H_

#include "Dialog.h"

class PictureWindow: public Dialog {
public:
	PictureWindow(HWND handle, int pictureType, int id);
	~PictureWindow(void);

	void update(void); // (2019/12/03LEE) void update=> static void update
	void update2(void); // (2019/12/25LEE) Picturebox 移動に対応

	static void create(Window *parent, int pictureType, int id);
	static void Windowupdate(HWND parent, int id);
	static void Windowupdate2(HWND parent);
	static void PopWindowHandle(HWND hParamWnd); // (2019/12/04LEE) PopupWindowのHandleを保存

private:
	int PictureId;
	double angle;
	int pictureType;
	Canvas *pictureBox;
	int pictureBox_clickX, pictureBox_clickY;

	void hotspot(int x, int y, int type);

	void pictureBoxOnDblClick(void);
	static LRESULT CALLBACK handlePictureBoxEvent(HWND hWnd, UINT uMsg,
			WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);
	// (2019/10/08LEE) 追加。　for Sub image.
	static INT_PTR CALLBACK handleEvent2(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);

};

#endif /* PICTUREWINDOW_H_ */
