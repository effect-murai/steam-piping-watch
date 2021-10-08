/*
 * Canvas.cpp
 *
 *  Created on: 2016/01/29
 *      Author: PC-EFFECT-011
 */

#include "Canvas.h"

//------------------------------------------------------------------------------
// Canvas Class
//------------------------------------------------------------------------------

Canvas::Canvas(int style, int x, int y, int width, int height,
		WindowContainer *parent) :
		Control(TEXT("STATIC"), NULL, style | SS_OWNERDRAW, x, y, width, height,
				parent, WS_EX_CLIENTEDGE) {
	// バックバッファの初期化
	initBackBuffer(width, height);
}

Canvas::Canvas(int x, int y, int width, int height, WindowContainer *parent) :
		Control(TEXT("STATIC"), NULL, WS_VISIBLE | SS_OWNERDRAW, x, y, width,
				height, parent, WS_EX_CLIENTEDGE) {
	// バックバッファの初期化
	initBackBuffer(width, height);
}

Canvas::Canvas(int x, int y, int width, int height, WindowContainer *parent,
		bool edge) :
		Control(TEXT("STATIC"), NULL, WS_VISIBLE | SS_NOTIFY | SS_OWNERDRAW, x,
				y, width, height, parent, edge ? WS_EX_CLIENTEDGE : 0) {
	// バックバッファの初期化
	initBackBuffer(width, height);
}

Canvas::~Canvas(void) {
	// バックバッファの削除
	DeleteObject(SelectObject(backBuffer, defaultBitmap));
	DeleteDC(backBuffer);
}

void Canvas::transfer(HDC frontBuffer) {
	RECT rect;
	getClientRect(&rect);
	// バックバッファの転送
	BitBlt(frontBuffer, 0, 0, rect.right, rect.bottom, backBuffer, 0, 0,
	SRCCOPY);
}

void Canvas::initBackBuffer(int width, int height) {
	// バックバッファの初期化
	HWND desktop = GetDesktopWindow();
	HDC hdc = GetDC(desktop);
	backBuffer = CreateCompatibleDC(NULL);
	defaultBitmap = (HBITMAP) SelectObject(backBuffer,
			(HBITMAP) CreateCompatibleBitmap(hdc, width, height));
	ReleaseDC(desktop, hdc);
}

void Canvas::resizeBackBuffer(void) {
	RECT client;
	getClientRect(&client);
	int width = client.right - client.left;
	int height = client.bottom - client.top;
	HWND desktop = GetDesktopWindow();
	HDC hdc = GetDC(desktop);
	DeleteObject(
			SelectObject(backBuffer,
					(HBITMAP) CreateCompatibleBitmap(hdc, width, height)));
	ReleaseDC(desktop, hdc);
}

HDC Canvas::getBackBuffer(void) {
	return backBuffer;
}

void Canvas::clear(HBRUSH brush) {
	RECT rect;
	getClientRect(&rect);
	FillRect(getBackBuffer(), &rect, brush);
}

void Canvas::drawText(int x, int y, const TCHAR *text) {
	int length = lstrlen(text);
	TextOut(getBackBuffer(), x, y, text, length);
}

void Canvas::drawEllipse(int x1, int y1, int x2, int y2) {
	Ellipse(getBackBuffer(), x1, y1, x2, y2);
}

void Canvas::drawCircle(int x, int y, int radius) {
	drawEllipse(x - radius, y - radius, x + radius, y + radius);
}

void Canvas::drawLine(int x1, int y1, int x2, int y2) {
	MoveToEx(getBackBuffer(), x1, y1, NULL);
	LineTo(getBackBuffer(), x2, y2);
}

void Canvas::drawPolyline(POINT *points, int count) {
	Polyline(getBackBuffer(), points, count);
}

void Canvas::drawPolygon(POINT *points, int count) {
	Polygon(getBackBuffer(), points, count);
}

void Canvas::drawRect(int x1, int y1, int x2, int y2) {
	Rectangle(getBackBuffer(), x1, y1, x2, y2);
}

HGDIOBJ Canvas::selectGdiObject(HGDIOBJ object) {
	return SelectObject(getBackBuffer(), object);
}
