/*
 * (c) Copyright 2016
 * File Name : Vector2F
 * Function : Vector2F
 * Author : Le Dung
 * Date : 2015-08-13
 * Last-modified by : Le Dung
 * Last-modified : 2016-10-31
 */

#ifndef __VECTOR2F__
#define __VECTOR2F__

#include <math.h>

#ifndef SAFE_DELETE
#define SAFE_DELETE(a) { delete (a); (a) = NULL; }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(ptr) { if(ptr) { delete [](ptr); (ptr)=NULL; } }
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2     1.57079632679489661923
#endif

#ifndef M_PI_4
#define M_PI_4     0.785398163397448309616
#endif

#define RADIANS_TO_DEGREES(radians) ((radians) * (180.0 / M_PI))
#define DEGREES_TO_RADIANS(angle) ((angle) / 180.0 * M_PI)

struct Vector2F {
	float x, y;

	Vector2F() {
		x = y = 0;
	}

	Vector2F(float xx, float yy) {
		x = xx;
		y = yy;
	}

	void operator=(const Vector2F &v) {
		x = v.x;
		y = v.y;
	}

	Vector2F operator-(const Vector2F &v) {
		return Vector2F(x - v.x, y - v.y);
	}

	Vector2F operator+(const Vector2F &v) {
		return Vector2F(x + v.x, y + v.y);
	}

	void operator+=(const Vector2F &v) {
		x += v.x;
		y += v.y;
	}

	Vector2F operator*(float a) {
		return Vector2F(a * x, a * y);
	}

	Vector2F operator/(float a) {
		if (a != 0) {
			return Vector2F(x / a, y / a);
		} else {
			return Vector2F(0, 0);
		}
	}

	static Vector2F normalize(const Vector2F &v) {
		float length = sqrt(v.x * v.x + v.y * v.y);
		return Vector2F(v.x / length, v.y / length);
	}

	static float cross(const Vector2F &v1, const Vector2F &v2) {
		return (v1.x * v2.y - v2.x * v1.y);
	}

	static float dot(const Vector2F &v1, const Vector2F &v2) {
		return (v1.x * v2.x + v1.y * v2.y);
	}

	static float distance(const Vector2F &v1, const Vector2F &v2) {
		return sqrt(
				(v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y));
	}

	static float squaredDistance(const Vector2F &v1, const Vector2F &v2) {
		return ((v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y));
	}

	float getLength() {
		return sqrt(x * x + y * y);
	}

	static float crossNormalized(const Vector2F &v1, const Vector2F &v2) {
		return cross(normalize(v1), normalize(v2));
	}

	static Vector2F average(const Vector2F &v1, const Vector2F &v2) {
		return Vector2F((v1.x + v2.x) / 2, (v1.y + v2.y) / 2);
	}

	Vector2F inverse() {
		return Vector2F(-x, -y);
	}

	void copy(const Vector2F &src) {
		x = src.x;
		y = src.y;
	}

	void transpose() {
		float tmp = x;
		x = y;
		y = tmp;
	}
};

#endif
