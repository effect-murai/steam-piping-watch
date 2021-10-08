/*
 * Author : Le Dung , FPT
 * Date : 2018-06-15
 * Last-modified by : Le Dung, FPT
 * Last-modified : 2018-06-15
 */

#ifndef __DSTRUCT__
#define __DSTRUCT__

#include <algorithm> 
#include "UtilFuncs.h"
#include "../Geometry/geometry_util.h"

struct LinePolar {
	float rho;
	float theta;
};

struct LineLinear {
	float a;
	float b;
};

struct RectF {
	// clock-wise
	float x1, y1;
	float x2, y2;
	float x3, y3;
	float x4, y4;
};

struct Rect3: public RectF {
	float width;
	float height;
	float ratio;

	float minX, maxX;
	float minY, maxY;

	float parallelError;

	int viewMode; // 0:landscape, 1:portrait

	Rect3();

	void calcSize() {
		width = (distance(x1, y1, x2, y2) + distance(x3, y3, x4, y4)) / 2;
		height = (distance(x2, y2, x3, y3) + distance(x1, y1, x4, y4)) / 2;
		if (height > width) {
			float tmp = width;
			width = height;
			height = tmp;

			viewMode = 1;
		}
	}

	void calcRatio() {
		if (width > height)
			ratio = width / height;
		else
			ratio = height / width;
	}

	void swap(float &v1, float &v2) {
		float tmp = v1;
		v1 = v2;
		v2 = tmp;
	}

	void swapPoint(float &x1, float &y1, float &x2, float &y2) {
		swap(x1, x2);
		swap(y1, y2);
	}

	void normalizeRotation() {
		bool clockWise = isClockwise((float*) this, 4);

		if (!clockWise) {
			swapPoint(x2, y2, x4, y4);
			float tmp = width;
			width = height;
			height = tmp;
		}
	}

	void normalizePosition() {
		if (width > height) {
			if (x1 > x3) {
				swapPoint(x1, y1, x3, y3);
				swapPoint(x2, y2, x4, y4);
			}
		} else {
			float tmpX = x1;
			float tmpY = y1;
			x1 = x2;
			y1 = y2;
			x2 = x3;
			y2 = y3;
			x3 = x4;
			y3 = y4;
			x4 = tmpX;
			y4 = tmpY;

			tmpX = width;
			width = height;
			height = tmpX;

			normalizePosition();
		}
	}

	void calcBound();
};

struct PointI {
	int x, y;
	PointI() {
		x = y = 0;
	}

	PointI(int x_, int y_) {
		x = x_;
		y = y_;
	}

	void transpose() {
		int tmp = x;
		x = y;
		y = tmp;
	}

	bool isSame(const PointI &pt) {
		return (x == pt.x && y == pt.y);
	}
};

struct DPoint {
	int type;
	int x;
	int lenx;
	int xf;
	int lenxf;
	int y;
	float yf;

	DPoint() {
		empty();
	}

	void empty() {
		type = 0;
		x = -1;
		lenx = 0;
		xf = -1;
		lenxf = 0;
		y = 0;
		yf = 0;
		sidm_offset = 6;
	}

	int getposf() {
		if (xf != -1) {
			return (xf + lenxf / 2);
		} else {
			return (x + lenxf / 2);
		}
	}

	int getposf2() {
		float s = y / lenx;
		return (x * s + (xf + lenxf / 2) * yf) / (s + yf);
	}

	int getposf3() {
		float s = y / lenx;
		return ((x + lenx) * s + (xf + lenxf / 2) * yf) / (s + yf);
	}

	float sidm_offset;

	float getposf4_b() {
		float s = sigmoid(fabs(yf) - sidm_offset);
		return (x * (1 - s) + getposf() * s);
	}

	float getposf4_e() {
		float s = sigmoid(fabs(yf) - sidm_offset);
		return ((x + lenx) * (1 - s) + getposf() * s);
	}

	void copy(const DPoint &p);
};

struct DPair {
	DPoint left;
	DPoint right;

	float getCenter() {
		return (left.x + left.lenx + right.x) / 2.f;
	}

	float getCenter1() {
		if (left.type != 0 && right.type != 0) {
			return (left.x + left.lenx + right.x) / 2.f;
		} else if (left.type == 0 && right.type != 0) {
			return right.getposf();
		} else if (left.type != 0 && right.type == 0) {
			return left.getposf();
		}
		return 0;
	}

	void copy(const DPair &pair) {
		left.copy(pair.left);
		right.copy(pair.right);
	}

	float getYf() {
		float sum = 0;
		if (left.type != 0) {
			sum -= left.yf;
		}
		if (right.type != 0) {
			sum += right.yf;
		}

		return sum;
	}

	float getCenterRight();

	float getCenterRight2();

	float getCenterLeft();

	float getCenterLeft2();

	float getValidLeft();

	float getValidRight();

	bool isGoodPair() {
		return (left.type != 0 && right.type != 0);
	}
};

#endif
