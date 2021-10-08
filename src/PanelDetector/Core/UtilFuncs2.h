/*
 * UtilFuncs2.h
 *
 *  Created on: 2020/04/09
 *      Author: k.tasaki
 */

#ifndef __UTIL_FUNCS2__
#define __UTIL_FUNCS2__

#include <opencv2/opencv.hpp>
#include "dstruct.h"

struct DLine {
	cv::Vec4f line;
	float lengthPow;

	DLine(cv::Vec4f l, float le) {
		line = l;
		lengthPow = le;
	}
};

struct DLineSort {
	bool operator()(DLine dl1, DLine dl2) {
		return (dl1.lengthPow > dl2.lengthPow);
	}
};

void line2dLine(std::vector<cv::Vec4f> lines, std::vector<DLine> &dLineGroup);

void removeShortLines(std::vector<DLine> &lineGroup, float minLength);

void drawLines(std::vector<cv::Vec4f> lines, cv::Mat imgLine,
		cv::String imageName);

void removeShortLines(std::vector<cv::Vec4f> &lineGroup, float minLength);

void sortTopsOfRects(std::vector<RectF> &rects);

void sortTopsOfRects(std::vector<Rect3> &rects);

void sortRects(std::vector<RectF> rectsBegin, std::vector<RectF> &rectsSorted);

float getDot(cv::Vec4f l1, cv::Vec4f l2);

bool checkPositiveNumber(float number, float value, float rate);

bool isIntersecting(cv::Vec4f &a, cv::Vec4f &b);

float checkIncline(cv::Mat edges);

void rotateImg(cv::Mat &image, float angle, cv::Point2f center);

void rerotateRect(std::vector<Rect3> &rects, cv::Mat edges, cv::Point2f center,
		double angle, int originalRows, int originalCols);

void removeWrongRect(std::vector<RectF> rectGroupIn,
		std::vector<RectF> &rectGroupFiltered);
#endif
