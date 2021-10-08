/*
 * MainWindow.h
 *
 *  Created on: 2015/10/09
 *      Author: PC-EFFECT-012
 */

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <vector>
#include "Controls.h"
#include "ControlPanel.h"
#include "CsvData.h"
#include "CommonData.h"
#include "PanelSettingData.h"
#include "AreaSettingData.h"
#include "MainCanvas.h"

/**
 * メインウィンドウ
 */
class MainWindow: public Window, public WindowContainer {
public:
	MainWindow(void);
	~MainWindow(void);
	void onCreate(void);

	/**
	 * ウィンドウのハンドルを取得する。
	 * @return ウィンドウのハンドル
	 */
	HWND getHandle(void);
	/**
	 * メインウィンドウの最小サイズを取得する。
	 * @return メインウィンドウの最小サイズ
	 */
	int getMinimalSize(void);

	/**
	 * ミニパネル(低解像度ディスプレイ用)モードの取得
	 * @return true:有効/false:無効
	 */
	bool IsMiniPanelMode();

	/**
	 * リサイズ時の処理を行う。
	 * @param sizingEdge リサイズを行っている境界
	 * @param windowRect リサイズ後のウィンドウサイズ
	 */
	LRESULT onResize(WPARAM sizingEdge, RECT *windowRect);
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
	/**
	 * コントロールの再描画を行う。
	 * @param uMsg メッセージID
	 * @param wParam 1つ目のパラメータ
	 * @param pDIS コントロールの再描画パラメータ
	 */
	LRESULT onDrawItem(UINT uMsg, WPARAM wParam, LPDRAWITEMSTRUCT pDIS);
	/**
	 * キャンバスのイベント処理を行う。
	 * @param hWnd 対象ウィンドウのハンドル
	 * @param uMsg メッセージID
	 * @param wParam 1つ目のパラメータ
	 * @param lParam 2つ目のパラメータ
	 */
	LRESULT CanvasProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	/**
	 * コントロールの背景色を指定する処理を行う。
	 * @param hDC 再描画対象のデバイスコンテキスト
	 *　@param hWnd 再描画対象のウィンドウハンドル
	 *　@return 背景色に使用するブラシのハンドル(0はデフォルト)
	 */
	LRESULT onColorCtrl(HDC hDC, HWND hWnd);

	LRESULT onMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam);

	/**
	 * フォルダ選択ボタンをクリックしたときの処理
	 */
	void button_SetFolderOnClicked(void);

	// (2017/4/4YM)画像位置初期化ボタンを追加
	void button_InitPictureOnClicked(void);

	/**
	 * ホットスポットの自動検出を行う。@n
	 * ホットスポットを記述したファイルが生成されている場合は、
	 * それを読み込み自動検出の処理は実行しない。
	 * @param force trueを指定すると強制的に自動検出を行う。
	 */
	void detectHotspot(bool force);

	/**
	 * 選択中の画像番号を取得する。
	 * @return 選択中の画像番号(>0:未選択)
	 */
	int getSelectedId(void);
	/**
	 * 次の画像を選択する。
	 */
	void selectNext(void);

	/**
	 * 前の画像を選択する。
	 */
	void selectPrev(void);

	/**
	 * メインキャンバスと座標マップを更新する。
	 */
	void canvasUpdate(void);
	void resultDataSave(void);
	void resultDataWrite(void);
	void canvasUpdateNext(void);
	void canvasUpdatePrev(void);
	void immediatelyRedraw(void);

	bool saveWholePicture(const TCHAR *path, int pictureType);
	bool saveWholePictureWithPanelData(const TCHAR *path, int pictureType);
	void saveWholePictureSplit(int *progress);

	bool makeHotpotIdList(int id, double viewRatio);
	void removeHotspotIdList(int id);
	// (2020/01/10LEE) testuing
	void removeHotspotALLIdList();
	// (2017/10/26YM)ホットスポットIDの変更処理を追加
	void refreshHotspotIdList(int id);
	bool saveHotspotIdListFile(TCHAR *filePath);
	bool saveHotspotIdList(void);
	// (2020/01/14LEE) ID更新をするために追加
	void resaveHotspotIdList(int idnum, int picnum, int hotnum);
	bool readHotspotIdListFile(TCHAR *filePath);
	bool readHotspotIdList(void);
	int getHotspotIdListSize(void);
	int getHotspotIdListId(int listNo);
	double getHotspotIdListViewRatio(int listNo);
	int getHotspotIdListListNo(int id);

	// (2020/02/12LEE) 報告書を出力する時、Imageが小さい場合拡大
	int Picturesizecheck(int width, int height, double Ratio, double x,
			double y);

	bool makeReport(TCHAR *path);
	bool makeWholePictureReport(TCHAR *path);

	/**
	 * モードを設定する。
	 * @param mode 操作モード
	 */
	void setMode(int mode);
	/**
	 * モードを取得する。
	 * @return 操作モード
	 */
	int getMode(void);

	/**
	 * コントロールパネルを設定する。
	 * @param mode 操作モード
	 */
	void setControlPanel(int mode);

	void hotspotSumMessage(void);

	// ダイアログ表示系
	LPTSTR showSaveXLSMFileDialog(void);
	LPTSTR showSaveCSVFileDialog(void);
	LPTSTR showLoadCSVFileDialog(void);
	LPTSTR showSaveCSVXLSMFileDialog(void);
	int showMessageBox(int textId, int titleId, int type);
	bool outputCsv(void);

	//(2020/01/15LEE) GPS ERROR MESSAGE BOX
	int GpsErrorMessageBox(int picnum, int errornum, int titleId, int type);

	void excludeHotspot(void);

	/**
	 * パネル管理領域をコピーする
	 */
	void PanelSettingPanelCopy();

	/**
	 * パネル管理領域を削除する
	 */
	void PanelSettingPanelDelete();

	/**
	 * ホットスポット数一覧を保存する
	 */
	void saveHotspotNumInfo();

	/**
	 * 管理番号を保存する
	 */
	void savePanelInfo();

	/**
	 * 管理番号を読み込む
	 */
	void loadPanelInfo();

	/**
	 * 解析範囲設定を初期化する。
	 */
	void AreaSettingInit();

	/**
	 * 解析範囲設定を完了する。
	 */
	void AreaSettingComplete();

	/**
	 * 解析範囲設定を取り消す。
	 */
	void AreaSettingCancel();

	/**
	 * 解析範囲設定を消去する。
	 */
	void AreaSettingClear();

	/** 表示倍率を取得する*/
	double getViewRatio(void);

	/**
	 * 指定した画像の方位を取得する。
	 * @param id 画像番号
	 * @return 方位
	 */
	double getDirection(int id);

	// GPS座標をキャンバス上の座標に変換する
	void gpsPosToPixel(double gpsX, double gpsY, int *x, int *y);
	void gpsPosToPixel(Vector2D *real, POINT *projective);
	void gpsPosToPixel(int id, int *x, int *y);

	// 画像の表示倍率を計算する
	double getPictureRatio(int id);

	// ホットスポットの描画
	void drawHotspotOnEachPicture(int id, COLORREF color, int radius);
	void getPictureRect(int id, int pictureType, POINT *rect);

	bool excludePicture(int id, int pictureType);

	void* loadPicture(int id, int pictureType);

	bool mouseCursorLocationCheck(RECT *rect);

	bool panelPosCheck();

	// 縮尺から高度を求めてresultに格納する(全画像)
	void SetHeightAll();
	// (2017/4/3YM追加)『角度一括適用』ボタン用処理追加
	void SetTurnAll();

	// (2019/11/06LEE) 追加。
	bool Reset(void);

	void createTempPictures(void);

	// ホットスポット表示モード関連
	int getHotspotShowMode(void);
	bool getEnabledHotspotShowMode(void);
	void setEnabledHotspotShowMode(bool mode);
	void toggleEnabledHotspotShowMode(void);
	bool getHotspotCandidateShowMode(void);
	void setHotspotCandidateShowMode(bool mode);
	void toggleHotspotCandidateShowMode(void);
	bool getDisabledHotspotShowMode(void);
	void setDisabledHotspotShowMode(bool mode);
	void toggleDisabledHotspotShowMode(void);
	bool getKeypointShowMode(void);
	void setKeypointShowMode(bool mode);
	void toggleKeypointShowMode(void);
	void changeHotspotShowMode(int mode);

	// データの保存
	void saveAll(void);

	void updateHotspot();

	void showOutOfRangeError(int titleId, double min, double max,
			int decimalCount);

	void saveTempRange(bool force);
	void loadTempRange(void);

	// (2017/2/2YM)ホットスポット自動リンク処理追加
	void HotspotAutoLink(void);
	void KeyPointAutoDetect(int id);	// (2017/2/6YM)特徴点検出処理追加（別関数化）
	// ↑ここまで追加

	// (2017/2/16YM)selectedPictureIDセット関数追加
	void setSelectedId(int id);
	// ↑ここまで追加

	// (2017/4/4YM)ポジションデータ再読み込み関数を追加
	void ReloadPositionInfo(void);

	// (2017/4/4YM)赤外線画像のタイプ選択関数追加
	void InfraredTypeSelect(int Type);

	// (2017/4/4YM)赤外線画像タイプの取得関数追加
	int getInfraredType(void);

	// (2017/4/6YM)全方位特徴点を検出する仕様追加
	void KeyPointAutoDetect2(int id);

	// (2017/5/31YM)閾値温度を取得する関数を追加
	double getThresholdTemp(void);

	// (2017/6/1YM)閾値温度をセットする関数を追加
	void setThresholdTemp(float threshold);

	// (2017/6/14YM)指定したタイプのホットスポットを削除する関数を追加
	void removeTypeHotspots(int chktype);

	// (2017/6/20YM)ホットスポット全削除処理関数追加
	void removeAllHotspots(void);

	// (2017/6/20YM)特徴点全削除処理関数追加
	void removeAllKeypoints(void);

	// (2019/12/17LEE) ProgressBarに対応するために追加
	void makingbinary(TCHAR *picturefath);

public:
	// getter関連
	const std::vector<HotspotIdList>& getHotspotIdList() const {
		return hotspotIdList;
	}

	double getViewPixelPerMeter() const {
		return viewPixelPerMeter;
	}

	double getViewX() const {
		return viewX;
	}

	double getViewY() const {
		return viewY;
	}

	const HotSpotArea& getHotSpotAreas() const {
		return HotSpotAreas;
	}

	HRGN getHotSpotAreaRgn() const {
		return HotSpotAreaRgn;
	}

	BOOL IsAreaComplete() const {
		return isAreaComplete;
	}

public:
	// CSVデータ
	CsvData *csvData;

	// PanelSettingデータ
	PanelSettingData *panelSettingData;

	/**
	 * メインウィンドウのインスタンスを取得する。
	 */
	static MainWindow* getInstance(void);

	static const int FLAG_RESIZED = 0x80000000;

	static const int NOT_SELECTED = -1;

	static const int PANEL_SIZE_WIDTH = 512;

	enum {
		CONTROL_PANEL_MODE_DEFAULT = 0,
		CONTROL_PANEL_MODE_CAMERA,
		CONTROL_PANEL_MODE_SHOOTINGDATA,
		CONTROL_PANEL_MODE_AREA,
		CONTROL_PANEL_MODE_HOTSPOT,
		CONTROL_PANEL_MODE_PANELSETTING
	};

	enum {
		VIEW_MODE_ALL = 0, VIEW_MODE_INDIVIDUAL
	};

	// (2017/5/31YM)閾値温度を格納するグローバル変数を追加, (2019/10/07LEE) float => doubleで変更。
	double thresholdtemp;

	int missioncounting;

private:
	/** UIのフォント */
	HFONT hUIFont;
	/** メインキャンバスのフォント */
	HFONT hCanvasFont;

	/** ミニマップ上のマウスイベント発生位置(X座標) */
	int minimapX;
	/** ミニマップ上のマウスイベント発生位置(Y座標) */
	int minimapY;
	/** メインキャンバス上のマウスイベント発生位置(X座標) */
	int mainX;
	/** メインキャンバス上のマウスイベント発生位置(Y座標) */
	int mainY;
	/** メインキャンバス上の前回マウスイベント発生位置 */
	POINT prevMainPos;
	/** ミニパネル(低解像度ディスプレイ用)モードの有無(true:有効/false:無効) */
	bool miniPanelMode;
	/** ウィンドウ枠のサイズ */
	int borderSize;
	/** 選択中の画像番号 */
	int selectedPictureId;

	double rectratio;

	/** 選択中のホットスポット */
	int selectedHotspotPictureId;
	int selectedHotspotPointId;

	/** ナビゲータに表示中の文字列ID */
	int navigatorMessageId;
	/** 操作モード */
	int controlMode;

	double viewPixelPerMeter;

	/** 表示の中心点 */
	double viewX;
	/** 表示の中心点 */
	double viewY;

	/** ホットスポット表示モード */
	bool hotspotShowMode;

	/**　ホットスポット候補表示モード　*/
	bool hotspotCandidateShowMode;

	/** 無効ホットスポット表示モード */
	bool disabledHotspotShowMode;

	/** 特徴点表示モード */
	bool keypointShowMode;

	/** panelSettingモード用 **/
	BOOL blMouse;

	/** ホットスポット解析範囲設定モード用 */
	HotSpotArea HotSpotAreas;
	HRGN HotSpotAreaRgn;
	BOOL isAreaMove;
	BOOL isAreaComplete;
	AreaSettingData *areaSettingData;

	/** ホットスポット更新の有無 */
	bool isHotspotUpdated;

	// 分割後の画像サイズ
	static const int splitWidth = 5000;
	static const int splitHeight = 5000;

	// 俯瞰画像全体サイズを1[cm/px]（単位pxあたり1cm）に設定
	// 1[m]あたりのpx数に10指定(10[px/m]=1[cm/px])
	// @todo 仮(変更不可)
	static const int PIXELS_PER_METER = 10;

	/**　報告書ホットスポットid、表示倍率のリスト用*/
	std::vector<HotspotIdList> hotspotIdList;
	// 表示倍率のリスト用backup
	std::vector<HotspotIdList> hotspotIdListbackup;

	//--------------------------------------------------------
	// コントロール
	//--------------------------------------------------------
	/** メインキャンバス */
	MainCanvas *canvas_Main;
	/** 座標マップキャンバス */
	Canvas *canvas_MiniMap;
	/** コントロールパネル */
	ControlPanel *controlPanel;

	//--------------------------------------------------------
	// メインキャンバスをクリックしたときの処理
	//--------------------------------------------------------
	void canvas_MainOnClicked(void);
	void canvas_MainOnDblClicked(void);

	//--------------------------------------------------------
	// 座標マップをクリックしたときの処理
	//--------------------------------------------------------
	void canvas_MiniMapOnClicked(void);
	void canvas_MiniMapOnDblClicked(void);

	/**
	 * メインキャンバスの表示を更新するときの処理
	 */
	void canvas_MainUpdate(void);
	/**
	 * 座標マップの表示を更新するときの処理
	 */
	void canvas_MiniMapUpdate(void);

	/**
	 * 指定した画像の高度を取得する。
	 * @param id 画像番号
	 * @return 高度
	 */
	double getHeight(int id);

	void zoomIn(int wheelCount, int xPos, int yPos);
	void scroll(void);
	void zoom(int wheelCount, int xPos, int yPos);

	void getPanelSize(double *width, double *height);
	void selectHotspot(int pictureNo, int pointNo);

	// 4点指定方式四角形判定関数
	bool squareJudgment(POINT pt1, POINT pt2, POINT pt3, POINT pt4);
	// 4点指定方式四角形追加関数
	int squareAdd(POINT pt1, POINT pt2, POINT pt3, POINT pt4);

	//--------------------------------------------------------
	// デフォルトモードの処理
	//--------------------------------------------------------
	bool canvas_Main_DefaultOnClick(void);
	bool canvas_Main_DefaultOnDblClick(void);
	bool canvas_Main_DefaultUpdate(void);
	LRESULT canvas_Main_DefaultProc(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);

	//--------------------------------------------------------
	// カメラ調整モードの処理
	//--------------------------------------------------------
	bool canvas_Main_CameraAdjustmentOnClick(void);
	bool canvas_Main_CameraAdjustmentOnDblClick(void);
	bool canvas_Main_CameraAdjustmentUpdate(void);
	LRESULT canvas_Main_CameraAdjustmentProc(HWND hWnd, UINT uMsg,
			WPARAM wParam, LPARAM lParam);

	//--------------------------------------------------------
	// 画像補正モードの処理
	//--------------------------------------------------------
	bool canvas_Main_ShootingDataAdjustmentOnClick(void);
	bool canvas_Main_ShootingDataAdjustmentOnDblClick(void);
	bool canvas_Main_ShootingDataAdjustmentUpdate(void);
	LRESULT canvas_Main_ShootingDataAdjustmentProc(HWND hWnd, UINT uMsg,
			WPARAM wParam, LPARAM lParam);
	bool canvas_Main_ShootingDataAdjustmentResultSet(void);
	bool canvas_Main_ShootingDataAdjustmentResultGet(void);

	//--------------------------------------------------------
	// 解析範囲設定モードの処理
	//--------------------------------------------------------
	bool canvas_Main_AreaSettingOnClick();
	bool canvas_Main_AreaSettingOnDblClick();
	bool canvas_Main_AreaSettingUpdate();

	LRESULT canvas_Main_AreaSettingProc(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);

	//--------------------------------------------------------
	// ホットスポット補正モードの処理
	//--------------------------------------------------------
	bool canvas_Main_HotspotAdjustmentOnClick(void);
	bool canvas_Main_HotspotAdjustmentOnDblClick(void);
	bool canvas_Main_HotspotAdjustmentUpdate(void);
	bool canvas_Main_HotspotAdjustmentOnPaint(void);
	LRESULT canvas_Main_HotspotAdjustmentProc(HWND hWnd, UINT uMsg,
			WPARAM wParam, LPARAM lParam);

	//--------------------------------------------------------
	// panel管理番号割当の処理
	//--------------------------------------------------------
	bool canvas_Main_PanelSettingOnClick(void);
	bool canvas_Main_PanelSettingOnDblClick(void);
	bool canvas_Main_PanelSettingUpdate(void);
	LRESULT canvas_Main_PanelSettingProc(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);
	void canvas_Main_PanelSettingOnLButtonUp(int x, int y);
	int panelFind(int mainX, int mainY);
	bool panelFind();
	bool panelMove();
	void updatePanelSettingInfo();
	void updateHotspotCount(int panelId);

	bool excludePictureForScope(RECT scopeRect, int id, int pictureType,
			double pixelPerMeter);

	bool checkHotspot(int x, int y);
	void addHotspot(int x, int y, int type);
	bool addLink(void);
	int deleteLinkLineSegment(int x, int y, int radius);
	int getClickedPictureId(int x, int y);

	void loadPositionInfo(void);

	void createWholePictures(void);
	void createWholePicturesReport(void);
	// (2017/2/9YM)2016/10/4田崎さん修正内容を反映
	void deleteWholePictures(void);

	void createDefaultTempPictures(void);
	void createTempPictures(double minTemp, double maxTemp, bool force);

	// ホットスポット詳細画像関連
	void* loadPicture(int id, int pictureType, double pixelPerMeter);

	// (2020/01/21LEE) Detailのホットスポットの写真を保存する事とHomeで報告書に保存する写真のSIZEを探して赤の線表示する事をここで全部処理
	double saveHotspotPictureAlgorithm(int id, double viewRatio, int setmode);

	// (2020/02/07LEE) Minimapに報告書に出力する画像を表示
	bool canvas_SerchDetailhotspot(int Picnum);

	bool saveFilePathList(TCHAR *fileName);
	bool saveFilePath2List(TCHAR *fileName, TCHAR *hotspotFileName);

	void getHotspotOverallPosition();
	double getPictureRatio(int id, double pixelPerMeter);
	void gpsPosToPixel(double gpsX, double gpsY, int *x, int *y, double offsetX,
			double offsetY, double pixelPerMeter);
	void gpsPosToPixel(int id, int *x, int *y, double offsetX, double offsetY,
			double pixelPerMeter);
	void getPictureRect(int id, int pictureType, POINT *rect,
			double pixelPerMeter);

	void resetView(void);
	void showHotspotDetail(void);

	void pixelToGPSPos(int x, int y, double *gpsX, double *gpsY);

	// パネル情報を更新
	void updateInfo(void);

	void saveWholePictureSplit(int pictureType, TCHAR *splitFileName,
			RECT *rect);
	void trimConstitutionPicture(int id, double viewRatio);

	int getHotspotId(int x, int y);
	HotspotNumber getHotspotOnAdjustmentMode(int x, int y);
	void clearHotspotIdList(void);

	bool createPanel4Points(void);
	bool createPanel2Points(void);

	// イベントハンドラ
	static LRESULT CALLBACK handleEvent(HWND hWnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);

};

#ifdef _cplusplus
extern "C" {
#endif
extern void setLicenseKeyRegistered(bool Registered);
extern bool getLicenseKeyRegistered(void);
extern void setActivationResult(bool Activation);
extern bool getActivationResult(void);

extern void Initialize(HINSTANCE hInst);
extern void Finalize(void);

#ifdef _cplusplus
};
#endif

#endif /* MAINWINDOW_H_ */
