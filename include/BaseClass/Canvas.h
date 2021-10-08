/*
 * Canvas.h
 *
 *  Created on: 2016/01/28
 *      Author: PC-EFFECT-011
 */

#ifndef CANVAS_H_
#define CANVAS_H_

#include "Control.h"

/**
 * キャンバス
 */
class Canvas: public Control {
public:
	/**
	 * スタイルを指定してキャンバスを作成する。
	 * @param style スタイル
	 * @param x 左の座標
	 * @param y 上の座標
	 * @param width 幅
	 * @param height 高さ
	 * @param parent 親ウィンドウ
	 */
	Canvas(int style, int x, int y, int width, int height,
			WindowContainer *parent);
	/**
	 * キャンバスを作成する。
	 * @param x 左の座標
	 * @param y 上の座標
	 * @param width 幅
	 * @param height 高さ
	 * @param parent 親ウィンドウ
	 */
	Canvas(int x, int y, int width, int height, WindowContainer *parent);
	/**
	 * 境界線の有無を指定してキャンバスを作成する。
	 * @param x 左の座標
	 * @param y 上の座標
	 * @param width 幅
	 * @param height 高さ
	 * @param parent 親ウィンドウ
	 * @param edge 境界線の有無(true:あり/false:なし)
	 */
	Canvas(int x, int y, int width, int height, WindowContainer *parent,
			bool edge);
	virtual ~Canvas(void);

	/**
	 * バックバッファのデバイスコンテキストする。
	 * @return バックバッファのデバイスコンテキスト
	 */
	HDC getBackBuffer(void);
	/**
	 * バックバッファをフロントバッファに転送する。
	 * @param frontBuffer フロントバッファのデバイスコンテキスト
	 */
	void transfer(HDC frontBuffer);
	/**
	 * 指定したブラシを使用して画面をクリアする。@n
	 * ※ソリッドブラシを使用してください。
	 * @param brush 画面クリア使用するブラシ
	 */
	void clear(HBRUSH brush);
	/**
	 * バックバッファのサイズをキャンバスサイズに合わせる。@n
	 * リサイズ後は必ずこの関数を呼び出してください。
	 */
	void resizeBackBuffer(void);

	/**
	 * テキストを描画する。
	 * @param x 描画位置のx座標
	 * @param y 描画位置のy座標
	 * @param text 描画する文字列
	 */
	void drawText(int x, int y, const TCHAR *text);
	/**
	 * 楕円を描画する。
	 * @param x1 左上のx座標
	 * @param y1 左上のy座標
	 * @param x2 右下のx座標
	 * @param y2 右下のy座標
	 */
	void drawEllipse(int x1, int y1, int x2, int y2);
	/**
	 * 円を描画する。
	 * @param x 中心のx座標
	 * @param y 中心のy座標
	 * @param radius 円の半径
	 */
	void drawCircle(int x, int y, int radius);
	/**
	 * 線分を描画する。
	 * @param x1 開始点のx座標
	 * @param y1 開始点のy座標
	 * @param x2 終了点のx座標
	 * @param y2 終了点のy座標
	 */
	void drawLine(int x1, int y1, int x2, int y2);
	/**
	 * ポリラインを描画する。
	 * @param points 各点の座標を格納した配列
	 * @param count 配列の要素数
	 */
	void drawPolyline(POINT *points, int count);
	/**
	 * 多角形を描画する。
	 * @param points 各頂点の座標を格納した配列
	 * @param count 配列の要素数
	 */
	void drawPolygon(POINT *points, int count);

	void drawRect(int x1, int y1, int x2, int y2);

	/**
	 * バックバッファにGDIオブジェクトを設定する。
	 * @param object GDIオブジェクト
	 * @return 前に設定されていたGDIオブジェクト
	 * @see SelectObject
	 */
	HGDIOBJ selectGdiObject(HGDIOBJ object);

private:
	HDC backBuffer;
	HBITMAP defaultBitmap;

	/**
	 * バックバッファを初期化する。
	 * @param width キャンバスの幅
	 * @param height キャンバスの高さ
	 */
	void initBackBuffer(int width, int height);
};

#endif /* CANVAS_H_ */
