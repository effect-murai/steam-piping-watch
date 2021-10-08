/*
 * (c) Copyright 2016
 * File Name : geometry_util.h
 * Function : geometry
 * Author : Le Dung
 * Date : 2016-07-20
 * Last-modified by : Le Dung
 * Last-modified : 2016-08-02
 */

#ifndef __GEOMETRY_UTIL__
#define __GEOMETRY_UTIL__

#include <vector>
#include "GeoStructure.h"
#include "Vector2f.h"

#define SAME_THRESHOLD	0.00001f
#define RAW_SAME_THRESHOLD	0.0001f
#define CLOCKWISE_THRESHOLD	0.001f

//  3 : ok, intersection is on both segment
//  2 : intersection is on second segment only
//  1 : intersection is on first segment only
//  0 : intersection is out of 2 segments
// -1 : parallel
int twoLineIntersection(
// In
		float x1, float y1, float x2, float y2, // segment 1-2
		float x3, float y3, float x4, float y4, // segment 3-4
		// Out
		float &xInterSection, float &yInterSection);

float distance(float x1, float y1, float x2, float y2);

float squaredDistance(float x1, float y1, float x2, float y2);

void getSymmetricPoint(Vector3F point1, Vector3F center, Vector3F &point2);

// ------------------http:// geomalgorithms.com/a06-_intersect-2.html
// intersect3D_RayTriangle(): find the 3D intersection of a ray with a triangle
//    Input:  a ray R, and a triangle T
//    Output: *I = intersection point (when it exists)
//    Return: -1 = triangle is degenerate (a segment or point)
//             0 =  disjoint (no intersect)
//             1 =  intersect in unique point I1
//             2 =  are in the same plane
int Intersect3D_RayTriangle(Vector3F V0,  // Triangle vertices
		Vector3F V1, Vector3F V2, Vector3F P0,  // Ray origin
		Vector3F dir,  // Ray direction
		Vector3F *I     // intersection
		);

Vector3F rotate(
// In
		Vector3F vecIn, Vector3F axis, // rotation axis, should be unit vector (normalized)
		float angle     // in radian
		);

void matrix4Muliply(float *left, float *right, float *result);

void matrix4Transpose(float *in, float *out);

void matrix3Muliply(float *left, float *right, float *result);

void matrix3Transpose(float *in, float *out);

// ----------------------------------------Funtion---------------------------------------
bool isSamePoint(float x1, float y1, float x2, float y2);

// check polygon clockwise
bool isClockwise(float *arPoint, int num);

// check 2 line segment clockwise
int isClockwise(float x1, float y1, float x2, float y2,     // segment 1-2
		float x3, float y3, float x4, float y4     // segment 3-4)
		);

//  3 : ok, intersection is on both segment
//  2 : intersection is on second segment only
//  1 : intersection is on first segment only
//  0 : intersection is not on any segment
// -1 : parallel
int twoLineIntersection(
// In
		float x1, float y1, float x2, float y2,     // segment 1-2
		float x3, float y3, float x4, float y4,     // segment 3-4
		// Out
		float &xInterSection, float &yInterSection);
int fastTwoLineIntersection(
// In
		float x1, float y1, float x2, float y2,     // segment 1-2
		float x3, float y3, float x4, float y4,     // segment 3-4
		// Out
		float &xInterSection, float &yInterSection);
int fastTwoLineIntersectionExceptEnd(
// In
		float x1, float y1, float x2, float y2,     // segment 1-2
		float x3, float y3, float x4, float y4,     // segment 3-4
		// Out
		float &xInterSection, float &yInterSection);

bool isInsideTriangle(float px, float py, float ax, float ay, float bx,
		float by, float cx, float cy);
bool isInsideTriangleExceptVertices(float Px, float Py, float Ax, float Ay,
		float Bx, float By, float Cx, float Cy);
bool isPointInPolygon(float x, float y, float *arPoint, int nvert);

// Returns a list of points on the convex hull in counter-clockwise order.
// Note: the last point in the returned list is the same as the first one.
std::vector<Vector2F> convexHull(std::vector<Vector2F> P);

// use for points already sort by y
std::vector<Vector2F> convexHullY(Vector2F *P, int size);

float invSqrt(float x);

// 3 points
// -1 : counter-clockwise
// 1 : clockwise
// 0 : thang hang
int isClockwise(float x1, float y1,     // point 1
		float x2, float y2,     // point 2
		float x3, float y3     // point 3
		);

// angle 2 between segment 2-1 va 2-3
float angle2Line(float x1, float y1,     // point 1
		float x2, float y2,     // point 2
		float x3, float y3     // point 3
		);

// angle 2 between to vector
float angle2Line(Vector2F v1, Vector2F v2);

Vector2F projectionPointOnLine(Vector2F pt, // point
		// line
		Vector2F linePoint, Vector2F lineDirection);

// goi ham nay neu lineDirection da duoc normalize truoc
Vector2F projectionPointOnLine1(Vector2F pt, // point
		// line
		Vector2F linePoint, Vector2F lineDirection);

Vector2F projectionPointOnLine2D(Vector2F pt, // point
											  // line
		Vector2F linePoint, Vector2F lineDirection);

float pistancePoint2Line2D(Vector2F pt, // point
										// line
		Vector2F linePoint, Vector2F lineDirection);

float distancePointToLine(int x, int y,	// point
		float a, float b  // line
		);

// square distance from point (x, y) to line y = a*x + b
float squareDistancePointToLine(int x, int y,	// point
		float a, float b  // line
		);

bool isInRect(float x, float y,  // point
		float left, float top,  // rect
		float width, float height);

float overlapArea2Rectangles(float left1, float top1, float width1,
		float height1, float left2, float top2, float width2, float height2);

float areaTriangle(float x1, float y1, float x2, float y2, float x3, float y3);

float areaQuad(float x1, float y1, float x2, float y2, float x3, float y3,
		float x4, float y4);

#endif
