/*
 * UtilFuncs2.h
 *
 *  Created on: 2020/04/09
 *      Author: k.tasaki
 */

#ifndef __FINDPANEL__
#define __FINDPANEL__

#include <opencv2/opencv.hpp>
#include "dstruct.h"
#include "GlobalVar.h"
#include "UtilFuncs2.h"

struct RectsSort {
	bool operator()(RectF rect1, RectF rect2) {
		return (rect1.x1 < rect2.x1);
	}
};

struct RectsLinesSort {
	bool operator()(std::vector<RectF> rectsLine1,
			std::vector<RectF> rectsLine2) {
		float yBeginLine1 = rectsLine1[0].y1;
		float yBeginLine2 = rectsLine2[0].y1;

		if (!checkPositiveNumber(yBeginLine1 - yBeginLine2, 0, minPointlDist))
			return (yBeginLine1 < yBeginLine2);
		else {
			float yEndLine1 = rectsLine1[rectsLine1.size() - 1].y2;
			float yEndLine2 = rectsLine2[rectsLine2.size() - 1].y2;

			return (yEndLine1 < yEndLine2);

		}
	}
};

// ----------------------------------functions-------------------------------------

float getDistancePointsPow(cv::Point2f p1, cv::Point2f p2);

bool checkTwoNumbers(float numb1, float numb2, float range);

void getPanels(cv::Mat image, std::vector<RectF> &rects,
		std::vector<cv::Vec4f> linesH, std::vector<cv::Vec4f> linesV);

cv::Vec2f getLineFunction(cv::Vec4f line);

bool haveLinesFunct(cv::Vec2f lineFunct, std::vector<cv::Vec2f> linesFunct,
		size_t &position, float aDiffRate, float bDiffRate);

void drawRects(std::vector<RectF> rects, cv::Mat src_ir, cv::String imageName);

#endif
