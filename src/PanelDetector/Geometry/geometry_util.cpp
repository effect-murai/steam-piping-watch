/********************************************************
 * (c) Copyright 2016
 * File Name : geometry_util.h
 * Function : geometry
 * Author : Le Dung
 * Date : 2016-07-20
 * Last-modified by : Le Dung
 * Last-modified : 2016-08-02
 ********************************************************/

#include "geometry_util.h"
#include <assert.h>
#include <algorithm>

bool compareByX(const Vector2F &e1, const Vector2F &e2) {
	return e1.x < e2.x;
}

// write from http://en.wikipedia.org/wiki/Line%E2%80%93line_intersection
//  3 : ok, intersection is on both segment
//  2 : intersection is on second segment only
//  1 : intersection is on first segment only
//  0 : intersection is out of 2 segments
// -1 : parallel
int twoLineIntersection(float x1, float y1, float x2, float y2, // segment 1-2
		float x3, float y3, float x4, float y4, // segment 3-4
		float &xInterSection, float &yInterSection) {
	float x12 = x1 - x2;
	float x34 = x3 - x4;
	float y12 = y1 - y2;
	float y34 = y3 - y4;

	float c = x12 * y34 - y12 * x34;

	if (fabs(c) < 0.00001f) {
		// No intersection
		return -1;
	} else {
		// Intersection
		float a = x1 * y2 - y1 * x2;
		float b = x3 * y4 - y3 * x4;

		xInterSection = (a * x34 - b * x12) / c;
		yInterSection = (a * y34 - b * y12) / c;

		int res = 0;

		// check is on segment
		if (xInterSection >= std::min(x1, x2)
				&& xInterSection <= std::max(x1, x2)
				&& yInterSection >= std::min(y1, y2)
				&& yInterSection <= std::max(y1, y2)) {
			res |= 1;
		} // first bit

		if (xInterSection >= std::min(x3, x4)
				&& xInterSection <= std::max(x3, x4)
				&& yInterSection >= std::min(y3, y4)
				&& yInterSection <= std::max(y3, y4)) {
			res |= 2;
		} // second bit

		return res;
	}
}

// distance of 2 3d point
float distance(float x1, float y1, float x2, float y2) {
	return sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

// square distance of 2 3d point
float squaredDistance(float x1, float y1, float x2, float y2) {
	return ((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

void getSymmetricPoint(Vector3F point1, Vector3F center, Vector3F &point2) {
	point2.x = 2 * center.x - point1.x;
	point2.y = 2 * center.y - point1.y;
	point2.z = 2 * center.z - point1.z;
}

// ------------------http://geomalgorithms.com/a06-_intersect-2.html
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
		Vector3F *I // intersection
		) {
	Vector3F u, v, n;              // triangle vectors
	Vector3F w0, w;           // ray vectors
	float r, a, b;              // params to calc ray-plane intersect

	// get triangle edge vectors and plane normal
	u = V1 - V0;
	v = V2 - V0;
	n = Vector3F::cross(u, v);              // cross product
	if (n.getLength() < 0.00000001) {             // triangle is degenerate
		return -1;
	}                  // do not deal with this case

	w0 = P0 - V0;
	a = -Vector3F::dot(n, w0);
	b = Vector3F::dot(n, dir);

	if (fabs(b) < 0.000001f) {     // ray is  parallel to triangle plane
		if (a == 0) {              // ray lies in triangle plane
			return 2;
		} else {
			return 0;              // ray disjoint from plane
		}
	}

	// get intersect point of ray with triangle plane
	r = a / b;
	// for a segment, also test if (r > 1.0) => no intersect

	*I = P0 + dir * r;            // intersect point of ray and plane

	// is I inside T?
	float uu, uv, vv, wu, wv, D;
	uu = Vector3F::dot(u, u);
	uv = Vector3F::dot(u, v);
	vv = Vector3F::dot(v, v);
	w = *I - V0;
	wu = Vector3F::dot(w, u);
	wv = Vector3F::dot(w, v);
	D = uv * uv - uu * vv;

	// get and test parametric coords
	float s, t;
	s = (uv * wv - vv * wu) / D;
	if (s < 0.0 || s > 1.0) {      // I is outside T
		return 0;
	}
	t = (uv * wu - uu * wv) / D;
	if (t < 0.0 || (s + t) > 1.0) {
		// I is outside T
		return 0;
	}

	return 1;                      // I is in T
}

// https://en.wikipedia.org/wiki/Rotation_matrix
Vector3F rotate(Vector3F vecIn, Vector3F axis, // rotation axis, should be unit vector (normalized)
		float angle                       // in radian
		) {
	// For some applications, it is helpful to be able to make a rotation with a given axis. Given a unit vector u = (ux, uy, uz), where ux2 + uy2 + uz2 = 1,
	// the matrix for a rotation by an angle of θ about an axis in the direction of u is[3]

	// multiple call, so please normalize this vector first
	Vector3F u = axis;

	double cosPhi = cos(angle);
	double sinPhi = sin(angle);
	double oneMinusCosPhi = 1 - cosPhi;

	double rotateMatrix[9];
	rotateMatrix[0] = cosPhi + u.x * u.x * oneMinusCosPhi;
	rotateMatrix[1] = u.x * u.y * oneMinusCosPhi - u.z * sinPhi;
	rotateMatrix[2] = u.x * u.z * oneMinusCosPhi + u.y * sinPhi;
	rotateMatrix[3] = u.y * u.x * oneMinusCosPhi + u.z * sinPhi;
	rotateMatrix[4] = cosPhi + u.y * u.y * oneMinusCosPhi;
	rotateMatrix[5] = u.y * u.z * oneMinusCosPhi - u.x * sinPhi;
	rotateMatrix[6] = u.z * u.x * oneMinusCosPhi - u.y * sinPhi;
	rotateMatrix[7] = u.z * u.y * oneMinusCosPhi + u.x * sinPhi;
	rotateMatrix[8] = cosPhi + u.z * u.z * oneMinusCosPhi;

	Vector3F vecOut;

	vecOut.x = (float) (rotateMatrix[0] * vecIn.x + rotateMatrix[1] * vecIn.y
			+ rotateMatrix[2] * vecIn.z);
	vecOut.y = (float) (rotateMatrix[3] * vecIn.x + rotateMatrix[4] * vecIn.y
			+ rotateMatrix[5] * vecIn.z);
	vecOut.z = (float) (rotateMatrix[6] * vecIn.x + rotateMatrix[7] * vecIn.y
			+ rotateMatrix[8] * vecIn.z);

	return vecOut;    // ok
}

// make sure matrices has 16 float size
void matrix3Muliply(float *left, float *right, float *result) {
	result[0] = left[0] * right[0] + left[1] * right[3] + left[2] * right[6];
	result[1] = left[0] * right[1] + left[1] * right[4] + left[2] * right[7];
	result[2] = left[0] * right[2] + left[1] * right[5] + left[2] * right[8];

	result[3] = left[3] * right[0] + left[4] * right[3] + left[5] * right[6];
	result[4] = left[3] * right[1] + left[4] * right[4] + left[5] * right[7];
	result[5] = left[3] * right[2] + left[4] * right[5] + left[5] * right[8];

	result[6] = left[6] * right[0] + left[7] * right[3] + left[8] * right[6];
	result[7] = left[6] * right[1] + left[7] * right[4] + left[8] * right[7];
	result[8] = left[6] * right[2] + left[7] * right[5] + left[8] * right[8];
}

// make sure matrices has 16 float size
void matrix3Transpose(float *in, float *out) {
	out[0] = in[0];
	out[1] = in[3];
	out[2] = in[6];
	out[3] = in[1];
	out[4] = in[4];
	out[5] = in[7];
	out[6] = in[2];
	out[7] = in[5];
	out[8] = in[8];
}

// make sure matrices has 16 float size
void matrix4Muliply(float *left, float *right, float *result) {
	result[0] = left[0] * right[0] + left[4] * right[1] + left[8] * right[2]
			+ left[12] * right[3];
	result[4] = left[0] * right[4] + left[4] * right[5] + left[8] * right[6]
			+ left[12] * right[7];
	result[8] = left[0] * right[8] + left[4] * right[9] + left[8] * right[10]
			+ left[12] * right[11];
	result[12] = left[0] * right[12] + left[4] * right[13] + left[8] * right[14]
			+ left[12] * right[15];

	result[1] = left[1] * right[0] + left[5] * right[1] + left[9] * right[2]
			+ left[13] * right[3];
	result[5] = left[1] * right[4] + left[5] * right[5] + left[9] * right[6]
			+ left[13] * right[7];
	result[9] = left[1] * right[8] + left[5] * right[9] + left[9] * right[10]
			+ left[13] * right[11];
	result[13] = left[1] * right[12] + left[5] * right[13] + left[9] * right[14]
			+ left[13] * right[15];

	result[2] = left[2] * right[0] + left[6] * right[1] + left[10] * right[2]
			+ left[14] * right[3];
	result[6] = left[2] * right[4] + left[6] * right[5] + left[10] * right[6]
			+ left[14] * right[7];
	result[10] = left[2] * right[8] + left[6] * right[9] + left[10] * right[10]
			+ left[14] * right[11];
	result[14] = left[2] * right[12] + left[6] * right[13]
			+ left[10] * right[14] + left[14] * right[15];

	result[3] = left[3] * right[0] + left[7] * right[1] + left[11] * right[2]
			+ left[15] * right[3];
	result[7] = left[3] * right[4] + left[7] * right[5] + left[11] * right[6]
			+ left[15] * right[7];
	result[11] = left[3] * right[8] + left[7] * right[9] + left[11] * right[10]
			+ left[15] * right[11];
	result[15] = left[3] * right[12] + left[7] * right[13]
			+ left[11] * right[14] + left[15] * right[15];
}

// make sure matrices has 16 float size
void matrix4Transpose(float *in, float *out) {
	out[0] = in[0];
	out[1] = in[4];
	out[2] = in[8];
	out[3] = in[12];
	out[4] = in[1];
	out[5] = in[5];
	out[6] = in[9];
	out[7] = in[13];
	out[8] = in[2];
	out[9] = in[6];
	out[10] = in[10];
	out[11] = in[14];
	out[12] = in[3];
	out[13] = in[7];
	out[14] = in[11];
	out[15] = in[15];
}

// Ray-casting
// arPoint : x0, y0, x1, y1, ...
// nvert : num vertices
bool isPointInPolygon(float x, float y, float *arPoint, int nvert) {
	int i, j;
	bool c = false;

	for (i = 0, j = nvert - 1; i < nvert; j = i++) {
		if ((((arPoint[(i << 1) + 1]) >= y) != (arPoint[(j << 1) + 1] >= y))
				&& (x
						<= (arPoint[j << 1] - arPoint[i << 1])
								* (y - arPoint[(i << 1) + 1])
								/ (arPoint[(j << 1) + 1] - arPoint[(i << 1) + 1])
								+ arPoint[i << 1])) {
			c = !c;
		}
	}

	return c;
}

// InsideTriangle decides if a point P is Inside of the triangle
// defined by A, B, C.
bool isInsideTriangle(float px, float py, float ax, float ay, float bx,
		float by, float cx, float cy) {
	float minX = std::min(std::min(ax, bx), cx);
	float maxX = std::max(std::max(ax, bx), cx);

	float minY = std::min(std::min(ay, by), cy);
	float maxY = std::max(std::max(ay, by), cy);

	if (px < minX || px > maxX) {
		return false;
	}
	if (py < minY || py > maxY) {
		return false;
	}

	float bcx, bcy, cax, cay, abx, aby, apx, apy, bpx, bpy, cpx, cpy;
	float cCROSSap, bCROSScp, aCROSSbp;

	bcx = cx - bx;
	bcy = cy - by;
	cax = ax - cx;
	cay = ay - cy;
	abx = bx - ax;
	aby = by - ay;
	apx = px - ax;
	apy = py - ay;
	bpx = px - bx;
	bpy = py - by;
	cpx = px - cx;
	cpy = py - cy;

	aCROSSbp = bcx * bpy - bcy * bpx;
	cCROSSap = abx * apy - aby * apx;
	bCROSScp = cax * cpy - cay * cpx;

	return ((aCROSSbp >= -CLOCKWISE_THRESHOLD)
			&& (bCROSScp >= -CLOCKWISE_THRESHOLD)
			&& (cCROSSap >= -CLOCKWISE_THRESHOLD));
}
;

bool isInsideTriangle1(float px, float py, float ax, float ay, float bx,
		float by, float cx, float cy) {
	float minX = std::min(std::min(ax, bx), cx);
	float maxX = std::max(std::max(ax, bx), cx);

	float minY = std::min(std::min(ay, by), cy);
	float maxY = std::max(std::max(ay, by), cy);

	if (px < minX || px > maxX) {
		return false;
	}
	if (py < minY || py > maxY) {
		return false;
	}

	bool c = false;

	double xTmp = ax
			+ ((double) bx - ax) * ((double) py - ay) / ((double) by - ay);
	if ((ay >= py) != (by >= py)) {
		if (fabs(px - xTmp) < RAW_SAME_THRESHOLD) {
			// on edge -> consider as in triangle
			return true;
		} else if (px <= xTmp) {
			c = !c;
		}
	}

	xTmp = bx + ((double) cx - bx) * ((double) py - by) / ((double) cy - by);
	if ((by >= py) != (cy >= py)) {
		if (fabs(px - xTmp) < RAW_SAME_THRESHOLD) {
			// on edge -> consider as in triangle
			return true;
		} else if (px <= xTmp) {
			c = !c;
		}
	}

	xTmp = cx + ((double) ax - cx) * ((double) py - cy) / ((double) ay - cy);
	if ((cy >= py) != (ay >= py)) {
		if (fabs(px - xTmp) < RAW_SAME_THRESHOLD) {
			// on edge -> consider as in triangle
			return true;
		} else if (px <= xTmp) {
			c = !c;
		}
	}

	return c;
}

bool isInsideTriangleExceptVertices(float px, float py, float ax, float ay,
		float bx, float by, float cx, float cy) {
	bool res = isInsideTriangle1(px, py, ax, ay, bx, by, cx, cy);

	if (!res) {
		return false;
	}

	// if point is one of vertices -> out
	if (isSamePoint(px, py, ax, ay) || isSamePoint(px, py, bx, by)
			|| isSamePoint(px, py, cx, cy)) {
		return false;
	}

	return true;
}

// 3 : ok, intersection is on both segment
// 2 : intersection is on second segment only
// 1 : intersection is on first segment only
// -1 : parallel
// -2 : no intersection (out of bound)
int fastTwoLineIntersection(float x1, float y1, float x2, float y2, // segment 1-2
		float x3, float y3, float x4, float y4,    // segment 3-4
		float &xInterSection, float &yInterSection) {
	if (std::max(x1, x2) < std::min(x3, x4)) {
		return -2;
	}

	if (std::min(x1, x2) > std::max(x3, x4)) {
		return -2;
	}

	if (std::max(y1, y2) < std::min(y3, y4)) {
		return -2;
	}

	if (std::min(y1, y2) > std::max(y3, y4)) {
		return -2;
	}

	return twoLineIntersection(x1, y1, x2, y2, x3, y3, x4, y4, xInterSection,
			yInterSection);
}

// 4 : intersection is on 1 of ends of 2 segment
// 3 : ok, intersection is on both segment
// 2 : intersection is on second segment only
// 1 : intersection is on first segment only
// -1 : parallel
// -2 : no intersection (out of bound)
int fastTwoLineIntersectionExceptEnd(float x1, float y1, float x2, float y2, // segment 1-2
		float x3, float y3, float x4, float y4,    // segment 3-4
		float &xInterSection, float &yInterSection) {
	if (std::max(x1, x2) < std::min(x3, x4)) {
		return -2;
	}

	if (std::min(x1, x2) > std::max(x3, x4)) {
		return -2;
	}

	if (std::max(y1, y2) < std::min(y3, y4)) {
		return -2;
	}

	if (std::min(y1, y2) > std::max(y3, y4)) {
		return -2;
	}

	int res = twoLineIntersection(x1, y1, x2, y2, x3, y3, x4, y4, xInterSection,
			yInterSection);

	if (res != 3) {
		return res;
	}

	// check the ends
	if (isSamePoint(xInterSection, yInterSection, x1, y1)
			|| isSamePoint(xInterSection, yInterSection, x2, y2)
			|| isSamePoint(xInterSection, yInterSection, x3, y3)
			|| isSamePoint(xInterSection, yInterSection, x4, y4)) {
		return 4;
	}

	return 3;
}

// https://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain
// Returns a list of points on the convex hull in counter-clockwise order.
// Note: the last point in the returned list is the same as the first one.
std::vector<Vector2F> convexHull(std::vector<Vector2F> p) {
	int n = p.size(), k = 0;
	std::vector<Vector2F> h(2 * n);

	// Sort points lexicographically
	std::sort(p.begin(), p.end(), compareByX);

	// Build lower hull
	for (int i = 0; i < n; ++i) {
		while (k >= 2
				&& Vector2F::cross(h[k - 1] - h[k - 2], p[i] - h[k - 1]) <= 0) {
			k--;
		}
		h[k++] = p[i];
	}

	// Build upper hull
	for (int i = n - 2, t = k + 1; i >= 0; i--) {
		while (k >= t
				&& Vector2F::cross(h[k - 1] - h[k - 2], p[i] - h[k - 1]) <= 0) {
			k--;
		}
		h[k++] = p[i];
	}

	h.resize(k);
	return h;
}

// use for points already sort by y
std::vector<Vector2F> convexHullY(Vector2F *p, int size) {
	int n = size, k = 0;
	std::vector<Vector2F> h(2 * n);

	// Build right hull
	for (int i = 0; i < n; ++i) {
		while (k >= 2
				&& Vector2F::cross(h[k - 1] - h[k - 2], p[i] - h[k - 1]) <= 0) {
			k--;
		}
		h[k++] = p[i];
	}

	// Build left hull
	for (int i = n - 2, t = k + 1; i >= 0; i--) {
		while (k >= t
				&& Vector2F::cross(h[k - 1] - h[k - 2], p[i] - h[k - 1]) <= 0) {
			k--;
		}
		h[k++] = p[i];
	}

	h.resize(k);
	return h;
}

float invSqrt(float x) {
	float xhalf = 0.5f * x;
	int i = *(int*) &x; // get bits for floating value
	i = 0x5f3759df - (i >> 1); // gives initial guess y0
	x = *(float*) &i; // convert bits back to float
	x = x * (1.5f - xhalf * x * x); // Newton step, repeating increases accuracy
	return x;
}

//	Calculate the signed area: A = 1/2 * (x1*y2 - x2*y1 + x2*y3 - x3*y2 + ... + xn*y1 - x1*yn)
// source http:// mathworld.wolfram.com/PolygonArea.html
bool isClockwise(float *arPoint, int num) {
	float signedArea = 0;
	float x1, y1, x2, y2;
	for (int i = 0; i < num - 1; i++) {
		x1 = arPoint[i << 1];
		y1 = arPoint[(i << 1) + 1];
		x2 = arPoint[(i + 1) << 1];
		y2 = arPoint[((i + 1) << 1) + 1];
		signedArea += (x1 * y2 - x2 * y1);
	}

	x1 = arPoint[(num - 1) << 1];
	y1 = arPoint[((num - 1) << 1) + 1];
	x2 = arPoint[0];
	y2 = arPoint[1];
	signedArea += (x1 * y2 - x2 * y1);

	// return signedArea / 2;
	return signedArea > 0;
}

// 1 : clockwise
// 0 : parallel
// -1 : counter-clockwise
// -2 : segment 12 has 0 lengh
// -3 : segment 34 has 0 lengh
int isClockwise(float x1, float y1, float x2, float y2, float x3, float y3,
		float x4, float y4) {

	if (isSamePoint(x1, y1, x2, y2)) {
		return -2;
	}

	if (isSamePoint(x3, y3, x4, y4)) {
		return -3;
	}

	// make this procedure independent from the threshold value by normalize vector
	Vector2F v1(x2 - x1, y2 - y1);
	Vector2F v2(x4 - x3, y4 - y3);

	assert(v1.x != 0 || v1.y != 0);
	assert(v2.x != 0 || v2.y != 0);

	v1 = Vector2F::normalize(v1);
	v2 = Vector2F::normalize(v2);
	float cross = Vector2F::cross(v1, v2);

	if (fabs(cross) < RAW_SAME_THRESHOLD) {
		return 0;
	}

	return (cross > 0) ? 1 : -1;	// cause y screen downward
}

//  3 points
//  1 : clockwise
// -1 : counter-clockwise
//  0 : thang hang
int isClockwise(float x1, float y1,	// point 1
		float x2, float y2,	// point 2
		float x3, float y3	// point 3
		) {
	// tinh cross
	float cross = (x2 - x1) * (y3 - y2) - (x3 - x2) * (y2 - y1);

	float eps = 1e-4;

	if (cross < -eps) {
		return -1;	// counter-clockwise
	} else if (cross > eps) {
		return 1;	// clockwise
	} else {
		return 0;	// thang hang
	}
}

// angle 2 between segment 2-1 va 2-3
float angle2Line(float x1, float y1, float x2, float y2, float x3, float y3) {
	// faster
	float x21 = x1 - x2;
	float x23 = x3 - x2;
	float y21 = y1 - y2;
	float y23 = y3 - y2;
	float dot = x21 * x23 + y21 * y23;

	return acos(
			dot * invSqrt((x21 * x21 + y21 * y21) * (x23 * x23 + y23 * y23)));
	// to speedup acos need to be pre-calculated in 0-3600 range e.g in 0.1 degree
}

// angle 2 between to vector
float angle2Line(Vector2F v1, Vector2F v2) {
	float dot = Vector2F::dot(v1, v2);
	return acos(
			dot
					* invSqrt(
							(v1.x * v1.x + v1.y * v1.y)
									* (v2.x * v2.x + v2.y * v2.y)));
	// to speedup acos need to be pre-calculated in 0-3600 range e.g in 0.1 degree
}

bool isSamePoint(float x1, float y1, float x2, float y2) {
	if (fabs(x1 - x2) < SAME_THRESHOLD && fabs(y1 - y2) < SAME_THRESHOLD) {
		return true;
	} else {
		return false;
	}
}

Vector2F projectionPointOnLine(Vector2F pt, Vector2F linePoint,
		Vector2F lineDirection) {
	assert(lineDirection.x != 0 || lineDirection.y != 0);

	Vector2F vLine = Vector2F::normalize(lineDirection);
	float projectValue = Vector2F::dot(vLine, pt - linePoint);
	Vector2F projectionPt = linePoint + vLine * projectValue;
	return projectionPt;
}

// goi ham nay neu lineDirection da duoc normalize truoc
Vector2F projectionPointOnLine1(Vector2F pt, Vector2F linePoint,
		Vector2F lineDirection) {
	assert(lineDirection.x != 0 || lineDirection.y != 0);

	float projectValue = Vector2F::dot(lineDirection, pt - linePoint);
	Vector2F projectionPt = linePoint + lineDirection * projectValue;
	return projectionPt;
}

Vector2F projectionPointOnLine2D(Vector2F pt, Vector2F linePoint,
		Vector2F lineDirection) {
	assert(lineDirection.x != 0 || lineDirection.y != 0);

	Vector2F vLine = Vector2F::normalize(lineDirection);
	float projectValue = Vector2F::dot(vLine, pt - linePoint);
	Vector2F projectionPt = linePoint + vLine * projectValue;
	return projectionPt;
}

float pistancePoint2Line2D(Vector2F pt, Vector2F linePoint,
		Vector2F lineDirection) {
	Vector2F ptProj = projectionPointOnLine2D(pt, linePoint, lineDirection);
	return Vector2F::distance(ptProj, pt);
}

// distance from point (x, y) to line y = a*x + b
float distancePointToLine(int x, int y, float a, float b) {
	return fabs(a * x + b - y) / sqrt(a * a + 1);
}

// square distance from point (x, y) to line y = a*x + b
float squareDistancePointToLine(int x, int y, float a, float b) {
	float temp = a * x + b - y;
	return temp * temp / (a * a + 1);
}

bool isInRect(float x, float y, float left, float top, float width,
		float height) {
	if (x < left || x > left + width || y < top || y > top + height) {
		return false;
	}

	return true;
}

float overlapArea2Rectangles(float left1, float top1, float width1,
		float height1, float left2, float top2, float width2, float height2) {
	float left = std::max(left1, left2);
	float right = std::min(left1 + width1, left2 + width2);
	float top = std::max(top1, top2);
	float bottom = std::min(top1 + height1, top2 + height2);

	return (right - left) * (bottom - top);
}

float areaTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
	float area = fabs((x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1)) * 0.5f;
	return area;
}

float areaQuad(float x1, float y1, float x2, float y2, float x3, float y3,
		float x4, float y4) {
	float area1 = fabs((x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1)) * 0.5f;
	float area2 = fabs((x4 - x1) * (y3 - y1) - (x3 - x1) * (y4 - y1)) * 0.5f;
	return (area1 + area2);
}
