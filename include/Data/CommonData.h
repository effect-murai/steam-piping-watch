/*
 * CommonData.h
 *
 *  Created on: 2016/02/01
 *      Author: PC-EFFECT-011
 */

#ifndef COMMONDATA_H_
#define COMMONDATA_H_

#include <windows.h>
#include <time.h>
#include <math.h>
#include <vector>

//------------------------------------------------------------------------------
// 定数値
//------------------------------------------------------------------------------

typedef enum {
	GPS_NO_FIX = 0, GPS_2D_FIX = 2, GPS_3D_FIX = 3
} GPSFixStatus;

enum {
	HOTSPOT_TYPE_UNKNOWN = -1,
	HOTSPOT_TYPE_DISABLED = 0,
	HOTSPOT_TYPE_CANDIDATE,
	HOTSPOT_TYPE_ENABLED,
	HOTSPOT_TYPE_KEYPOINT
};

// GPS ERROR NUMBERING
enum {

	GPS_ERROR_LONGITUDE = 1,
	GPS_ERROR_LATITUDE = 2,
	GPS_ERROR_HEIGHT,
	GPS_ERROR_LONGITUDE_LATITUDE,
	GPS_ERROR_LONGITUDE_HEIGHT,
	GPS_ERROR_LATITUDE_HEIGHT,
	GPS_ERROR_ALL
};

//------------------------------------------------------------------------------
// 構造体
//------------------------------------------------------------------------------

/**
 * 3次元ベクトル
 * 他のライブラリにあれば定義不要
 */
typedef struct {
	double x;
	double y;
	double z;
} Vector3D;

typedef struct {
	double x;
	double y;
} Vector2D;

typedef struct {
	double r;
	double arg;
} Vector2DPolar;

typedef struct {
	Vector2D offset;
	Vector2D ratio;
	double direction;		// (2016/12/20YM)回転補正データの型をint→doubleへ変更
	int datatype; // (2019/11/08LEE) 全体と個別を区分する事。0=>全体、1=>個別。
} CameraInfo;

typedef struct {
	int width;
	int height;
} Size;

typedef struct {
	Size infrared;
	Size visible;
} PictureSize;

/**
 * GPS情報
 */
typedef struct {
	/** Time of Weekの整数値 */
	int iTOW;
	/** 緯度 */
	double latitude;
	/** 経度 */
	double longitude;
	/** 高度 */
	double height;
	/** GPS補正の状態 */
	int gpsOk;
	/** GPS補正の状態(詳細) */
	int gpsFix;
} GPSInfo;

/**
 * センサー情報と画像ファイルのリンク情報
 */
typedef struct {
	/** 加速度センサー情報 */
	Vector3D accel;
	/** ジャイロセンサー情報 */
	Vector3D gyro;
	/** 磁気センサー情報 */
	Vector3D mag;
	/** GPSセンサー情報 */
	GPSInfo gps;
	/** 撮影時刻 */
	struct tm time;
	/** 撮影時刻 */
	int tm_usec;
	/** 可視光画像ファイル名(8文字) */
	TCHAR fileName_8[8];
	/** 赤外線画像ファイル名(8文字) */
	TCHAR fileName_ir_8[8];
	// (2017/5/30YM) 輪郭画像ファイル名(8文字)追加
	TCHAR fileName_pn_8[8];
} GPSPictureInfo;

typedef struct {
	/**	X座標[m] */
	double x;
	/** Y座標[m] */
	double y;
	/** 方位[rad] */
	double direction;
	/** 高度[m] */
	double height;
} InternalPositionInfo;

typedef struct {
	/** ポットスポットX座標(画像上) */
	int x;
	/** ポットスポットY座標(画像上) */
	int y;
	/** ポットスポットの種別 */
	int type;
	/** ポットスポットの固有番号 */
	int id;
} Hotspot;

typedef struct {
	int pictureNo;
	int pointNo;
} HotspotNumber;

//報告書２用
typedef struct {
	TCHAR *panelName; //パネル管理番号
	double longitude; // 経度 (x軸)
	double latitude; // 緯度 (y軸)
	double temperature; //温度
	double temperatureAverage; // 平均温度
	TCHAR *fileNameIr; //熱画像
	TCHAR *fileNameVb; //可視画像
	double panelmaxtemp;	// (2017/6/2YM)パネル最高温度
	double panelavetemp;	// (2017/5/31YM)パネル平均温度
	double thresholdtemp;	// (2017/5/31YM)閾値温度
} HotspotDetail;

typedef struct {
	int id;
	double viewRatio;
	int picnum;	// (2020/01/14LEE) Detail ホットスポットの更新のためにpicnumとhotnumを追加
	int hotnum;	// (2020/01/14LEE) Detail ホットスポットの更新のためにpicnumとhotnumを追加
	int bestpicnum;				// (2020/02/07LEE) Minimapに表示する事は点だけ
} HotspotIdList;

typedef struct {
	int id;
	double viewRatio;
} HotspotIdListbackup;

typedef struct {
	Vector2D pt1;
	Vector2D pt2;
} LineSegment;

//------------------------------------------------------------------------------
// マクロ
//------------------------------------------------------------------------------

typedef std::vector<Hotspot> HotspotList;
typedef std::vector<HotspotNumber> HotspotLink;
typedef std::vector<Vector2D> HotSpotArea;

#ifndef M_PI
#	define M_PI 3.14159265358979323846
#endif

// 度からラジアンへの変換
#define toRad(deg) ((deg) * M_PI / 180.0)

// ラジアンから度への変換
#define toDeg(rad) ((rad) * 180.0 / M_PI)

//------------------------------------------------------------------------------
// Global Functions
//------------------------------------------------------------------------------

// 各種センサー情報の加算
GPSPictureInfo& operator+=(GPSPictureInfo &x, const GPSPictureInfo &y);
GPSPictureInfo* operator+=(GPSPictureInfo *const x, const GPSPictureInfo &y);

// 各種センサー情報の除算
GPSPictureInfo operator/(const GPSPictureInfo x, const int y);

/**
 * センサー情報ファイルの内容を解析する。
 * @param[in] lineBuf 解析対象の文字列バッファへのポインタ。文字列はNULL文字で終わる必要がある。
 * @param[out,ref] inf センサー情報を保存する変数
 * @return 解析の成否(true:成功/false:失敗)
 */
bool parseSensorInfo(const char *buffer, GPSPictureInfo &inf);

#endif /* COMMONDATA_H_ */
