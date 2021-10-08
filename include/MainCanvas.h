/*
 * MainCanvas.h
 *
 *  Created on: 2016/03/15
 *      Author: PC-EFFECT-011
 */

// 2017/4/16　画像タイプを再読み込みフラグに変更する関数追加
#ifndef MAINCANVAS_H_
#define MAINCANVAS_H_

#include "Canvas.h"

class MainCanvas: public Canvas {
public:
	/**
	 * スタイルを指定してメインキャンバスを作成する。
	 * @param style スタイル
	 * @param x 左の座標
	 * @param y 上の座標
	 * @param width 幅
	 * @param height 高さ
	 * @param parent 親ウィンドウ
	 */
	MainCanvas(int style, int x, int y, int width, int height,
			WindowContainer *parent);

	/**
	 * デストラクタ
	 */
	virtual ~MainCanvas();

	/**
	 * 画像表示(指定)
	 * @param id 画像ID
	 * @param pictureType 画像種別
	 * ※ データ(resultData)有無は、上位で確認すること
	 */
	void drawPicture(int id, int pictureType);

	/**
	 * 画像表示(全部)
	 * @param pictureType 画像種別
	 * ※ データ(resultData)有無は、上位で確認すること
	 */
	void drawPictureAll(int pictureType);

	/**
	 * 画像表示(管理番号入力)
	 * @param id 画像ID
	 * @param pictureType 画像種別
	 * ※ データ(resultData)有無は、上位で確認すること
	 */
	void setNumberdrawPictureAll(int pictureType, int size);

	/**
	 * 画像枠表示(指定)
	 * @param id 画像ID
	 * @param pictureType 画像種別
	 * ※ データ(resultData)有無は、上位で確認すること
	 */
	void drawPictureFrame(int id, int pictureType);

	/**
	 * 画像ID表示
	 * @param id 画像ID
	 * @param maxCount 全画像数
	 * ※ データ(resultData)有無は、上位で確認すること
	 */
	void drawId(int id, int maxCount);

	/**
	 * ホットスポット表示
	 * @param radius ホットスポット表示サイズ(半径)
	 * ※ データ(resultData)有無は、上位で確認すること
	 */
	void drawHotspotAll(int radius);

	/**
	 * ホットスポット表示(選択画像内)
	 * @param id 画像ID
	 * @param radius ホットスポット表示サイズ(半径)
	 * ※ データ(resultData)有無は、上位で確認すること
	 */
	void drawHotspot(int id, int radius);

	/**
	 * ホットスポット表示(解析範囲内)
	 * @param radius ホットスポット表示サイズ(半径)
	 * ※ データ(resultData)有無は、上位で確認すること
	 */
	void drawHotspotArea(int radius);

	/**
	 * ホットスポット解析範囲表示
	 */
	void drawAreaSetting();

	/**
	 * 管理パネル表示
	 */
	void drawPanelSetting();

	void drawMultiSelectedPanels(int *selected, int count);

	/**
	 * ホットスポット表示状態表示
	 */
	void drawHotspotShowMode(void);

	/**
	 * 座標のクリップ
	 */
	void clipPos(POINT &pos);

	// (2017/4/16YM)画像タイプを再読み込みフラグに変更する関数追加
	void PicTypReset(void);

private:
	/**
	 * 管理パネル表示(選択中)
	 */
	void drawSelectedPanel(int id);

	/**
	 * 管理パネル名表示
	 */
	void drawPanelName(int id);

	// (2020/01/07LEE) Fontを区分して使用するために追加
	HFONT mainCanvasFont;

};

#endif /* MAINCANVAS_H_ */
