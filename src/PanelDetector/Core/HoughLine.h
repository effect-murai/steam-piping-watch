/*
 * Author : Le Dung , FPT
 * Date : 2018-06-15
 * Last-modified by : Le Dung, FPT
 * Last-modified : 2018-06-15
 */

#ifndef __HOUGH_LINE__
#define __HOUGH_LINE__

#include <vector>

#include "dstruct.h"

void initSinCosTable();

std::vector<LinePolar> getHoughLine(std::vector<PointI> points);

// return line in form : y = a*x + b
bool getHoughLine(std::vector<PointI> points, float maxDis, float &error,
		float &a, float &b);

bool getBestLine(std::vector<PointI> points, float maxDistance, float &error,
		float &a, float &b);

void getError(std::vector<PointI> points, float a, float b, float maxDistance,
		float &error, std::vector<PointI> &good_points);

#endif // __HOUGH_LINE__
