/*
 * ResultData.h
 *
 *  Created on: 2016/02/01
 *      Author: PC-EFFECT-011
 */

#ifndef RESULTDATA_H_
#define RESULTDATA_H_

#include <vector>
#include <opencv2/core/types.hpp>
#include "CommonData.h"
#include "CalibrationData.h"
#include "FlightData.h"

/**
 * 結果データ
 */
class ResultData {
public:
	typedef enum {
		VISIBLE_LIGHT_IMAGE = 0, INFRARED_IMAGE, PANEL_IMAGE // (2017/5/30YM)輪郭画像用データを追加
	} PictureType;

	static const int NOT_ASSIGNED = -1;

	ResultData(const TCHAR *path);
	virtual ~ResultData(void);

	GPSPictureInfo operator[](unsigned int id);

	double getGPSHeight(unsigned int id);
	bool hasVisible(void);

	TCHAR* getFirstDateString(void);
	TCHAR* getLastDateString(void);
	TCHAR* getDataCountString(void);
	TCHAR* getFilePath(int id, int type);
	TCHAR* getVisibleFilePath(int id);
	TCHAR* getInfraredFilePath(int id);
	// (2017/5/30YM)輪郭画像ファイルパスを取得する関数を追加
	TCHAR* getPanelFilePath(int id);
	TCHAR* getPanelInfoPath(void);

	int getFilePath(int id, int type, TCHAR **str);

	TCHAR* getDataPath(void);
	TCHAR* getCachePath(void);
	TCHAR* getExePath(void);

	int getDataVersion(void);

	int getDataCount(void);

	void gpsPosToPixel(int id, int *x, int *y, double pixelPerMeter);
	void pixelToGPSPos(int x, int y, double *gpsX, double *gpsY,
			double pixelPerMeter);
	void gpsPosToPixel(double gpsX, double gpsY, int *x, int *y,
			double pixelPerMeter);
	double getPictureRatio(int pictureSize, double height, double viewAngle,
			double pixelPerMeter);
	double getCardinalDirection(Vector3D *mag);
	double getCardinalDirection(int id);
	double getGPSCardinalDirection(int id);

	bool enableCorrection(double speed, int points);
	void disableCorrection(void);

	bool isAvailable(void);

	float* getTempData(int id, int *width, int *height, struct tm *time);
	void getTempRange(void);
	float getTempMin(void);
	float getTempMax(void);
	float getTempAverage(void);
	// (2017/5/30YM)パネル上の温度データを返す関数を追加
	float getPanelTempMax(void);
	float getPanelTempMin(void);
	float getPanelTempAverage(void);

	// ---------------- ↓ ホットスポット管理 ↓ ----------------
	void detectHotspot(int id, float threshold);
	// (2019/09/25LEE) int threshold => float threshold に変更。
	int saveHotspot(void);
	int loadHotspot(void);
	int disableHotspotCount(int id);
	int getHotspotCount(int id);
	int getHotspotCountAll(void);
	void getHotspot(int dataId, int spotId, POINT *point);

	bool existHotspot(int id);

	void addHotspot(int id, int x, int y, int type);
	void delHotspot(int id, int hotspotId);
	int getHotspot(int id, int x, int y);

	void clearHotspot(void);

	int getHotspotId(int pictureNo, int pointNo);
	void setHotspotId(int dataId, int spotId, int id);

	int getHotspotType(int dataId, int spotId);
	void setHotspotType(int dataId, int spotId, int type);
	void enableHotspot(int dataId, int spotId);
	void disableHotspot(int dataId, int spotId);
	bool isHotspot(int dataId, int spotId);
	bool isEnabledHotspot(int dataId, int spotId);
	bool isHotspotCandidate(int dataId, int spotId);
	bool isDisabledHotspot(int dataId, int spotId);
	bool isKeypoint(int dataId, int spotId);
	bool existsLink(int picNo1, int picNo2);
	int getHotspotSum(void);
	TCHAR* getHotspotSumString(void);
	Vector2D getHotspotInRealWorld(HotspotNumber number);
	HotspotNumber getHotspotNumber(int id);
	int getUniqueHotspotCount(void);
	bool isUniqueHotspot(int pictureNo, int pointNo);
	void initHotspotOverallPosition(void);
	void getHotspotOverallPosition(int pictureNo, int pointNo);
	int getHotspotCountInArea(Vector2D *area, int count);
	HotspotDetail getHotspotDetail(int id);
	HotspotDetail* getHotspotListDetail(void);
	TCHAR* getPanelName(double x, double y);
	float getHotspotTemperature(HotspotNumber num);		//試しに。
	float* getHotspotTemperatures(int picNo);
	bool saveHotspotDetail(void);
	TCHAR* getHotspotDetail(void);
	bool saveHotspotDetail(const TCHAR *filePath);
	void getSelectHotspotId(int x, int y);
	TCHAR* getFileHotspotIdList(void);
	// (2017/6/2YM)追加データを保存する関数を追加
	bool saveAddData(const TCHAR *filePath);
	// ---------------- ↑ ホットスポット管理 ↑ ----------------

	// ---------------- ↓ 補正情報管理 ↓ ----------------
	void setPosition(int id, InternalPositionInfo *pos);
	void getPosition(int id, InternalPositionInfo *pos);
	void setDirection(int id, double direction);
	double getDirection(int id);
	void setBaseDirection(double direction);
	double getBaseDirection(void);
	void setHeight(int id, double height);
	double getHeight(int id);
	bool savePositionInfo(const TCHAR *filePath);
	bool loadPositionInfo(const TCHAR *filePath);
	bool savePositionInfo(void);
	bool loadPositionInfo(void);

	void setDefaultInternalPositionInfo(int id);
	void setDefaultDirection(int id);

	void getPolarCoordinates(int pictureNo, int pointNo, Vector2DPolar *point);
	void getHotspotRelativePos(int picNo1, int ptNo1, int picNo2, int ptNo2,
			double ratio, Vector2D *relativePos);
	bool adjustPosition(int base, int target);
	bool adjustPosition(int id);
	// ---------------- ↑ 補正情報管理 ↑ ----------------

	// ---------------- ↓ カメラ補正情報管理 ↓ ----------------
	// (2019/11/01LEE) 別々に管理するために配列で変更。int idの値を追加。
	void setPosition2(int id, CameraInfo *PictureIR);
	void getCameraInfo(int id, CameraInfo *cameraInfo);
	void setCameraOffset(double x, double y);
	void setCameraOffset2(int id, double x, double y); //(2019/11/05LEE)追加。
	void getCameraOffset(double *x, double *y);
	void getCameraOffset2(int id, double *x, double *y); //(2019/11/05LEE)追加。
	void setCameraRatio(double vertical, double horizontal);
	void setCameraRatio2(int id, double vertical, double horizontal); //(2019/11/05LEE)追加。
	void getCameraRatio(double *vertical, double *horizontal);
	void getCameraRatio2(int id, double *vertical, double *horizontal); //(2019/11/05LEE)追加。
	// (2016/12/20YM)可視赤外カメラ回転補正データセット関数追加
	// (2019/11/01LEE) 別々に管理するために配列で変更。int idの値を追加。
	void setCameraDirection(double direction);
	void setCameraDirection2(int id, double direction); //(2019/11/05LEE)追加。
	void getCameraDirection(double *direction);
	void getCameraDirection2(int id, double *direction); //(2019/11/05LEE)追加。
	void setCameraDatatype(int id, int datatype);  // (2019/11/08LEE)追加。
	void getCameraDatatype(int id, int *datatype); // (2019/11/08LEE)追加。
	void resetalldata(int id); // (2019/11/29/LEE) 追加。
	void getalldata(double *x, double *y, double *vertical, double *horizontal,
			double *direction); //(2019/11/29/LEE) 追加。
	void setalldata(int id, double x, double y, double vertical,
			double horizontal, double direction); //(2019/12/03/LEE) 追加。
	//　↑ここまで追加
	bool saveCameraInfo(const TCHAR *filePath);
	bool saveCameraInfo2(const TCHAR *filePath); //(2019/11/05LEE)追加。
	bool loadCameraInfo(const TCHAR *filePath);
	bool loadCameraInfo2(const TCHAR *filePath); //(2019/11/05LEE)追加。
	// ---------------- ↑ カメラ補正情報管理 ↑ ----------------

	// ---------------- ↓ カメラ情報管理 ↓ ----------------
	int getInfraredWidth(void);
	int getInfraredHeight(void);
	int getVisibleWidth(void);
	int getVisibleHeight(void);
	int getPictureWidth(int type);
	int getPictureHeight(int type);
	double getViewAngle(void);
	// ---------------- ↑ カメラ情報管理 ↑ ----------------

	static double getPixelPerMeter(int pictureSize, double viewAngle,
			double height);
	static double getMeterPerPixel(int pictureSize, double viewAngle,
			double height);
	static double meterToPixel(int pictureSize, double viewAngle, double height,
			double distance);
	static double pixelToMeter(int pictureSize, double viewAngle, double height,
			double distance);

	double meterToPixel(int id, double distance);
	double pixelToMeter(int id, double distance);

	void getXY(int id, double *x, double *y);

	int getAroundPictureId(int id, double angle, double error);
	int getAroundPictureId(int id, int directionId);
	bool isAroundPicture(int id, int target);

	TCHAR* getHotspotLinkFileName(void);
	TCHAR* getCameraInfoFileName(void);

	void getDataAreaSize2(double *west, double *south, double *east,
			double *north);

	int getAllHotspotCount(void);
	Vector2D getAllHotspotPos(int id);

	// (2017/3/3YM)ホットスポットNo取得変数を追加
	HotspotNumber getAllHotspotNo(int id);
	//　↑ここまで追加

	TCHAR* getfileNameIr(int id);
	TCHAR* getfileNameVb(int id);
	TCHAR* selcetIR(void);				//(2020/01/20LEE) 追加
	TCHAR* getfileWholeIr(void);
	TCHAR* getfileWholeVb(void);

	TCHAR* getfilePath(void);

	bool existWholePicture(int pictureType);
	TCHAR* getWholePictureFileName(int pictureType);
	bool loadWholePicture(TCHAR *filePath);

	TCHAR* getFlightDataFileName(void);

	bool saveTemperaturePicture(int id, double min, double max);
	bool savePanelPicture(int id);

	void saveFlightData(void);

	// (2017/4/4YM)ストリングバッファを取得するパブリック関数を追加
	TCHAR* getStringBufferPublic(void);
	// (2017/4/4YM)画像位置を初期化する関数を追加
	void ResetDefaultInternalPositionInfo(int id);

	// (2017/5/25YM)温度データファイル名を返す関数を追加
	TCHAR* getTempDataFileName(void);
	// (2017/5/25YM)温度データを読み込む関数を追加
	int loadTempdata(void);
	// (2017/5/25YM)温度データを読み込む関数を追加
	int loadTempdata(TCHAR *filePath);
	// (2017/5/25YM)最高最低平均温度データをセットする関数を追加
	void setTemp(void);
	// (2017/5/29YM)パネル平均温度を保存する処理を追加
	int savePanelTemp(TCHAR *filePath);
	//　(2017/6/14YM)ホットスポットデータ変数クリアチェック関数追加
	void CheckHSClearData(void);
	/**
	 * パネル検出モードを設定する
	 * @param mode パネル検出モード。
	 * trueなら検出する、falseなら検出しない
	 */
	void setPanelDetection(bool mode);
	/**
	 * パネル検出モードを取得する
	 * @returns パネル検出モード。
	 * trueなら検出する、falseなら検出しない
	 */
	bool getPanelDetection(void);
	// (2017/7/31YM)ホットスポット温度を返す関数を追加
	float getHotspotTemp(int id);
	// (2017/7/31YM)パネル平均温度を返す関数を追加
	float getPanelAveTemp(int id);
	// (2017/7/31YM)全ホットスポット№を返す関数を追加
	int getHotspotID(int dataID, int spotID);

	// (2020/01/15LEE) ERROR Num return
	bool getErrorcheck();
	int getpiccount();
	int getpicnum(int i);
	int getErrornumber(int picnum);

	bool isPanelDetected();
	void updatePanelDetected();

	int errorpicnum[1000];

	void setPanels(int id, const std::vector<std::vector<cv::Point> > &panels);
	bool getPanels(int id, std::vector<std::vector<cv::Point> > &panels);
protected:
	void setStringBufferSize(unsigned int size);
	TCHAR* getStringBuffer(void);
	void getXY(double latitude, double longitude, double *x, double *y);

	// ---------------- ↓ ホットスポット管理 ↓ ----------------
	TCHAR* getHotspotFileName(void);
	int detectHotspotImpl(int id, POINT **hotspots, float threshold);
	// (2019/09/25LEE) int threshold => float threshold に変更。
	int saveHotspot(TCHAR *fileName);
	int loadHotspot(TCHAR *fileName);
	// ---------------- ↑ ホットスポット管理 ↑ ----------------

private:
	TCHAR* getCurrentDateString(void);
	TCHAR* getDateString(struct tm *date);
	void sortData(double latitude, double longitude);

	int getInfraredDataPath(int id, TCHAR **str);

	bool available;
	int piccount;
	int picnumbering;
	//(2020/01/15LEE) GPS Data Error Check

	bool Errornumcheck;

	std::vector<GPSPictureInfo> data;

	// ---------------- ↓ ホットスポット管理 ↓ ----------------
	std::vector<HotspotList> hotspots;
	std::vector<Vector2D> allHotspot;
	std::vector<HotspotNumber> allHotspotNo;
	std::vector<float> allHotspotTemp;				//温度
	typedef std::vector<int> allHotspotCheckedData;
	allHotspotCheckedData allHotspotChecked;
	// ---------------- ↑ ホットスポット管理 ↑ ----------------

	// ---------------- ↓ カメラ補正情報管理 ↓ ----------------
	CameraInfo cameraInfo; // (2019/11/01LEE) 別々に管理するために配列で変更。
	CameraInfo resetdata; // (2019/11/29LEE) For reset, データを保存。
	std::vector<CameraInfo> picturedata;
	std::vector<CameraInfo> readpicturedata;
	PictureSize pictureSize;

	// ---------------- ↑ 補正情報管理 ↑ ----------------

	std::vector<InternalPositionInfo> posData;

	double baseDirection;

	double gpsLeft;
	double gpsBottom;
	double gpsRight;
	double gpsTop;

	TCHAR *stringBuffer;
	unsigned int stringBufferSize;

	CalibrationData *calibrationData;
	FlightData *flightDetail;
	double flightSpeed;
	int correctPoints;

	TCHAR *dataPath;

	TCHAR *cachePath;

	TCHAR *exePath;

	/** データバージョン */
	int dataVersion;
	float maxTemp;
	float minTemp;
	float aveTemp;
	float PNmaxTemp;				// (2017/5/30YM)パネル全体の最高温度
	float PNminTemp;				// (2017/5/30YM)パネル全体の最低温度
	float PNaveTemp;				// (2017/5/30YM)パネル全体の平均温度
	float PanelaveTemp[9999];		// (2017/5/24YM)パネル毎の平均温度データを追加
	float PanelmaxTemp[9999];		// (2017/5/25YM)パネル毎の最高温度データを追加
	float PanelminTemp[9999];		// (2017/5/25YM)パネル毎の最低温度データを追加

	/** パネル検出モード */
	bool panelDetection;

	/** パネル検出済み */
	bool panelIsDetected;
};

#endif /* RESULTDATA_H_ */
