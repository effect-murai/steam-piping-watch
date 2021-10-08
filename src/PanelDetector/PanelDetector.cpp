/*
 * PanelDetector.cpp
 *
 *  Created on: 2020-04-08
 *      Author: k.tasaki
 */

#include "PanelDetector.h"

#include "app.h"
#include <windows.h>
#include <filesystem>

#include "Core/dsolar.h"
#include "Core/FindPanel.h"

using namespace std;
using namespace cv;

inline Mat rotate(Mat src, double angle) {
	Mat dst;
	Point2f pt(0, 0);
	Mat r = getRotationMatrix2D(pt, angle, 1.0);
	warpAffine(src, dst, r, Size(src.cols, src.rows));
	return dst;
}

inline void saveImageWithRects(const Mat &image, vector<RectF> rects,
		string path) {
	float fontScale, dis;
	for (size_t i = 0; i < rects.size(); i++) {
		line(image, Point(rects[i].x1, rects[i].y1),
				Point(rects[i].x2, rects[i].y2), Scalar(0, 255, 255), 1);
		line(image, Point(rects[i].x2, rects[i].y2),
				Point(rects[i].x3, rects[i].y3), Scalar(0, 255, 255), 1);
		line(image, Point(rects[i].x3, rects[i].y3),
				Point(rects[i].x4, rects[i].y4), Scalar(0, 255, 255), 1);
		line(image, Point(rects[i].x4, rects[i].y4),
				Point(rects[i].x1, rects[i].y1), Scalar(0, 255, 255), 1);

		dis = distance(rects[i].x1, rects[i].y1, rects[i].x2, rects[i].y2);
		fontScale = dis / 55;
		putText(image, (String() + (i + 1)),
				Point(rects[i].x1 + dis / 5, rects[i].y1 + dis / 3),
				FONT_HERSHEY_COMPLEX_SMALL, fontScale, Scalar(0, 0, 255), 0.5,
				CV_AA);
	}

	imwrite(path, image);
}

namespace PanelDetector {

void findSolarPanels(const Mat &imageOrigin, vector<vector<Point> > &panels,
		const int detectMode) {
	Mat imageGray;
	cvtColor(imageOrigin, imageGray, CV_BGR2GRAY);

	// loop qua dong rect, validate
	Ptr<CLAHE> clahe0 = createCLAHE(4.0);
	Mat imageGray2;
	clahe0->apply(imageGray, imageGray2);

	// Reduce noise with a kernel 3x3
	Mat edges;
	blur(imageGray2, edges, Size(3, 3));

#	ifdef PANEL_DETECTOR_DEBUG
	imwrite("high_contrast.png", edges);
#	endif

	vector<RectF> rects;

	if (detectMode == 1) {

		////------------------  Danh cho Viet  -----------------------

		vector<Vec4f> linesV, linesH;
		solarCanny(edges.data, edges.step, edges.cols, edges.rows, linesV,
				linesH);

		getPanels(imageGray, rects, linesH, linesV);

#		ifdef PANEL_DETECTOR_DEBUG
		saveImageWithRects(imageOrigin, rects, "all_rects.png");
#		endif

		Mat prepro;
		blur(imageGray2, prepro, Size(3, 3));

#		ifdef PANEL_DETECTOR_DEBUG
		imwrite("blurred.png", prepro);
#		endif

		validateCells(prepro.data, prepro.step, prepro.cols, prepro.rows,
				rects);
	} else {
		vector<Rect3> rectTmps;

		// 1. No rotate
		vector<Rect3> rectsNoRotate;
		Mat imgNoRotate = edges;

#		ifdef PANEL_DETECTOR_DEBUG
		imwrite("viet_rotate_non.png", imgNoRotate);
#		endif

		if (detectMode == 2) {
			detectSolarCell(imgNoRotate.data, imgNoRotate.step,
					imgNoRotate.cols, imgNoRotate.rows, rectsNoRotate);

		}

		// 2. Rotate 45 degree
		vector<Rect3> rectsRotate45;
		Mat imgRotate45 = edges;

		Point2f center((edges.cols - 1) / 2.0, (edges.rows - 1) / 2.0);
		rotateImg(imgRotate45, 360 - 45, center);

#		ifdef PANEL_DETECTOR_DEBUG
		imwrite("viet_rotate45.png", imgRotate45);
#		endif

		if (detectMode == 2) {
			detectSolarCell(imgRotate45.data, imgRotate45.step,
					imgRotate45.cols, imgRotate45.rows, rectsRotate45);
		}

		int originalRows = edges.rows;
		int originalCols = edges.cols;

		rerotateRect(rectsRotate45, imgRotate45, center, M_PI_4, originalRows,
				originalCols);

		for (size_t i = 0; i < rectsRotate45.size(); i++) {
			rectsRotate45[i].normalizeRotation();
			rectsRotate45[i].normalizePosition();
			rectsRotate45[i].calcBound();
		}

		// 3. Rotate 90 degree
		vector<Rect3> rectsRotate90;
		Mat imgRotate90 = edges;

		rotateImg(imgRotate90, 360 - 90, center);

#		ifdef PANEL_DETECTOR_DEBUG
		imwrite("viet_rotate90.png", imgRotate90);
#		endif

		if (detectMode == 2) {
			detectSolarCell(imgRotate90.data, imgRotate90.step,
					imgRotate90.cols, imgRotate90.rows, rectsRotate90);
		}

		rerotateRect(rectsRotate90, imgRotate90, center, M_PI_2, originalRows,
				originalCols);
		for (size_t i = 0; i < rectsRotate90.size(); i++) {
			rectsRotate90[i].normalizeRotation();
			rectsRotate90[i].normalizePosition();
			rectsRotate90[i].calcBound();
		}

		// sum up 3 panels group
		rectTmps.insert(rectTmps.end(), rectsNoRotate.begin(),
				rectsNoRotate.end());
		rectTmps.insert(rectTmps.end(), rectsRotate45.begin(),
				rectsRotate45.end());
		rectTmps.insert(rectTmps.end(), rectsRotate90.begin(),
				rectsRotate90.end());

		// Merge duplicate group
		rectTmps = mergeRect(rectTmps);

		for (size_t i = 0; i < rectTmps.size(); i++) {
			rects.push_back(rectTmps[i]);
		}
	}

#	ifdef PANEL_DETECTOR_DEBUG
	drawRects(rects, edges, "viet_rects_before_sort_top.png");
#	endif

	sortTopsOfRects(rects);

#	ifdef PANEL_DETECTOR_DEBUG
	drawRects(rects, edges, "viet_rects_after_sort_top.png");
#	endif

	// Sort rectangles
	sortRects(rects, rects);

	// Remove alone rectangles
	removeWrongRect(rects, rects);

#	ifdef PANEL_DETECTOR_DEBUG
	drawRects(rects, edges, "viet_rects_after_sort.png");
#	endif

	panels.clear();
	for (vector<RectF>::iterator it = rects.begin(); it != rects.end(); it++) {
		vector<Point> panel;
		panel.push_back(Point(it->x1, it->y1));
		panel.push_back(Point(it->x2, it->y2));
		panel.push_back(Point(it->x3, it->y3));
		panel.push_back(Point(it->x4, it->y4));
		panels.push_back(panel);
	}
}

void findSolarPanels(const Mat &imageOrigin, vector<vector<Point> > &panels) {
	findSolarPanels(imageOrigin, panels, 2);
}

}
