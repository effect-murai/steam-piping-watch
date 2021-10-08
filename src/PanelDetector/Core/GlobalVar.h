/*
 * GlobalVar.h
 *
 *  Created on: 2020/04/09
 *      Author: k.tasaki
 */

#ifndef GLOBAL_VAR_H_
#define GLOBAL_VAR_H_

#include <string>
#include <opencv2/opencv.hpp>

//----------------------------------variables-------------------------------------
extern int gValidateCell;
extern float gCellRatio;
extern float gCellRatioThres;
extern float minRectQuadRate;

extern float minLineLongPow;
extern float minLineLengthPow;
extern float lengthPowRatio;
extern float minPointlDist; // (vertical or horizontal)
extern float distPowDiff;
extern float minWidthPow;
extern float perpendRate; // = cos(alpha)
extern float rectLengthRatio;
extern float radianAccuracy;
extern float maxPositiveRadian;
extern float maxRectDistanceRate;

extern int gMinLen;
extern int gMaxLen;

#endif // GLOBAL_VAR_H_
