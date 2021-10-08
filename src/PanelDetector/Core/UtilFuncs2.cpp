#include "UtilFuncs2.h"

#include <fstream>
#include <opencv2/opencv.hpp>

#include "dstruct.h"
#include "FindPanel.h"
#include "GlobalVar.h"
#include "UtilFuncs.h"
#include "../Geometry/DCoordinateChanger2d.h"
#include "../Geometry/geometry_util.h"
#include "../Geometry/Vector2f.h"

using namespace std;
using namespace cv;

void drawLines(vector<Vec4f> lines, Mat imgLine, String imageName) {
	size_t lineSize = lines.size();

	if (lineSize == 0) {
		return;
	}

	for (size_t i = 0; i < lineSize; i++) {
		Vec4f l = lines[i];
		line(imgLine, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255),
				1, LINE_AA);
	}

	imwrite(imageName, imgLine);
}

void drawLines(vector<DLine> lines, Mat imgLine, String imageName) {
	size_t lineSize = lines.size();

	if (lineSize == 0) {
		return;
	}

	for (size_t i = 0; i < lineSize; i++) {
		DLine l = lines[i];
		line(imgLine, Point(l.line[0], l.line[1]), Point(l.line[2], l.line[3]),
				Scalar(0, 0, 255), 1, LINE_AA);
	}

	imwrite(imageName, imgLine);
}

void removeShortLines(std::vector<cv::Vec4f> &lineGroup, float minLength) {
	if (lineGroup.size() == 0) {
		return;
	}
	// Remove short lines
	for (int i = 0; i < (int) lineGroup.size(); i++) {
		Vec4f lineCheck = lineGroup[i];
		Point2f p1(lineCheck[0], lineCheck[1]);
		Point2f p2(lineCheck[2], lineCheck[3]);

		float d = getDistancePointsPow(p1, p2);
		// add long line first
		if (d < minLength) {
			lineGroup.erase(lineGroup.begin() + i);
			i--;
		}
	}
}

void line2dLine(std::vector<cv::Vec4f> lineGroup,
		std::vector<DLine> &dLineGroup) {
	size_t lineGroupSize = lineGroup.size();
	if (lineGroupSize == 0) {
		return;
	}

	for (size_t i = 0; i < lineGroupSize; i++) {
		Vec4f line = lineGroup[i];
		Point2f p1(line[0], line[1]);
		Point2f p2(line[2], line[3]);

		float d = getDistancePointsPow(p1, p2);

		dLineGroup.push_back(DLine(line, d));
	}
}

void removeShortLines(std::vector<DLine> &lineGroup, float minLength) {
	if (lineGroup.size() == 0) {
		return;
	}

	// Remove short lines
	for (int i = 0; i < (int) lineGroup.size(); i++) {
		DLine lineCheck = lineGroup[i];

		// add long line first
		if (lineCheck.lengthPow < minLength) {
			lineGroup.erase(lineGroup.begin() + i);
			i--;
		}

	}

}

float findDirect(DLine line) {
	Vec4f l0(0, 0, 1, 0);
	Vec4f l1 = line.line;

	return getDot(l0, l1);
}

float getDot(cv::Vec4f l1, cv::Vec4f l2) {
	Vector2F v;
	Vector2F v1(l1[2] - l1[0], l1[3] - l1[1]);
	Vector2F v2(l2[2] - l2[0], l2[3] - l2[1]);

	Vector2F v1Nom = v.normalize(v1);
	Vector2F v2Nom = v.normalize(v2);
	return v.dot(v1Nom, v2Nom); // = cos(alpha)
}

bool checkPositiveNumber(float number, float value, float rate) {
	if (number < 0) {
		number *= -1;
	}
	if (value == 0) {
		if (number < rate) {
			return true;
		} else {
			return false;
		}
	} else {
		if ((number > value / rate) && (number < value * rate)) {
			return true;
		} else {
			return false;
		}
	}
}

bool isIntersecting(cv::Vec4f &a, cv::Vec4f &b) {
	float x1, y1, x2, y2, x3, y3, x4, y4, xI, yI;
	x1 = a[0];
	y1 = a[1];
	x2 = a[2];
	y2 = a[3];
	x3 = b[0];
	y3 = b[1];
	x4 = b[2];
	y4 = b[3];
	int res;

	res = twoLineIntersection(x1, y1, x2, y2, x3, y3, x4, y4, xI, yI);

	if (res == 3) {
		return true;
	} else {
		return false;
	}
}

float checkIncline(cv::Mat imgBlurred) {

	Mat imgHSV, canniedImg;

	int threshCanny = 40;
	int max_thresh = 80;

	// Edge detection
	Canny(imgBlurred, canniedImg, threshCanny, max_thresh, 3);

	// Probabilistic Line Transform
	vector<Vec4f> linesP;
	int threshHoughlinesP = 36;
	int minLineLength = 10;
	int maxLineGap = 10;

	HoughLinesP(canniedImg, linesP, 1, CV_PI / 180, threshHoughlinesP,
			minLineLength, maxLineGap);

#	ifdef PANEL_DETECTOR_DEBUG
		Mat imgCanny = imgBlurred;
		cvtColor(imgCanny, imgCanny, COLOR_GRAY2BGR);
		imwrite(gExePath + "viet_Canny Image.png", canniedImg);

		Mat imgLine = imgBlurred;
		cvtColor(imgLine, imgLine, COLOR_GRAY2BGR);

		drawLines(linesP, imgLine, "viet_imgLine.png");
#	endif

	// ********* Find the length of lines *************
	vector<DLine> d_LineGroup;
	line2dLine(linesP, d_LineGroup);

	// ********* Sort Line ***********
	sort(d_LineGroup.begin(), d_LineGroup.end(), DLineSort());

	// ********* Choose 10% longest lines *********

	size_t longestLinesAmount = d_LineGroup.size() / 10;
	vector<DLine> longestLines;

	for (size_t i = 0; i < longestLinesAmount; i++) {
		longestLines.push_back(d_LineGroup[i]);
	}

	// Remove line that not have intersection and not perpendicular to any other line
	for (int i = 0; i < (int) longestLines.size(); i++) {
		DLine lineCheck = longestLines[i];
		size_t count = 0;

		for (size_t j = i + 1; j < longestLines.size(); j++) {
			DLine line = longestLines[j];

			if (isIntersecting(lineCheck.line, line.line)
					&& checkPositiveNumber(getDot(lineCheck.line, line.line), 0,
							perpendRate)) {
				count++;
			}
		}

		if (count == 0) {
			longestLines.erase(longestLines.begin() + i);
			i--;
		}
	}

	// ******** Find Direction in longest lines ***************
	vector<vector<DLine> > longLinesGroup;
	vector<float> dotNomalizeGroup;

	for (size_t i = 0; i < longestLines.size(); i++) {
		DLine line = longestLines[i];

		float dotNomalize = findDirect(line);

		if (dotNomalize < 0) {
			dotNomalize *= -1;
		}

		size_t longLinesGroupSize = longLinesGroup.size();

		if (longLinesGroupSize == 0) {
			vector<DLine> newLineGroup;
			newLineGroup.push_back(line);
			longLinesGroup.push_back(newLineGroup);
			dotNomalizeGroup.push_back(dotNomalize);
		} else {
			size_t radGroupSize = dotNomalizeGroup.size();
			bool found = false;

			for (size_t j = 0; j < radGroupSize; j++) {
				float dotNomalizeCheck = dotNomalizeGroup[j];

				if (checkPositiveNumber(dotNomalizeCheck - dotNomalize, 0,
						radianAccuracy)) {
					longLinesGroup[j].push_back(line);

					found = true;

					dotNomalizeCheck = (dotNomalizeCheck + dotNomalize) / 2;
					dotNomalizeGroup[j] = dotNomalizeCheck;
					break;
				}
			}

			if (!found) {
				vector<DLine> newLineGroup;
				newLineGroup.push_back(line);
				longLinesGroup.push_back(newLineGroup);
				dotNomalizeGroup.push_back(dotNomalize);
			}
		}
	}
#	ifdef PANEL_DETECTOR_DEBUG
		Mat imgLineLong = imgBlurred;
		cvtColor(imgLineLong, imgLineLong, COLOR_GRAY2BGR);
		for (size_t i = 0; i < longLinesGroup.size(); i++) {
			drawLines(longLinesGroup[i], imgLineLong, "viet_imgLine_Long.png");
		}
#	endif

	// Find the most popular direction
	size_t longLinesGroupSize = longLinesGroup.size();

	float mostPopularDotNomalized = dotNomalizeGroup[0];

	size_t biggestAmountOfLines = longLinesGroup[0].size();

	for (size_t i = 0; i < longLinesGroupSize; i++) {
		size_t groupOfLineSize = longLinesGroup[i].size();

		if (groupOfLineSize > biggestAmountOfLines) {
			mostPopularDotNomalized = dotNomalizeGroup[i];
			biggestAmountOfLines = groupOfLineSize;
		}
	}

#	ifdef PANEL_DETECTOR_DEBUG
		Mat imgLineLongMost = imgBlurred;
		cvtColor(imgLineLongMost, imgLineLongMost, COLOR_GRAY2BGR);

		drawLines(longLinesGroup[mostPopularGroupLineIndex], imgLineLongMost,
				"viet_imgLine_Long_Most_Popular.png");
# 	endif

	return acos(mostPopularDotNomalized);

}

void rotateImg(cv::Mat &image, float angle, cv::Point2f center) {

	cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
	// determine bounding rectangle, center not relevant
	cv::Rect2f bbox =
			cv::RotatedRect(cv::Point2f(), image.size(), angle).boundingRect();
	// adjust transformation matrix
	rot.at<double>(0, 2) += bbox.width / 2.0 - image.cols / 2.0;
	rot.at<double>(1, 2) += bbox.height / 2.0 - image.rows / 2.0;

	cv::warpAffine(image, image, rot, bbox.size());

#	ifdef PANEL_DETECTOR_DEBUG
		imwrite(gExePath + "viet_rotated_img.png", image);
#	endif
}

void rerotateRect(std::vector<Rect3> &rectGroup, cv::Mat edges,
		cv::Point2f center, double angle, int originalRows, int originalCols) {
	double cosAlpha = cos(angle);
	double sinAlpha = sin(angle);

	// xac dinh vi tri goc he truc toa do moi
	Point2f newO(originalRows * sinAlpha, 0);
	Point2f newX(edges.cols, originalCols * sinAlpha);
	Point2f newY(0, originalRows * cosAlpha);

#	ifdef PANEL_DETECTOR_DEBUG
		Mat imgPoints = edges;
		cvtColor(imgPoints, imgPoints, COLOR_GRAY2BGR);

		circle(imgPoints, newO, 3, Scalar(0, 0, 255), 2, 8);
		circle(imgPoints, newX, 3, Scalar(0, 0, 255), 2, 8);
		circle(imgPoints, newY, 3, Scalar(0, 0, 255), 2, 8);

		imwrite(gExePath + "viet_new_Ordinate.png", imgPoints);
#	endif

	DCoordinateChanger2D coordTool;
	Vector2F newOrigin(newO.x, newO.y);
	Vector2F newDirX = Vector2F::normalize(
			Vector2F(newX.x - newO.x, newX.y - newO.y));
	Vector2F newDirY = Vector2F::normalize(
			Vector2F(newY.x - newO.x, newY.y - newO.y));
	coordTool.SetNewCoordSystemValue(newOrigin, newDirX, newDirY);

	size_t rectGroupSize = rectGroup.size();

	Vector2F tmpP;
	tmpP = coordTool.Convert2NewCoordSystem2(Vector2F(newY.x, newY.y));
	tmpP = coordTool.Convert2NewCoordSystem2(Vector2F(newO.x, newO.y));
	tmpP = coordTool.Convert2NewCoordSystem2(Vector2F(newX.x, newX.y));

	for (size_t j = 0; j < rectGroupSize; j++) {
		tmpP = coordTool.Convert2NewCoordSystem2(
				Vector2F(rectGroup[j].x1, rectGroup[j].y1));
		rectGroup[j].x1 = tmpP.x;
		rectGroup[j].y1 = tmpP.y;

		tmpP = coordTool.Convert2NewCoordSystem2(
				Vector2F(rectGroup[j].x2, rectGroup[j].y2));
		rectGroup[j].x2 = tmpP.x;
		rectGroup[j].y2 = tmpP.y;

		tmpP = coordTool.Convert2NewCoordSystem2(
				Vector2F(rectGroup[j].x3, rectGroup[j].y3));
		rectGroup[j].x3 = tmpP.x;
		rectGroup[j].y3 = tmpP.y;

		tmpP = coordTool.Convert2NewCoordSystem2(
				Vector2F(rectGroup[j].x4, rectGroup[j].y4));
		rectGroup[j].x4 = tmpP.x;
		rectGroup[j].y4 = tmpP.y;
	}

}

void sortTopsOfRects(std::vector<RectF> &rectGroup) {
	size_t rectsSize = rectGroup.size();

	for (size_t i = 0; i < rectsSize; i++) {

		RectF rect = rectGroup[i];

		vector<Point2f> tops;
		tops.push_back(Point2f(rect.x1, rect.y1));
		tops.push_back(Point2f(rect.x2, rect.y2));
		tops.push_back(Point2f(rect.x3, rect.y3));
		tops.push_back(Point2f(rect.x4, rect.y4));

		if (tops.size() != 4) {
			continue;
		}

		for (size_t j = 0; j < 4; j++) {

			Point2f topLeft, topRight, bottomLeft, bottomRight;

			topLeft = tops[j];

			if (j == 0) {
				bottomLeft = tops[3];
			} else {
				bottomLeft = tops[j - 1];
			}

			if (j == 2) {
				bottomRight = tops[0];
			}

			if (j == 3) {
				topRight = tops[0];
				bottomRight = tops[1];
			} else {
				topRight = tops[j + 1];
				bottomRight = tops[j + 2];
			}

			if (topRight.x - topLeft.x > minPointlDist) {
				if (bottomLeft.y - topLeft.y > minPointlDist) {

					if (j != 0) {
						rect.x1 = topLeft.x;
						rect.y1 = topLeft.y;

						rect.x2 = topRight.x;
						rect.y2 = topRight.y;

						rect.x3 = bottomRight.x;
						rect.y3 = bottomRight.y;

						rect.x4 = bottomLeft.x;
						rect.y4 = bottomLeft.y;

						rectGroup[i] = rect;
					}

					break;
				}
			}
		}
	}
}

void sortRects(std::vector<RectF> rectsBegin, std::vector<RectF> &rectsSorted) {
	rectsSorted.clear();
	if (rectsBegin.size() == 0) {
		return;
	}

	vector<vector<RectF> > rectsLineGroup;

	// Phan loai group line

	vector<RectF> rectsLine;
	rectsLine.push_back(rectsBegin[0]);
	rectsLineGroup.push_back(rectsLine);

	rectsBegin.erase(rectsBegin.begin());

	for (size_t i = 0; i < rectsBegin.size(); i++) {
		RectF rectCheck = rectsBegin[i];

		size_t rectsLineGroupSize = rectsLineGroup.size();
		bool isPushed = false;

		for (size_t j = 0; j < rectsLineGroupSize; j++) {
			size_t rectsLineSize = rectsLineGroup[j].size();
			for (size_t k = 0; k < rectsLineSize; k++) {

				RectF rect = rectsLineGroup[j][k];
				Point2f pCheck1(rectCheck.x1, rectCheck.y1);
				Point2f pCheck2(rectCheck.x2, rectCheck.y2);
				Point2f p1(rect.x1, rect.y1);
				Point2f p2(rect.x2, rect.y2);

				float d1Pow = getDistancePointsPow(pCheck1, p2);
				float d2Pow = getDistancePointsPow(pCheck2, p1);
				if (d1Pow < 100 || d2Pow < 100) {
					rectsLineGroup[j].push_back(rectCheck);

					rectsBegin.erase(rectsBegin.begin() + i);
					i--;
					isPushed = true;
					break;
				}
			}

			if (isPushed) {
				break;
			}
		}

		size_t rectsBeginSize = rectsBegin.size();

		if (rectsBeginSize > 0 && (i == rectsBeginSize - 1)) {
			// The end of rectsBegin
			// push a new group line
			vector<RectF> rectsLine;
			rectsLine.push_back(rectsBegin[0]);
			rectsLineGroup.push_back(rectsLine);

			rectsBegin.erase(rectsBegin.begin());
			i = -1;
		}

	}

	// Sorting
	size_t rectsLineGroupSize = rectsLineGroup.size();

	for (size_t i = 0; i < rectsLineGroupSize; i++) {
		// Sort rectangles in line
		sort(rectsLineGroup[i].begin(), rectsLineGroup[i].end(), RectsSort());
	}

	// Sort rectangle line in group
	sort(rectsLineGroup.begin(), rectsLineGroup.end(), RectsLinesSort());

	rectsSorted.erase(rectsSorted.begin(), rectsSorted.end());

	for (int i = 0; i < (int) rectsLineGroupSize; i++) {

		size_t rectsLineSize = rectsLineGroup[i].size();

		for (size_t j = 0; j < rectsLineSize; j++)
			rectsSorted.push_back(rectsLineGroup[i][j]);
	}
}

void removeWrongRect(std::vector<RectF> rectGroupIn,
		std::vector<RectF> &rectGroupFiltered) {
	if (rectGroupIn.size() == 0) {
		return;
	}

	for (size_t i = 0; i < rectGroupIn.size(); i++) {
		bool isNear = false;
		RectF rectCheck = rectGroupIn[i];
		float rectQuad = areaQuad(rectCheck.x1, rectCheck.y1, rectCheck.x2,
				rectCheck.y2, rectCheck.x3, rectCheck.y3, rectCheck.x4,
				rectCheck.y4);

		vector<Point2f> topsCheckGroup;
		topsCheckGroup.push_back(Point2f(rectCheck.x1, rectCheck.y1));
		topsCheckGroup.push_back(Point2f(rectCheck.x2, rectCheck.y2));
		topsCheckGroup.push_back(Point2f(rectCheck.x3, rectCheck.y3));
		topsCheckGroup.push_back(Point2f(rectCheck.x4, rectCheck.y4));

		for (size_t k = 0; k < 4; k++) {

			for (size_t j = 0; j < rectGroupIn.size(); j++) {
				if (i == j) {
					continue;
				}

				RectF rect = rectGroupIn[j];
				vector<Point2f> topsGroup;
				topsGroup.push_back(Point2f(rect.x1, rect.y1));
				topsGroup.push_back(Point2f(rect.x2, rect.y2));
				topsGroup.push_back(Point2f(rect.x3, rect.y3));
				topsGroup.push_back(Point2f(rect.x4, rect.y4));

				for (size_t m = 0; m < 4; m++) {
					float dCheckPow = getDistancePointsPow(topsCheckGroup[k],
							topsGroup[m]);

					if (dCheckPow / rectQuad < maxRectDistanceRate) {
						isNear = true;
						break;
					}
				}

				if (isNear) {
					break;
				}
			}

			if (isNear) {
				break;
			}
		}

		if (!isNear) {
			rectGroupIn.erase(rectGroupIn.begin() + i);
			rectGroupFiltered.erase(rectGroupFiltered.begin() + i);
			i--;
		}
	}
}
