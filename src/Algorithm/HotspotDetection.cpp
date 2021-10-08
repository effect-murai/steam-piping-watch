/*
 * HotspotDetection.cpp
 *
 *  Created on: 2016/08/10
 *      Author: PC-EFFECT-012
 */

#include "Graphics.h"
#include "ResultData.h"
#include "PanelDetector.h"
#include "Matrix.h"
#include <opencv2/imgproc/imgproc.hpp>

#include <stdio.h>
#include "MainWindow.h"
#include "FileUtils.h"
#include "StringUtils.h"
#include "SubWindows/ProgressBar.h"
#include "TemperaturePicture.h"

// 度からラジアンへの変換
#define toRad(deg) ((deg) * M_PI / 180.0)

inline int matGet(cv::Mat *src, int x, int y) {
	return static_cast<int>(src->data[y * src->step + x * src->elemSize()]);
}

inline void savePanelImage(ResultData *result, int id, cv::Mat &mask) {
	//　輪郭画像を保存
	LPTSTR fileName;
	result->getFilePath(id, ResultData::PANEL_IMAGE, &fileName);
	Graphics::saveCVImage(fileName, mask);
	delete fileName;
}

inline cv::Mat loadPanelImage(ResultData *result, int id) {
	// 画像を読み込む
	LPTSTR fileName;
	result->getFilePath(id, ResultData::PANEL_IMAGE, &fileName);
	cv::Mat imagePanel = Graphics::loadCVImage(fileName);
	delete fileName;
	return imagePanel;
}

inline cv::Mat createIrImage(ResultData *result, int id, float *tempData,
		int width, int height) {
	// 赤外線画像をグレースケールで作成する。
	Gdiplus::Bitmap *bitmap = TemperaturePicture::create(tempData, width,
			height, result->getTempMin(), result->getTempMax(),
			TemperaturePicture::GRAY_SCALE);
	cv::Mat imageIr = Graphics::toCVMatrix(bitmap);
	delete bitmap;
	// 一度JPEGとして保存した後、再度読み込む。
	// (従来の動きと結果が大きく変わるため。)
	savePanelImage(result, id, imageIr);
	return loadPanelImage(result, id);
}

/**
 * ホットスポット検出@n
 * 画像処理を行いエッジ画像を出力する。エッジ部分を黒で表現した2値ビットマップ画像@n
 * CPUが処理するため極めて低速である。
 * @param[in] id 飛行データ番号
 * @param[out] hotspots ホットスポットデータ配列
 * @paran[in] threshold 閾値
 * @return ホットスポット個数
 */
int ResultData::detectHotspotImpl(int id, POINT **hotspots, float threshold) {
	std::vector<POINT> hotSpots;
	int width, height;

	// ---------------------- 温度データの取得 --------------------------
	float *_tempData = getTempData(id, &width, &height, NULL);

	if (_tempData == NULL) {
		return 0;
	}

	bool isPanelDetected = false;
	cv::Mat mask;
	if (this->getPanelDetection()) {
		// グレースケールの赤外線画像を作成する。
		cv::Mat imageIr = createIrImage(this, id, _tempData, width, height);

		// パネル以外の部分をマスクする
		cv::Mat imagePanel(imageIr.size(), CV_8UC3);
		mask = cv::Mat(imageIr.size(), CV_8UC1, cv::Scalar(0));
		imageIr.copyTo(imagePanel);

		std::vector<std::vector<cv::Point> > panels;
		PanelDetector::findSolarPanels(imageIr, panels);

		// 検出したパネルを保存する。
		setPanels(id, panels);

		for (std::vector<std::vector<cv::Point> >::iterator panel =
				panels.begin(); panel != panels.end(); panel++) {
			const cv::Point *p = panel->data();
			int n = (int) panel->size();
			fillPoly(imagePanel, &p, &n, 1, cv::Scalar(0, 0, 255));
			fillPoly(mask, &p, &n, 1, cv::Scalar(255));
		}

		// パネル画像を保存する
		savePanelPicture(id);

		isPanelDetected = true;
	} else {
		// 保存されたパネルデータを取得する。
		std::vector<std::vector<cv::Point> > panels;

		// 検出したパネルを取得する。
		if (getPanels(id, panels)) {
			// パネル検出済みの場合
			// パネル以外の部分をマスクする。
			mask = cv::Mat(cv::Size(width, height), CV_8UC1, cv::Scalar(0));
			for (std::vector<std::vector<cv::Point> >::iterator panel =
					panels.begin(); panel != panels.end(); panel++) {
				const cv::Point *p = panel->data();
				int n = (int) panel->size();
				fillPoly(mask, &p, &n, 1, cv::Scalar(255));
			}

			isPanelDetected = true;
		} else {
			mask = cv::Mat(cv::Size(width, height), CV_8UC1, cv::Scalar(255));
		}
	}

	// 検出対象範囲データを宣言
	Matrix<char> inPolygonMap(width, height);
	inPolygonMap.clear();

	// 最高温度，最低温度を求める変数宣言
	float maxTemp = -INFINITY;
	float minTemp = INFINITY;

	// 平均値を求める
	int count = 0;
	double average = 0;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (matGet(&mask, x, y) != 0) {
				// 多角形の範囲内の場合、合計を求める
				inPolygonMap.set(x, y, 1);
				// 温度を変数に設定する
				float temp = _tempData[x + y * width];
				average += temp;
				count++;
				// 最高温度，最低温度チェック
				if (maxTemp < temp) {
					maxTemp = temp;
				}
				if (minTemp > temp) {
					minTemp = temp;
				}
			} else {
				inPolygonMap.set(x, y, 0);
			}
		}
	}
	if (count > 0) {
		average /= count;
	} else {
		average = NAN;
		maxTemp = NAN;
		minTemp = NAN;
	}

	// 平均温度を画像別に保持する
	PanelaveTemp[id] = average;
	// 最高温度を画像別に保持する
	PanelmaxTemp[id] = maxTemp;
	// 最低温度を画像別に保持する
	PanelminTemp[id] = minTemp;

	//　閾値を0～100℃で0.5きざみの計算に変更
	const double th = average + (threshold * (100 - 0) / 200);

	// 全画面に対し閾値より20℃高い箇所を検出
	const double th1 = threshold + 20;
	double xa = 0, ya = 0;
	int detected = 0;

	// パネル検出時は全画面検出はしない
	if (!isPanelDetected) {
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				if (_tempData[x + y * width] > th1) {
					int lcount = 0;
					int ok = 1;
					// 太陽光反射除外処理
					for (int k = 0; k < 2; k++) {
						int dist = 10 - k;
						for (int j = -dist; j <= dist; j++) {
							int _y = y + j;
							if (_y < 0 || _y >= height) {
								continue;
							}
							for (int i = -dist; i <= dist; i++) {
								if (i <= -dist || i == dist || j == -dist
										|| j == dist) {
									int _x = x + i;
									if (_x < 0 || _x >= width) {
										continue;
									}
									if (_tempData[_x + _y * width] > th1) {
										// 一応枠外もチェックする
										lcount++;
									}
								}
							}
						}
					}
					if (lcount < 5 && ok == 1) {
						detected = 1;
						xa = x;
						ya = y;
					}
					if (detected == 1) {
						if (xa != 0 && ya != 0) {
							// ホットスポットの位置を保存する
							POINT pt;
							pt.x = xa;
							pt.y = ya;
							// 近くにホットスポットがないかチェックする
							bool sameHotspot = false;
							for (std::vector<POINT>::iterator l =
									hotSpots.begin(); l != hotSpots.end();
									l++) {
								int dx = l->x - pt.x;
								int dy = l->y - pt.y;
								// ホットスポット間の距離が32ピクセル未満の場合
								// 同じホットスポットと判定する。
								if (dx * dx + dy * dy < 1024) {
									sameHotspot = true;
									break;
								}
							}

							// 近くにホットスポットがない場合のみ追加する
							if (sameHotspot == false) {
								hotSpots.push_back(pt);
							}
						}
						detected = 0;
					}
				}
			}
		}
	}

	xa = 0;
	ya = 0;

	detected = 0;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if ((_tempData[x + y * width] > th) && (matGet(&mask, x, y) > 0)) {
				int lcount = 0;
				int ok = 1;
				// 太陽光反射除外処理
				for (int k = 0; k < 2; k++) {
					int dist = 10 - k;
					for (int j = -dist; j <= dist; j++) {
						int _y = y + j;
						if (_y < 0 || _y >= height) {
							continue;
						}
						for (int i = -dist; i <= dist; i++) {
							if (i <= -dist || i == dist || j == -dist
									|| j == dist) {
								int _x = x + i;
								if (_x < 0 || _x >= width) {
									continue;
								}
								// 太陽光判別温度を固定
								if (_tempData[_x + _y * width] > th) {
									// 一応枠外もチェックする
									lcount++;
								}
							}
						}
					}
				}
				if (lcount < 5 && ok == 1) {
					detected = 1;
					xa = x;
					ya = y;
				}
				if (detected == 1) {
					if (xa != 0 && ya != 0) {
						// ホットスポットの位置を保存する
						POINT pt;
						pt.x = xa;
						pt.y = ya;
						// 近くにホットスポットがないかチェックする
						bool sameHotspot = false;
						for (std::vector<POINT>::iterator l = hotSpots.begin();
								l != hotSpots.end(); l++) {
							int dx = l->x - pt.x;
							int dy = l->y - pt.y;
							// ホットスポット間の距離が32ピクセル未満の場合
							// 同じホットスポットと判定する。
							if (dx * dx + dy * dy < 1024) {
								sameHotspot = true;
								break;
							}
						}
						// パネルの縁でないかチェック
						for (int yf = -2; yf <= 2; yf++) {
							for (int xf = -2; xf <= 2; xf++) {
								int xfx = xa + xf;
								int yfy = ya + yf;
								if (xfx < 0 || xfx > (width - 1) || yfy < 0
										|| yfy > (height - 1)) {
									continue;
								} else {
									if (matGet(&mask, xfx, yfy) <= 0) {
										sameHotspot = true;
										break;
									}
								}
							}
						}
						// 近くにホットスポットがない場合のみ追加する
						if (sameHotspot == false) {
							hotSpots.push_back(pt);
						}
					}
					detected = 0;
				}
			}
		}
	}

	// 後片付け
	delete _tempData;

	// 関数の入出力仕様にしたがって出力する
	if (hotSpots.size() > 0) {
		// メモリを再確保して配列に変換する
		*hotspots = (POINT*) malloc(hotSpots.size() * sizeof(POINT));

		size_t i = 0;
		for (std::vector<POINT>::iterator hotSpot = hotSpots.begin();
				hotSpot != hotSpots.end(); hotSpot++, i++) {
			(*hotspots)[i] = *hotSpot;
		}
	}
	return hotSpots.size();
}
