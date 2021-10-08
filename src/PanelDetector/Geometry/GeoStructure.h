/*
 * (c) Copyright CANON INC. 2016
 * File Name : GeoStructure.h
 * Function : geometry structures
 * Author : DungL , FPT
 * Date : 2016-07-20
 * Last-modified by : DungL, FPT
 * Last-modified : 2016-08-02
 */

#ifndef __GEOMETRY_STRUCTURE__
#define __GEOMETRY_STRUCTURE__

#include "Vector3f.h"

// Plane define by Point and normal
struct PlanePN {
	Vector3F pt;
	//vertex
	Vector3F normal;

	PlanePN() {
	}

	PlanePN(float xPoint, float yPoint, float zPoint, float xNormal,
			float yNormal, float zNormal) {
		pt.setValue(xPoint, yPoint, zPoint);
		normal.setValue(xNormal, yNormal, zNormal);
	}

	void Copy(const PlanePN &Src) {
		pt.copy(Src.pt);
		normal.copy(Src.normal);
	}
};

// Line define points and direction
struct LinePD {
	Vector3F pt;
	Vector3F direction;
};

// Plane define by segment (2 points) and direction
struct Plane2PD {
	//vertex
	Vector3F pt1, pt2;
	Vector3F direction;
};

// Plane define by 3 points
struct Plane3P {
	Vector3F pt1, pt2, pt3;
};

struct Segment3F {
	Vector3F pt1;
	Vector3F pt2;
};

#endif
