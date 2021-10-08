/*
 * dstruct.cpp
 *
 *  Created on: 2020/04/07
 *      Author: k.tasaki
 */

#include "dstruct.h"

#include <float.h>
#include <string.h>

Rect3::Rect3() {
	memset(this, 0, sizeof(Rect3));
}

void Rect3::calcBound() {
	minX = std::min(x1, x4);
	maxX = std::max(x2, x3);
	minY = std::min(y1, y2);
	maxY = std::max(y3, y4);
}

void DPoint::copy(const DPoint &p) {
	memcpy(this, &p, sizeof(DPoint));
}

float DPair::getCenterRight() {
	if (left.type == -1 && right.type == 1) {
		return (std::max((float) right.x,
				(left.getposf() * (-left.yf) + right.getposf() * right.yf)
						/ (-left.yf + right.yf)));
	} else if (left.type == 0 && right.type == 1) {
		return right.getposf();
	} else if (left.type == -1 && right.type == 0) {
		return left.x + left.lenx;
	} else {
		return 0.0f;
	}
}

float DPair::getCenterRight2() {
	if (right.type == 1)
		return right.getposf4_b();
	else if (left.type == -1)
		return left.x + left.lenx + 1;

	return FLT_MAX;
}

float DPair::getCenterLeft() {
	if (left.type == -1 && right.type == 1) {
		return (std::min((float) (left.x + left.lenx),
				(left.getposf() * (-left.yf) + right.getposf() * right.yf)
						/ (-left.yf + right.yf)));
	} else if (left.type == -1 && right.type == 0) {
		return left.getposf();
	} else if (left.type == 0 && right.type == 1) {
		return right.x;
	} else {
		return 0.0f;
	}
}

float DPair::getCenterLeft2() {
	if (left.type == -1)
		return left.getposf4_e();
	else if (right.type == 1)
		return right.x - 1;

	return -FLT_MAX;
}

float DPair::getValidLeft() {
	if (left.type != 0)
		return left.x;
	else
		return -FLT_MAX;
}

float DPair::getValidRight() {
	if (right.type != 0)
		return right.x + right.lenx;
	else
		return FLT_MAX;
}
