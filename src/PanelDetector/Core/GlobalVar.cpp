#include "GlobalVar.h"

//----------------------------------variables-------------------------------------
int gValidateCell = 0;
float gCellRatio = 1.67f;
float gCellRatioThres = 0.4f;
float minRectQuadRate = 1.4;
int gMinLen = 10;
int gMaxLen = 80;

float minLineLongPow = 6400;
float minLineLengthPow = 100;
float lengthPowRatio = 1.3;
float minWidthPow = 400;
float distPowDiff = 500;
float minPointlDist = 10;
float rectLengthRatio = 2.7;
float perpendRate = 0.1; // = cos(alpha)
float radianAccuracy = 0.04; // ~ 3 degree
float maxPositiveRadian = 0.52; // ~ 30 / 180 * M_PI;
float maxRectDistanceRate = 0.1;
