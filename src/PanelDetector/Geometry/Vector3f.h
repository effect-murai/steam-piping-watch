/*
 * (c) Copyright 2016
 * File Name : Vector3F
 * Function : Vector3F
 * Author : Le Dung
 * Date : 2015-08-13
 * Last-modified by : Le Dung
 * Last-modified : 2016-10-31
 */

#ifndef __VECTOR3F__
#define __VECTOR3F__

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

#define RADIANS_TO_DEGREES(radians) ((radians) * (180.0 / M_PI))
#define DEGREES_TO_RADIANS(angle) ((angle) / 180.0 * M_PI)

struct Vector3F {
	float x, y, z;

	Vector3F() {
		x = y = z = 0;
	}

	Vector3F(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	void setValue(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	void operator=(const Vector3F &v) {
		x = v.x;
		y = v.y;
		z = v.z;
	}

	static Vector3F normalize(const Vector3F &v) {
		float length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		return Vector3F(v.x / length, v.y / length, v.z / length);
	}

	static float distance(const Vector3F &v1, const Vector3F &v2) {
		return sqrt(
				(v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y)
						+ (v2.z - v1.z) * (v2.z - v1.z));
	}

	static float squaredDistance(const Vector3F &v1, const Vector3F &v2) {
		return ((v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y)
				+ (v2.z - v1.z) * (v2.z - v1.z));
	}

	float getLength() {
		return sqrt(x * x + y * y + z * z);
	}

	Vector3F operator-(const Vector3F &v) {
		return Vector3F(x - v.x, y - v.y, z - v.z);
	}

	Vector3F operator+(const Vector3F &v) {
		return Vector3F(x + v.x, y + v.y, z + v.z);
	}

	Vector3F operator*(float a) {
		return Vector3F(a * x, a * y, a * z);
	}

	Vector3F operator*(double a) {
		return Vector3F((float) (a * x), (float) (a * y), (float) (a * z));
	}

	Vector3F operator/(float a) {
		if (a != 0) {
			return Vector3F(x / a, y / a, z / a);
		} else {
			return Vector3F(0, 0, 0);
		}
	}

	Vector3F operator/(double a) {
		if (a != 0) {
			return Vector3F((float) (x / a), (float) (y / a), (float) (z / a));
		} else {
			return Vector3F(0, 0, 0);
		}
	}

	static float dot(const Vector3F &v1, const Vector3F &v2) {
		return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
	}

	static Vector3F cross(const Vector3F &v1, const Vector3F &v2) {
		return Vector3F(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z,
				v1.x * v2.y - v1.y * v2.x);
	}

	static Vector3F crossNormalized(const Vector3F &v1, const Vector3F &v2) {
		return cross(normalize(v1), normalize(v2));
	}

	static Vector3F average(const Vector3F &v1, const Vector3F &v2) {
		return Vector3F((v1.x + v2.x) / 2, (v1.y + v2.y) / 2, (v1.z + v2.z) / 2);
	}

	Vector3F inverse() {
		return Vector3F(-x, -y, -z);
	}

	void copy(const Vector3F &src) {
		x = src.x;
		y = src.y;
		z = src.z;
	}
};

#endif
