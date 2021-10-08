/*
 * Author : Le Dung , FPT
 * Date : 2018-06-15
 * Last-modified by : Le Dung, FPT
 * Last-modified : 2018-06-15
 */

#ifndef __DLINEAR_REGRESSION__
#define __DLINEAR_REGRESSION__

#include <vector>
#include "dstruct.h"
#include <stdint.h>

struct lr_support {
	int64_t sumX;
	int64_t sumX2;
	int64_t sumY;
	int64_t sumY2;
	int64_t sumXY;

	lr_support();

	void calc(const std::vector<PointI> &points);

	lr_support operator+(const lr_support &lr);

	void operator+=(const lr_support &lr);

	bool getLine(int num, float &a, float &b);
};

// return line in format : y = a*x + b
bool getLine(int *points, // x0, y0, x1, y1, ..., xn, yn
		int numPoints, float &a, float &b);

bool getLine(std::vector<PointI> points, float &a, float &b);

bool getLine(std::vector<PointI> points, float maxDis, float &error, float &a,
		float &b);

bool getLineT(std::vector<PointI> points, float maxDis, float &error, float &a,
		float &b);

// distance from point x, y to line y =a*x + b
float distancePointToLine(int x, int y,	//point
		float a, float b	//line
		);

bool getLineExclude(std::vector<PointI> points, float limDis, float &error,
		float &a, float &b);

#endif // __DLINEAR_REGRESSION__

