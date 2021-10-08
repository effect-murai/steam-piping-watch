/*
 * PanelDetector.cpp
 *
 *  Created on: 2015/12/22
 *      Author: PC-EFFECT-012
 */

#include "PanelDetector.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <math.h>
#include <tchar.h>
#include <stdio.h>

#include "app.h"
#include "MainWindow.h"
#include "StringUtils.h"
#include "Graphics.h"
#include "ResultData.h"

namespace PanelDetector {
using namespace ::cv;
using namespace ::std;

#define N 9

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
inline double angle(Point pt1, Point pt2, Point pt0) {
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1 * dx2 + dy1 * dy2)
			/ sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2) + 1e-10);
}

inline double angle2(Point pt1, Point pt2) {
	double dx1 = pt2.x - pt1.x;
	double dy1 = pt2.y - pt1.y;
	return atan2(dy1, dx1);
}

/**
 * returns sequence of squares detected on the image.
 * the sequence is stored in the specified memory storage
 */
void findSquares(const Mat *image, vector<vector<Point> > *squares) {
	Mat pyr, timg, gray0(image->size(), CV_8U), gray;
	// 縮小・拡大でノイズを除去する
	pyrDown(*image, pyr, cv::Size(image->cols / 2, image->rows / 2));
	pyrUp(pyr, timg, image->size());

	vector<vector<Point> > contours;

	// 各カラーチャンネルごとに矩形を検出する
	for (int color = 0; color < 3; color++) {
		int ch[] = { color, 0 };
		mixChannels(&timg, 1, &gray0, 1, ch, 1);

		// 複数の閾値レベルで矩形を検出する
		for (int level = 0; level < N; level++) {
			if (level == 0) {
				// 閾値レベルが0のとき
				cv::cvtColor(*image, gray, CV_BGR2GRAY);
				cv::adaptiveThreshold(gray, gray, 255,
						cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 7,
						8);
			} else {
				// 閾値レベルが0以外のとき
				gray = gray0 >= (level + 1) * 255 / N;
			}

			// find contours and store them all as a list
			findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

			vector<Point> approx;

			// test each contour
			for (size_t i = 0; i < contours.size(); i++) {
				// approximate contour with accuracy proportional
				// to the contour perimeter
				approxPolyDP(Mat(contours[i]), approx,
						arcLength(Mat(contours[i]), true) * 0.015, true);

				// square contours should have 4 vertices after approximation
				// relatively large area (to filter out noisy contours)
				// and be convex.
				// Note: absolute value of an area is used because
				// area may be positive or negative - in accordance with the
				// contour orientation
				if (approx.size() == 4 && fabs(contourArea(Mat(approx))) > 30
						&& isContourConvex(Mat(approx))) {
					double maxCosine = 0;

					for (int j = 2; j < 5; j++) {
						// find the maximum cosine of the angle between joint edges
						double cosine = fabs(
								angle(approx[j % 4], approx[j - 2],
										approx[j - 1]));
						maxCosine = MAX(maxCosine, cosine);
					}
					// if cosines of all angles are small
					// (all angles are ~90 degree) then write quandrange
					// vertices to resultant sequence
					if (maxCosine < 0.3) {
						double s = contourArea(approx);
						if ((s / (image->rows * image->cols) > 0.8)
								|| (s < 100)) {
							continue;
						}
						// if cosines of all angles are small
						// (all angles are ~90 degree) then write quandrange
						// vertices to resultant sequence
						if (maxCosine < 0.3) {
							double s = contourArea(approx);
							if ((s / (image->rows * image->cols) > 0.8)
									|| (s < 100)) {
								continue;
							}
							double ath = 5 * M_PI / 180;
							if (fabs(
									angle2(approx[0], approx[1])
											- angle2(approx[3], approx[2]))
									< ath
									|| fabs(
											angle2(approx[1], approx[2])
													- angle2(approx[0],
															approx[3])) < ath) {
								squares->push_back(approx);
							}
						}
					}
				}
			}
		}
	}
}

void findPanels(const Mat *image, vector<vector<Point> > *panelArea,
		vector<vector<Point> > *panels) {
	if (panelArea != NULL) {
		panelArea->clear();
	}
	if (panels != NULL) {
		panels->clear();
	}

	vector<vector<Point> > squares;
	findSquares(image, &squares);

	// 図形サイズで大まかに分ける
	vector<int> sizeMap;
	for (size_t i = 0; i < squares.size(); i++) {
		double size = contourArea(squares[i]);
		int index = (int) size / 64;
		if (index >= 256) {
			index = 255;
		}
		sizeMap.push_back(index);
	}

	// ヒストグラムを作成する
	int sizeHist[256] = { 0 };
	for (size_t i = 0; i < squares.size(); i++) {
		int index = sizeMap[i];
		sizeHist[index]++;
	}

	// 最も出現頻度の高いサイズを調べる
	int sizeMax = 0;
	for (int i = 1; i < 255; i++) {
		if (sizeHist[sizeMax] < sizeHist[i]) {
			sizeMax = i;
		}
	}

	// 重複している図形を結合する
	int invalid[squares.size()];
	memset(invalid, 0, squares.size() * sizeof(int));
	for (size_t i = 0; i < squares.size(); i++) {
		if (invalid[i] == 0) {
			vector<Point> figure = squares[i];
			// 太陽電池のセルかどうかを判定する
			int isCell = ((fabs(sizeMax - sizeMap[i]) < 6) ? 1 : 0);

			int cellCount = 0;
			for (size_t j = 0; j < squares.size(); j++) {
				if ((invalid[j] == 1) || (i == j)) {
					continue;
				}
				int _isCell = (fabs(sizeMax - sizeMap[j]) < 6) ? 1 : 0;
				int including = 1;

				for (size_t k = 0; k < squares[j].size(); k++) {
					if (cv::pointPolygonTest(figure, squares[j][k], false)
							< 0) {
						// 図形外にある場合
						including = 0;
						break;
					}
				}
				if (including == 1) {
					if (_isCell == 1 && isCell == 0) {
						cellCount++;
					} else {
						invalid[j] = 1;
					}
				}
			}
			// 内部にセルがない場合
			if (isCell == 0 && cellCount == 0) {
				invalid[i] = 1;
			}
		}
	}

	for (size_t i = 0; i < squares.size(); i++) {
		if (invalid[i] == 1) {
			continue;
		}

		// サイズごとに
		int size = sizeMap[i];
		if (fabs(sizeMax - size) < 6) {
			// 太陽光パネル
			if (panels != NULL) {
				panels->push_back(squares[i]);
			}
		} else {
			// 太陽光パネルの範囲
			if (panelArea != NULL) {
				panelArea->push_back(squares[i]);
			}
		}
	}
}

double getAngle(vector<vector<Point> > *panels) {
	double angle = 0, count = 0;
	vector<vector<Point> >::iterator panel;

	for (panel = panels->begin(); panel != panels->end(); panel++) {
		int vertexCount = (int) panel->size();
		for (int k = 1; k < vertexCount - 1; k++) {
			double _angle = angle2((*panel)[k - 1], (*panel)[k]);
			if (fabs(_angle) < M_PI / 4) {
				angle += _angle;
				count++;
			}
		}
	}
	if (count != 0) {
		return (angle / count);
	} else {
		return 0;
	}
}

double getPanelAngle(LPCTSTR fileName) {
	Mat image = Graphics::loadCVImage(fileName);
	if (image.empty()) {
		return 0.0;
	}
	vector<vector<Point> > panelArea, panels;

	findPanels(&image, &panelArea, &panels);
	return getAngle(&panels);
}

}
