#include <math.h>
#include "surveycalc.h"

// 内部用関数
double factorial(int n);
double getMeridianArcLength(double latitude);

#define xCommon(i) (1.0 / factorial(i) * N * pow(cos(phi), i) * t * A[i / 2 - 1] * pow(delta_lambda, i))
#define yCommon(i) (1.0 / factorial(i) * N * pow(cos(phi), i) * B[(i - 1) / 2] * pow(delta_lambda, i))

// 赤道半径(長半径)
const double a = 6378137;

// 逆扁平率
#ifdef _USE_GRS80
// GRS80(計算式確認用)
const double F = 298.257222101;
#else // _USE_GRS80
// WGS84(デフォルト)
// GPSで使用されている地球楕円体
const double F = 298.257223563;
#endif

// 原点における縮尺係数
const double m0 = 0.9999;

/**
 * 階乗を計算する
 * @param[in] n 整数
 * @return 階乗
 */
inline double factorial(int n) {
	double result = 1.0;
	int k;

	for (k = 1; k <= n; k++) {
		result *= k;
	}
	return result;
}

/**
 * 平面直角座標から緯度・経度に変換する
 * @param[in] x X座標[m]
 * @param[in] y Y座標[m]
 * @param[in] phi0 基準点の緯度[rad]
 * @param[in] lambda0 基準点の経度[rad]
 * @param[in] phi1 緯度の最大値[rad]
 * @param[in] lambda1 経度の最小値[rad]
 * @param[in] phi 緯度[rad]
 * @param[in] lambda 経度[rad]
 * @todo 関数の戻り値に誤差が残っているかも?
 * @todo 緯度±90°/経度±180°をまたぐと正常に動作しない
 */
void xy2bl(double x, double y, double phi0, double lambda0, double phi1,
		double lambda1, double *phi, double *lambda) {
	const unsigned int MSB32 = 0x80000000;
	const double INT32_CNT = 4294967296.0;
	const double dphi = phi1 - phi0;
	const double dlambda = lambda1 - lambda0;
	unsigned int nphi = 0, nlambda = 0;
	for (unsigned int bit_phi = MSB32; bit_phi != 0; bit_phi >>= 1) {
		const double vphi = (nphi + bit_phi) * dphi / INT32_CNT + phi0;
		double vx, vy;
		ll2xy(vphi, lambda0, phi0, lambda0, &vx, &vy);
		if (vx <= x) {
			nphi += bit_phi;
		}
		*phi = vphi;
	}
	for (unsigned int bit_lambda = MSB32; bit_lambda != 0; bit_lambda >>= 1) {
		const double vlambda = (nlambda + bit_lambda) * dlambda / INT32_CNT
				+ lambda0;
		double vx, vy;
		ll2xy(*phi, vlambda, phi0, lambda0, &vx, &vy);
		if (vy <= y) {
			nlambda += bit_lambda;
		}
		*lambda = vlambda;
	}
}

/**
 * 緯度・経度から平面直角座標に変換する
 * @param[in] phi 緯度[rad]
 * @param[in] lambda 経度[rad]
 * @param[in] phi0 基準点の緯度[rad]
 * @param[in] lambda0 基準点の経度[rad]
 * @param[out] x X座標[m]
 * @param[out] y Y座標[m]
 * @todo 関数の戻り値に誤差が残っているかも?
 */
void ll2xy(double phi, double lambda, double phi0, double lambda0, double *x,
		double *y) {
	double A[4], B[4];

	const double e = sqrt(2 * F - 1) / F;
	const double W = sqrt(1 - pow(e * sin(phi), 2));
	const double e_dash = sqrt(2 * F - 1) / (F - 1);
	//const double M = a * (1 - pow(e, 2)) / pow(W, 3);
	const double N = a / W;
	const double eta = e_dash * cos(phi);
	const double t = tan(phi);
	const double delta_lambda = lambda - lambda0;

	A[0] = 1;
	A[1] = 5.0 - pow(t, 2) + 9 * pow(eta, 2) + 4.0 * pow(eta, 4);
	A[2] = -61.0 + 58.0 * pow(t, 2) - pow(t, 4) - 270.0 * pow(eta, 2)
			+ 330.0 * pow(t, 2) * pow(eta, 2);
	A[3] = -1385.0 + 3111.0 * pow(t, 2) - 543.0 * pow(t, 4) + pow(t, 6);
	*x = (getMeridianArcLength(phi) - getMeridianArcLength(phi0) + xCommon(2)
			+ xCommon(4) - xCommon(6) - xCommon(8)) * m0;

	B[0] = 1.0;
	B[1] = -1.0 + pow(t, 2) - pow(eta, 2);
	B[2] = -5.0 + 18.0 * pow(t, 2) - pow(t, 4) - 14.0 * pow(eta, 2)
			+ 58.0 * pow(t, 2) * pow(eta, 2);
	B[3] = -61.0 + 479.0 * pow(t, 2) - 179.0 * pow(t, 4) + pow(t, 6);
	*y = (yCommon(1) - yCommon(3) - yCommon(5) - yCommon(7)) * m0;
}

/**
 * 赤道からphiまでの子午線弧長を求める。
 * @param[in] latitude 緯度
 * @return 子午線弧長
 * @todo getMeridianArcLength関数の戻り値に誤差が残っているかも?
 */
double getMeridianArcLength(double latitude) {
	double A[9], B[10];

	const double e = sqrt(2 * F - 1) / F;

	A[0] = 1.0 + 3.0 / 4.0 * pow(e, 2) + 45.0 / 64.0 * pow(e, 4)
			+ 175.0 / 256.0 * pow(e, 6) + 11025.0 / 16384.0 * pow(e, 8)
			+ 43659.0 / 65536.0 * pow(e, 10) + 693693.0 / 1048576.0 * pow(e, 12)
			+ 19324305.0 / 29360128.0 * pow(e, 14)
			+ 4927697775.0 / 7516192768.0 * pow(e, 16);

	A[1] = 3.0 / 4.0 * pow(e, 2) + 15.0 / 16.0 * pow(e, 4)
			+ 525.0 / 512.0 * pow(e, 6) + 2205.0 / 2048.0 * pow(e, 8)
			+ 72765.0 / 65536.0 * pow(e, 10) + 297297.0 / 266144.0 * pow(e, 12)
			+ 135270135.0 / 117440512.0 * pow(e, 14)
			+ 547521975.0 / 469762048.0 * pow(e, 16);

	A[2] = 15.0 / 64.0 * pow(e, 4) + 105.0 / 256.0 * pow(e, 6)
			+ 2205.0 / 2048.0 * pow(e, 8) + 10395.0 / 16384.0 * pow(e, 10)
			+ 1486485.0 / 2097152.0 * pow(e, 12)
			+ 45090045.0 / 58720256.0 * pow(e, 14)
			+ 766530765.0 / 939524096.0 * pow(e, 16);

	A[3] = 35.0 / 512.0 * pow(e, 6) + 315.0 / 2048.0 * pow(e, 8)
			+ 31185.0 / 131072.0 * pow(e, 10) + 165165.0 / 524288.0 * pow(e, 12)
			+ 45090045.0 / 117440512.0 * pow(e, 14)
			+ 209053845.0 / 469762048.0 * pow(e, 16);

	A[4] = 315.0 / 16384.0 * pow(e, 8) + 3465.0 / 65536.0 * pow(e, 10)
			+ 99099.0 / 1048576.0 * pow(e, 12)
			+ 4099095.0 / 29360128.0 * pow(e, 14)
			+ 348423075.0 / 1879048192.0 * pow(e, 16);

	A[5] = 693.0 / 131072.0 * pow(e, 10) + 9009.0 / 524288.0 * pow(e, 12)
			+ 4099095.0 / 117440512.0 * pow(e, 14)
			+ 26801775.0 / 469762048.0 * pow(e, 16);

	A[6] = 3003.0 / 2097152.0 * pow(e, 12) + 315315.0 / 58720256.0 * pow(e, 14)
			+ 11486475.0 / 939524096.0 * pow(e, 16);

	A[7] = 45045.0 / 117440512.0 * pow(e, 14)
			+ 765765.0 / 469762048.0 * pow(e, 16);

	A[8] = 765765.0 / 7516192768.0 * pow(e, 16);

	B[0] = a * (1 - pow(e, 2));
	B[1] = B[0] * A[0];
	B[2] = B[0] * (-A[1] / 2.0);
	B[3] = B[0] * (A[2] / 4.0);
	B[4] = B[0] * (-A[3] / 6.0);
	B[5] = B[0] * (A[4] / 8.0);
	B[6] = B[0] * (-A[5] / 10.0);
	B[7] = B[0] * (A[6] / 12.0);
	B[8] = B[0] * (-A[7] / 14.0);
	B[9] = B[0] * (A[8] / 16.0);

	double sum = 0;
	for (int i = 2; i <= 9; i++) {
		sum += B[i] * sin(2 * (i - 1) * latitude);
	}

	return B[1] * latitude + sum;
}

#ifdef UNIT_TEST_LL2XY_MODULE
//-----------------------------------------------------------------------------
// モジュール単体テスト用
// ※関数の単体テストはデバッガを使ってください。
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

// 度からラジアンへの変換
#define toRad(deg) ((deg) * M_PI / 180.0)

//--------------------------------------------------------------
// 単体動作テスト
// 関数の入力引数と同じ順番でコマンド引数を入力すると、
// 変換後のXY座標値が出力する。
// 極座標系の大きさも出力する。
//--------------------------------------------------------------
int main(int argc, char** argv) {
	double x, y;
	double lat, lat0, lon, lon0;
	
	// ★暫定(GRS80)
	#ifdef _USE_GRS80
	printf("Use GRS80(Standard)\n");
	#else
	printf("Use WGS84(GPS)\n");
	#endif // _USE_GRS80
	
	if (argc != 5) {
		// コマンド引数の数が5でない場合は不正動作なの
		return 1;
	}
	
	lat = toRad(atof(argv[1]));
	lon = toRad(atof(argv[2]));
	lat0 = toRad(atof(argv[3]));
	lon0 = toRad(atof(argv[4]));
	ll2xy(lat, lon, lat0, lon0, &x, &y);
	
	// 直交座標系(X,Y)での表示
	printf("X[m]:%.9lg(%.10lf)\nY[m]:%.9lg(%.10lf)\n", x, x, y, y);
	// 極座標系(r,θ)での表示
	printf("Radius[m]:%.10lf\n", sqrt(x * x + y * y));
	return 0;
}
#endif // UNIT_TEST_LL2XY_MODULE
