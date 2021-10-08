/*
 * (c) Copyright CANON INC. 2016
 * File Name : DCoordinateChanger.java
 * Function : DCoordinateChanger
 * Author : DungL , FPT
 * Date : 2016-12-07
 * Last-modified by : DungL, FPT
 * Last-modified : 2016-12-07
 */

#ifndef TESTF_DCOORDINATECHANGER2D_H
#define TESTF_DCOORDINATECHANGER2D_H

#include "Vector2F.h"

//Use to convert the point from the coordinate system (x, y, z) to new coordinate system (u, v, w) and vice versus
//Based on http://www.math.tau.ac.il/~dcor/Graphics/cg-slides/geom3d.pdf

class DCoordinateChanger2D {
public:
	DCoordinateChanger2D();
	~DCoordinateChanger2D();

	void SetNewCoordSystemValue(Vector2F newCoordOrigin, Vector2F newCoordU,
			Vector2F newCoordV);

	//convert point from coordinate system (x, y, z) to new coordinate system (u, v, w)
	Vector2F Convert2NewCoordSystem(Vector2F pointXY);
	Vector2F Convert2NewCoordSystem2(Vector2F pointXY);

	//convert point from new coordinate system (u, v, w) to coordinate system (x, y, z)
	Vector2F Convert2OriginCoordSystem(Vector2F pointUV);

private:
	Vector2F mNewCoordOrigin;
	Vector2F mNewCoordU;
	Vector2F mNewCoordV;

	float mM[9];
	float mM_1[9]; //transpose of M
};

#endif //TESTF_DCOORDINATECHANGER_H
