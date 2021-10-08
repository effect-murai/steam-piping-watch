/*
 * (c) Copyright CANON INC. 2016
 * File Name : DCoordinateChanger.cpp
 * Function : DCoordinateChanger
 * Author : DungL , FPT
 * Date : 2016-12-07
 * Last-modified by : DungL, FPT
 * Last-modified : 2016-12-07
 */

#include <cstring>
#include "DCoordinateChanger2d.h"
#include "geometry_util.h"

#define _USE_MATH_DEFINES
#include <cmath>

DCoordinateChanger2D::DCoordinateChanger2D() {
}

DCoordinateChanger2D::~DCoordinateChanger2D() {
}

// convert point from coordinate system (x, y, z) to new coordinate system (u, v, w)
// apply transform for tha point as for new coord system
Vector2F DCoordinateChanger2D::Convert2NewCoordSystem(Vector2F pointXY) {
	Vector2F pointUV;

	pointUV.x = mM[0] * pointXY.x + mM[1] * pointXY.y + mM[2];
	pointUV.y = mM[3] * pointXY.x + mM[4] * pointXY.y + mM[5];

	return pointUV;
}

// convert point from coordinate system (x, y, z) to new coordinate system (u, v, w)
// new values relative new coordinate system
Vector2F DCoordinateChanger2D::Convert2NewCoordSystem2(Vector2F pointXY) {
	Vector2F pointUV;
	Vector2F pointRelativeNew = pointXY - mNewCoordOrigin;
	pointUV.x = Vector2F::dot(pointRelativeNew, mNewCoordU);
	pointUV.y = Vector2F::dot(pointRelativeNew, mNewCoordV);

	return pointUV;
}

// convert point from new coordinate system (u, v, w) to coordinate system (x, y, z)
Vector2F DCoordinateChanger2D::Convert2OriginCoordSystem(Vector2F pointUV) {
	Vector2F pointXY;

	pointXY.x = mM_1[0] * pointUV.x + mM_1[1] * pointUV.y + mM_1[2];
	pointXY.y = mM_1[3] * pointUV.x + mM_1[4] * pointUV.y + mM_1[5];

	return pointXY;
}

void DCoordinateChanger2D::SetNewCoordSystemValue(Vector2F newCoordOrigin,
		Vector2F newCoordU, Vector2F newCoordV) {
	mNewCoordOrigin = newCoordOrigin;
	mNewCoordU = newCoordU; // should be normalized before
	mNewCoordV = newCoordV; // should be normalized before

	// Solution: M=RT where T is a
	// translation matrix by (x0,y0,z0), and
	// R is rotation matrix whose columns
	// are U,V, and W:

	float r[9]; // Rotation matrix
	float t[9]; // translation matrix

	r[0] = newCoordU.x;
	r[1] = newCoordV.x;
	r[2] = 0;
	r[3] = newCoordU.y;
	r[4] = newCoordV.y;
	r[5] = 0;
	r[6] = 0;
	r[7] = 0;
	r[8] = 1;

	t[0] = 1;
	t[1] = 0;
	t[2] = newCoordOrigin.x;
	t[3] = 0;
	t[4] = 1;
	t[5] = newCoordOrigin.y;
	t[6] = 0;
	t[7] = 0;
	t[8] = 1;

	matrix3Muliply(r, t, mM);
	matrix3Transpose(mM, mM_1);
}
