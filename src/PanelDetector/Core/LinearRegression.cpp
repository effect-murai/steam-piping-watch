/*
 * Author : Le Dung , FPT
 * Date : 2018-06-15
 * Last-modified by : Le Dung, FPT
 * Last-modified : 2018-06-15
 */

#include <string.h>
#include <math.h>
#include <limits>
#include <assert.h>

#include "../Geometry/geometry_util.h"
#include "LinearRegression.h"

lr_support::lr_support() {
	memset(this, 0, sizeof(lr_support));
}

void lr_support::calc(const std::vector<PointI> &points) {
	sumX = 0;
	sumX2 = 0;
	sumY = 0;
	sumY2 = 0;
	sumXY = 0;
	for (size_t i = 0; i < points.size(); i++) {
		sumX += points[i].x;
		sumY += points[i].y;
		sumXY += points[i].x * points[i].y;
		sumX2 += points[i].x * points[i].x;
		sumY2 += points[i].y * points[i].y;
	}
}

lr_support lr_support::operator+(const lr_support &lr) {
	lr_support new_lr;
	new_lr.sumX = sumX + lr.sumX;
	new_lr.sumX2 = sumX2 + lr.sumX2;
	new_lr.sumY = sumY + lr.sumY;
	new_lr.sumY2 = sumY2 + lr.sumY2;
	new_lr.sumXY = sumXY + lr.sumXY;

	return new_lr;
}

void lr_support::operator+=(const lr_support &lr) {
	sumX += lr.sumX;
	sumX2 += lr.sumX2;
	sumY += lr.sumY;
	sumY2 += lr.sumY2;
	sumXY += lr.sumXY;
}

bool lr_support::getLine(int num, float &a, float &b) {
	int64_t det = (int64_t) num * sumX2 - sumX * sumX;
	if (det == 0) {
		return false;
	}

	b = (float) (sumY * sumX2 - sumX * sumXY) / det;
	a = (float) (num * sumXY - sumX * sumY) / det;

	return true;
}

// https://en.wikipedia.org/wiki/Linear_regression

// return line in form : y = a*x + b
bool getLine(int *points, // x0, y0, x1, y1, ...
		int numPoints, float &a, float &b) {
	a = 0;
	b = 0;

	int64_t sumX = 0;
	int64_t sumX2 = 0;
	int64_t sumY = 0;
	int64_t sumY2 = 0;
	int64_t sumXY = 0;
	int64_t x, y;

	size_t size = numPoints * 2;
	for (size_t i = 0; i < size; i += 2) {
		x = points[i];
		y = points[i + 1];

		sumX += x;
		sumY += y;
		sumXY += x * y;
		sumX2 += x * x;
		sumY2 += y * y;
	}

	int64_t det = numPoints * sumX2 - sumX * sumX;
	if (det == 0) {
		return false;
	}

	b = (float) (sumY * sumX2 - sumX * sumXY) / det;
	a = (float) (numPoints * sumXY - sumX * sumY) / det;

	return true;
}

// return line in form : y = a*x + b
bool getLine(std::vector<PointI> points, float &a, float &b) {
	assert(points.size() >= 2);

	a = 0;
	b = 0;

	int64_t sumX = 0;
	int64_t sumX2 = 0;
	int64_t sumY = 0;
	int64_t sumY2 = 0;
	int64_t sumXY = 0;
	for (size_t i = 0; i < points.size(); i++) {
		sumX += points[i].x;
		sumY += points[i].y;
		sumXY += points[i].x * points[i].y;
		sumX2 += points[i].x * points[i].x;
		sumY2 += points[i].y * points[i].y;
	}

	int64_t det = (int64_t) points.size() * sumX2 - sumX * sumX;

	if (det == 0) {
		// return false;
		// this is the vertical line, work around
		a = 1000.f;
		int64_t mid = points.size() / 2;
		b = -a * points[mid].x;
	} else {
		a = (float) ((int64_t) points.size() * sumXY - sumX * sumY) / det;
		b = (float) (sumY * sumX2 - sumX * sumXY) / det;
	}

	return true;
}

// return line in form : y = a*x + b
bool getLine(PointI pt1, PointI pt2, float &a, float &b) {
	if (pt2.x != pt1.x) {
		a = (pt2.y - pt1.y) / (pt2.x - pt1.x);
		b = pt1.y - a * pt1.x;
	} else {
		// this is the vertical line, work around
		a = 1000.f;
		b = -a * pt1.x;
	}

	return true;
}

// return line in form : y = a*x + b
// and check if all distance from points to the line is smaller than threshold
bool getLine(int *points,		// x0, y0, x1, y1, ...
		int numPoints, float maxDis, float &a, float &b) {
	if (!getLine(points, numPoints, a, b)) {
		return false;
	}

	int x, y;
	float dis;
	size_t size = numPoints * 2;
	for (size_t i = 0; i < size; i += 2) {
		x = points[i];
		y = points[i + 1];

		dis = distancePointToLine(x, y, a, b);
		if (dis > maxDis) {
			return false;
		}
	}

	return true;
}

// return line in form : y = a*x + b
// and return the sum distance^2
bool getLine(int *points,		// x0, y0, x1, y1, ...
		int numPoints, float maxDis, float &a, float &b, float &error) {
	if (!getLine(points, numPoints, a, b)) {
		return false;
	}

	int x, y;
	error = 0;
	size_t size = numPoints * 2;
	for (size_t i = 0; i < size; i += 2) {
		x = points[i];
		y = points[i + 1];

		error += squareDistancePointToLine(x, y, a, b);
	}

	return true;
}

// return line in form : y = a*x + b
// and check if all distance from points to the line is smaller than threshold
bool getLine(std::vector<PointI> points, float maxDis, float &error, float &a,
		float &b) {
	if (points.size() < 2) {
		return false;
	} else if (points.size() == 2) {
		if (!getLine(points[0], points[1], a, b)) {
			return false;
		}
	} else {
		if (!getLine(points, a, b)) {
			return false;
		}
	}

	float dis;
	error = 0;
	for (size_t i = 0; i < points.size(); i++) {
		dis = distancePointToLine(points[i].x, points[i].y, a, b);
		if (dis > maxDis) {
			return false;
		}

		error += dis;
	}

	error /= points.size();

	return true;
}

void transposeLine(float &a, float &b) {
	// should be faster way than this

	// transpose 2 point
	float x1, y1;
	float x2, y2;

	assert(a != 0);
	y1 = 0;
	x1 = -b / a;

	x2 = 0;
	y2 = b;

	if (y2 == y1) {
		x2 = 1;
		y2 = a + b;
	}

	float x1t, y1t;
	float x2t, y2t;

	x1t = y1;
	y1t = x1;

	x2t = y2;
	y2t = x2;

	a = (y2t - y1t) / (x2t - x1t);
	b = y1t - a * x1t;
}

// linear regression work badly with almost vertical line, so transpose them
// return line in form : y = a*x + b
// and check if all distance from points to the line is smaller than threshold
bool getLineT(std::vector<PointI> points, float maxDis, float &error, float &a,
		float &b) {
	if (points.size() < 2) {
		return false;
	} else if (points.size() == 2) {
		if (!getLine(points[0], points[1], a, b)) {
			return false;
		}

		error = 0;
	} else {
		// transpose
		PointI tmp;
		for (size_t i = 0; i < points.size(); i++) {
			points[i].transpose();
		}

		if (!getLine(points, a, b)) {
			return false;
		}

		float dis;
		float dis2 = maxDis * maxDis;
		error = 0;
		for (size_t i = 0; i < points.size(); i++) {
			dis = squareDistancePointToLine(points[i].x, points[i].y, a, b);
			if (dis > dis2) {
				return false;
			}

			error += dis;
		}

		error /= points.size();

		// transpose the line
		if (a == 0) {
			a = 1000.f;
			int mid = points.size() / 2;
			// actually should be x, but the point transposed, so y
			b = -a * points[mid].y;
		} else {
			transposeLine(a, b);
		}
	}

	return true;
}

// find best line, may exclude some points
bool getLineExclude(std::vector<PointI> points, float limDis, float &error,
		float &a, float &b) {
	int maxNumDeleted = points.size() * 0.2f;
	int numDeleted = 0;
	while (true) {
		if (points.size() < 2) {
			return false;
		} else if (points.size() == 2) {
			if (!getLine(points[0], points[1], a, b)) {
				return false;
			}
		} else {
			if (!getLine(points, a, b)) {
				return false;
			}
		}

		float dis;
		error = 0;
		float maxDis = 0;
		int worst = -1;
		bool passed = true;
		for (size_t i = 0; i < points.size(); i++) {
			dis = squareDistancePointToLine(points[i].x, points[i].y, a, b);

			if (dis > maxDis) {
				maxDis = dis;
				worst = i;
			}

			if (dis > limDis) {
				passed = false;
			}

			error += dis;
		}

		error /= points.size();

		// try to exclude the bad guy
		if (!passed) {
			if (numDeleted < maxNumDeleted) {
				points.erase(points.begin() + worst);
				numDeleted++;
			} else {
				return false;
			}
		} else {
			break;
		}
	}

	return true;
}

// linear regression work badly with almost vertical line, so transpose them
// return line in form : y = a*x + b
// and check if all distance from points to the line is smaller than threshold
bool getLine2(std::vector<PointI> points, float maxDis, float &error, float &a,
		float &b) {
	if (points.size() < 2) {
		return false;
	} else if (points.size() == 2) {
		if (!getLine(points[0], points[1], a, b)) {
			return false;
		}

		error = 0;
	} else {
		if (!getLine(points, a, b)) {
			return false;
		}

		float dis;
		float dis2 = maxDis * maxDis;
		error = 0;
		for (size_t i = 0; i < points.size(); i++) {
			dis = squareDistancePointToLine(points[i].x, points[i].y, a, b);
			if (dis > dis2) {
				return false;
			}

			error += dis;
		}

		error /= points.size();

		// transpose the line
		if (a == 0) {
			a = 1000.f;
			int mid = points.size() / 2;
			b = -a * points[mid].y;	// actually should be x, but the point transposed, so y
		} else {
			transposeLine(a, b);
		}
	}

	return true;
}
