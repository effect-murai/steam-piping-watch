/*
 * SAlgorithm.cpp
 *
 *  Created on: 2020/04/09
 *      Author: k.tasaki
 */

#include "SAlgorithm.h"

#include <math.h>
#include <algorithm> // notice this

// khac biet % so voi gia tri trung binh
//  0 -> 0%
//  1 -> 100%
float diff2ValueAverage(float v1, float v2) {
	float ave = (v1 + v2) / 2;
	if (ave == 0) {
		return 0;
	}

	return (fabs(v1 - v2) / fabs(ave));
}

// khac biet % so voi gia tri max
//  0 -> 0%
//  1 -> 100%
float diff2ValueMax(float v1, float v2) {
	if (v1 == 0 && v2 == 0) {
		return 0;
	}

	return fabs(v1 - v2) / std::max(fabs(v1), fabs(v2));
}

// khac biet % so voi gia tri max
// dung khi ca 2 deu khg am
//  0 -> 0%
//  1 -> 100%
float diff2PositiveValueMax(float v1, float v2) {
	if (v1 == 0 && v2 == 0) {
		return 0;
	}

	return (fabs(v1 - v2) / std::max(v1, v2));
}

float diff2ValueSign(float v1, float v2) {
	if (v1 == 0 && v2 == 0)
		return 0;

	return ((v1 - v2) / std::max(v1, v2));
}
