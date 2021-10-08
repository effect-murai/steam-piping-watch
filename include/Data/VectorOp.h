/*
 * VectorOp.h
 *
 *  Created on: 2016/03/15
 *      Author: PC-EFFECT-012
 */

#ifndef VECTOROP_H_
#define VECTOROP_H_

#include "CommonData.h"

inline Vector2D operator+(Vector2D a, Vector2D b) {
	Vector2D c;
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	return c;
}

inline Vector2D operator-(Vector2D a, Vector2D b) {
	Vector2D c;
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	return c;
}

inline double abs(Vector2D vec) {
	return sqrt(vec.x * vec.x + vec.y * vec.y);
}

// 内積 (dot product) : a・b = |a||b|cosθ
inline double dotProduct(Vector2D a, Vector2D b) {
	return (a.x * b.x + a.y * b.y);
}

// 外積 (cross product) : a×b = |a||b|sinθ
inline double crossProduct(Vector2D a, Vector2D b) {
	return (a.x * b.y - a.y * b.x);
}

// 点a,bを端点とする線分と点cとの距離
inline double distanceLineSegmentAndPoint(Vector2D a, Vector2D b, Vector2D c) {
	if (dotProduct(b - a, c - a) < 0.0) {
		return abs(c - a);
	}
	if (dotProduct(a - b, c - b) < 0.0) {
		return abs(c - b);
	}
	return fabs(crossProduct(b - a, c - a)) / abs(b - a);
}

// 線分と点cとの距離
inline double distanceLineSegmentAndPoint(LineSegment &line, Vector2D &point) {
	return distanceLineSegmentAndPoint(line.pt1, line.pt2, point);
}

inline Vector2D toVector2D(POINT &val2) {
	Vector2D val1;
	val1.x = val2.x;
	val1.y = val2.y;
	return val1;
}

inline POINT toPOINT(Vector2D &val2) {
	POINT val1;
	val1.x = val2.x;
	val1.y = val2.y;
	return val1;
}

inline Vector2D getCrossPoint(LineSegment &line1, LineSegment &line2) {
	// 2直線の交点
	Vector2D pt;
	// 2つの線分のxの長さを計算
	double dx1 = (line1.pt1.x - line1.pt2.x);
	double dx2 = (line2.pt1.x - line2.pt2.x);
	if ((dx1 != 0) && (dx2 != 0)) {
		// 傾きa　切片bを求める式
		double a1 = (line1.pt1.y - line1.pt2.y) / dx1;
		double b1 = line1.pt1.y - a1 * line1.pt1.x;

		double a2 = (line2.pt1.y - line2.pt2.y) / dx2;
		double b2 = line2.pt1.y - a2 * line2.pt1.x;

		if ((a1 - a2) != 0) {
			// 交点を求める
			pt.x = (b2 - b1) / (a1 - a2);
			pt.y = a2 * pt.x + b2;
		} else {
			// 平行の場合
			pt.x = NAN;
			pt.y = NAN;
		}
	} else if (dx2 != 0) {
		// 傾きa　切片bを求める式
		double a2 = (line2.pt1.y - line2.pt2.y) / dx2;
		double b2 = line2.pt1.y - a2 * line2.pt1.x;

		// 交点を求める
		pt.x = line1.pt1.x;
		pt.y = a2 * pt.x + b2;
	} else if (dx1 != 0) {
		// 傾きa　切片bを求める式
		double a1 = (line1.pt1.y - line1.pt2.y) / dx1;
		double b1 = line1.pt1.y - a1 * line1.pt1.x;

		// 交点を求める
		pt.x = line2.pt1.x;
		pt.y = a1 * pt.x + b1;
	} else {
		pt.x = NAN;
		pt.y = NAN;
	}

	return pt;
}

#endif /* VECTOROP_H_ */
