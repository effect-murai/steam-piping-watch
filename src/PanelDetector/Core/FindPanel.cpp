#include "FindPanel.h"
#include "../Geometry/geometry_util.h"
#include "../Geometry/Vector2f.h"
#include "GlobalVar.h"

using namespace std;
using namespace cv;

int rho = 12.0;
int thresh = 50;
int minLineLength = 10;
int maxLineGap = 10;

struct RectV {
	Point2f a;
	Point2f b;
	Point2f c;
	Point2f d;

	RectV() {
	}

	RectV(float xA, float yA, float xB, float yB, float xC, float yC, float xD,
			float yD) {
		a.x = xA;
		a.y = yA;

		b.x = xB;
		b.y = yB;

		c.x = xC;
		c.y = yC;

		d.x = xD;
		d.y = yD;
	}
};

struct InterPoint2f {
	Point2f pt;
	Vec4f l1;
	Vec4f l2;

	InterPoint2f() {
	}

	InterPoint2f(float x, float y) {
		pt.x = x;
		pt.y = y;
	}

	InterPoint2f(Point2f point, Vec4f line1, Vec4f line2) {
		pt = point;
		l1 = line1;
		l2 = line2;
	}
};

struct TopOf3Rect {
	InterPoint2f sharedTop;
	vector<vector<InterPoint2f> > sharedRectTop;
};

struct PointSort {
	bool operator()(InterPoint2f pt1, InterPoint2f pt2) {
		int x1 = pt1.pt.x;
		int x2 = pt2.pt.x;
		int y1 = pt1.pt.y;
		int y2 = pt2.pt.y;
		if (x1 == x2) {
			return y1 < y2;
		} else {
			return x1 < x2;
		}
	}
};

bool isIntersecting(Vec4f A, Vec4f B, float &xI, float &yI) {
	float x1, y1, x2, y2, x3, y3, x4, y4;
	x1 = A[0];
	y1 = A[1];
	x2 = A[2];
	y2 = A[3];
	x3 = B[0];
	y3 = B[1];
	x4 = B[2];
	y4 = B[3];
	int res;

	res = twoLineIntersection(x1, y1, x2, y2, x3, y3, x4, y4, xI, yI);

	if (res == 3) {
		return true;
	} else {
		return false;
	}
}

cv::Vec2f getLineFunction(cv::Vec4f line) {
	// y = ax + b
	float a = (line[1] - line[3]) / (line[0] - line[2]);
	float b = line[1] - a * line[0];

	return Vec2f(a, b);
}

bool checkTwoNumbers(float d1, float d2, float rate) {
	if (d1 * d2 >= 0) {
		if (d1 < 0) {
			d1 *= -1;
			d2 *= -1;
		}

		if (d1 > d2 / rate && d1 < d2 * rate) {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

bool isOnLine(InterPoint2f pointCheck, InterPoint2f point) {
	Vec4f l1 = point.l1;
	Vec4f l2 = point.l2;
	Vec4f lineCheck = pointCheck.l1;

	if (lineCheck == l1 || lineCheck == l2) {
		return true;
	} else {
		lineCheck = pointCheck.l2;
		if (lineCheck == l1 || lineCheck == l2) {
			return true;
		} else {
			return false;
		}
	}
}

bool isOnLine(InterPoint2f pointCheck, Vec4f line) {
	Vec4f l1 = pointCheck.l1;
	Vec4f l2 = pointCheck.l2;

	if (line == l1 || line == l2) {
		return true;
	} else {
		return false;
	}
}

float getDistancePointsPow(cv::Point2f p1, cv::Point2f p2) {
	return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
}

bool isEdge(Point2f A, Point2f B, float widthPow, float maxRate,
		float minRate) {
	float d = getDistancePointsPow(A, B);
	if (d < widthPow * maxRate && d > widthPow * minRate) {
		return true;
	} else {
		return false;
	}

}

float getAngle(Vec4f &A, Vec4f &B) {

	// Move 1ST Point of vector B --> 1ST point of vector A
	int d0 = B[0] - A[0];
	int d1 = B[1] - A[1];
	int newB2 = B[2] - d0;
	int newB3 = B[3] - d1;

	// Move every point of 2 vector that A[0], A[1] become (0,0)
	int finalX1 = A[2] - A[0];
	int finalY1 = A[3] - A[1];
	int finalX2 = newB2 - A[0];
	int finalY2 = newB3 - A[1];

	// Calculate the angle
	int dot = finalX1 * finalX2 + finalY1 * finalY2; // dot product between[x1, y1] and [x2, y2]
	int det = finalX1 * finalY2 - finalY1 * finalX2;      // determinant
	float angle = atan2(det, dot);  // atan2(y, x) or atan2(sin, cos)
	return angle;
}

void getPositiveDirect(Vec4f lineIn, Vec4f &lineOut, bool isHorizontal) {
	Point2f A, B;

	A.x = lineIn[0];
	A.y = lineIn[1];
	B.x = lineIn[2];
	B.y = lineIn[3];

	Point2f pMax;
	bool haveTochange = false;
	if (isHorizontal) {
		if (A.x > B.x) {
			pMax = A;
			A = B;
			B = pMax;
			haveTochange = true;
		}
	} else {
		if (A.y > B.y) {
			pMax = A;
			A = B;
			B = pMax;
			haveTochange = true;
		}
	}

	if (haveTochange) {
		lineOut[0] = A.x;
		lineOut[1] = A.y;
		lineOut[2] = B.x;
		lineOut[3] = B.y;
	}
}

Vec4f mergeLines(Point2f A1, Point2f B1, Point2f A2, Point2f B2,
		bool isHorizontal) {
	// find the farthest points
	Point2f pMin, pMax;
	if (isHorizontal) {
		if (A1.x < A2.x) {
			pMin = A1;
		} else {
			pMin = A2;
		}
		if (B1.x > B2.x) {
			pMax = B1;
		} else {
			pMax = B2;
		}
	} else {
		if (A1.y < A2.y) {
			pMin = A1;
		} else {
			pMin = A2;
		}
		if (B1.y > B2.y) {
			pMax = B1;
		} else {
			pMax = B2;
		}

	}

	return Vec4f(pMin.x, pMin.y, pMax.x, pMax.y);
}

void getAverageLine(vector<Vec4f> &lines) {
	Vec2f l1, l2;
	for (size_t i = 0; i < lines.size() - 1; i++) {
		Vec4f v1(lines[i]);
		l1 = getLineFunction(v1);

		for (size_t j = i + 1; j < lines.size(); j++) {
			Vec4f v2(lines[j]);

			l2 = getLineFunction(v2);
			if (checkPositiveNumber(l1[1], l2[1], 1.2)) {
				v1 = (v1 + v2) / 2;
				lines[i] = v1;
				lines.erase(lines.begin() + j);
				j--;
			}
		}
	}

}

void arrangeLineEdges(vector<Vec4f> &lines) {
	size_t lineSize = lines.size();

	for (size_t i = 0; i < lineSize; i++) {
		Vec4f l = lines[i];
		int x1 = l[0];
		int y1 = l[1];
		int x2 = l[2];
		int y2 = l[3];
		if ((x2 < x1) || (x2 == x1 && y2 < y1)) { // change position
			l[0] = x2;
			l[1] = y2;
			l[2] = x1;
			l[3] = y1;
			lines[i] = l;
		}
	}
}

bool isStacking(Vec4f l1, Vec4f l2) {
	// 2 edge points of l1: 
	Point2f o1(l1[0], l1[1]);
	Point2f p1(l1[2], l1[3]);
	// 2 edge points of l2: 
	Point2f o2(l2[0], l2[1]);
	Point2f p2(l2[2], l2[3]);

	// find the shorter lines
	if (getDistancePointsPow(o1, p1) > getDistancePointsPow(o2, p2)) {
		Point2f o = o2;
		Point2f p = p2;

		// revert two line
		o2 = o1;
		p2 = p1;
		o1 = o;
		p1 = p;
	}

	// find 2 projection points of 2 edge points l1 on l2
	Vector2F pt(o1.x, o1.y);
	Vector2F linePoint(o2.x, o2.y);
	Vector2F lineDirection(p2.x - o2.x, p2.y - o2.y);

	Vector2F A = projectionPointOnLine2D(pt, linePoint, lineDirection);

	pt = Vector2F(p1.x, p1.y);
	Vector2F B = projectionPointOnLine2D(pt, linePoint, lineDirection);

	Point2f o1P(A.x, A.y);
	Point2f p1P(B.x, B.y);

	// get the distance from projection points to 2 edge points of l2
	float dpow = getDistancePointsPow(o2, p2);
	float d1pow = getDistancePointsPow(o1P, o2);
	float d2pow = getDistancePointsPow(o1P, p2);
	float d3pow = getDistancePointsPow(p1P, o2);
	float d4pow = getDistancePointsPow(p1P, p2);

	if ((dpow >= (d1pow + d2pow)) || (dpow >= (d3pow + d4pow))) {
		return true;
	} else {
		return false;
	}
}

bool isSameDirection(bool isHorizontal, Vec4f line, float rate) {
	Vec4f lineHorizontal(0, 0, 1, 0);
	Vec4f lineVertical(0, 0, 0, 1);

	if (isHorizontal) {
		if (checkPositiveNumber(getDot(line, lineHorizontal), 1, rate)) {
			return true;
		} else {
			return false;
		}
	} else if (checkPositiveNumber(getDot(line, lineVertical), 1, rate)) {
		return true;
	} else {
		return false;
	}
}

void mergeTwoPoints(InterPoint2f &iPt1, InterPoint2f iPt2) {
	iPt1.pt = (iPt1.pt + iPt2.pt) / 2;
	iPt1.l1 = (iPt1.l1 + iPt2.l1) / 2;
	iPt1.l2 = (iPt1.l2 + iPt2.l2) / 2;
}

void getDot(Vec4f l1, Vec4f l2, float &dot) {
	Vector2F v;
	Vector2F v1(l1[2] - l1[0], l1[3] - l1[1]);
	Vector2F v2(l2[2] - l2[0], l2[3] - l2[1]);
	Vector2F v1Nom = v.normalize(v1);
	Vector2F v2Nom = v.normalize(v2);

	dot = v.dot(v1Nom, v2Nom); // = cos(alpha)
}

void getAABB(float x1, float y1, float x2, float y2, float &minX, float &minY,
		float &maxX, float &maxY) {

	if (x1 < x2) {
		minX = x1;
		maxX = x2;
	} else {
		minX = x2;
		maxX = x1;
	}

	if (y1 < y2) {
		minY = y1;
		maxY = y2;
	} else {
		minY = y2;
		maxY = y1;
	}
}

bool isOverlapAABB(Vec4f l1, Vec4f l2, float plusFactor) {
	float minX1, minY1, maxX1, maxY1, minX2, minY2, maxX2, maxY2;
	getAABB(l1[0], l1[1], l1[2], l1[3], minX1, minY1, maxX1, maxY1);
	getAABB(l2[0], l2[1], l2[2], l2[3], minX2, minY2, maxX2, maxY2);

	if (maxX1 + plusFactor < minX2) {
		return false; // a is left of b
	} else if (minX1 - plusFactor > maxX2) {
		return false; // a is right of b
	} else if (maxY1 + plusFactor < minY2) {
		return false; // a is above b
	} else if (minY1 - plusFactor > maxY2) {
		return false; // a is below b
	} else {
		return true; // boxes overlap
	}
}

bool isPerpendicular(Vec4f l1, Vec4f l2, float rate) {
	float dot;
	getDot(l1, l2, dot);
	if (checkPositiveNumber(dot, 0, rate)) {
		return true;
	} else {
		return false;
	}
}

bool isParaell(Vec4f l1, Vec4f l2, float rate) {
	float dot;
	getDot(l1, l2, dot);
	if (checkPositiveNumber(dot, 1, rate)) {
		return true;
	} else {
		return false;
	}
}

void myGetNearestPoint(vector<Point2f> ptsFind, vector<Point2f> ptsMearsure,
		Vector2F &nearestPoint, bool isHorizontal) {
	// find the point in ptsFind that have the nearest distance to ptsMearsure
	size_t ptsFindSize = ptsFind.size();
	size_t ptsMearsureSize = ptsMearsure.size();
	Point2f ptFind, ptMearsure;
	if (isHorizontal) {
		float dx;
		float dxMin = FLT_MAX;

		for (size_t i = 0; i < ptsFindSize; i++) {
			ptFind = ptsFind[i];
			for (size_t j = 0; j < ptsMearsureSize; j++) {
				ptMearsure = ptsMearsure[j];
				dx = ptMearsure.x - ptFind.x;
				if (dx < 0) {
					dx *= -1;
				}
				if (dxMin > dx) {
					dxMin = dx;
					nearestPoint.x = ptFind.x;
					nearestPoint.y = ptFind.y;
				}
			}
		}
	} else {
		float dy;
		float dyMin = FLT_MAX;

		for (size_t i = 0; i < ptsFindSize; i++) {
			ptFind = ptsFind[i];
			for (size_t j = 0; j < ptsMearsureSize; j++) {
				ptMearsure = ptsMearsure[j];
				dy = ptMearsure.y - ptFind.y;
				if (dy < 0) {
					dy *= -1;
				}
				if (dyMin > dy) {
					dyMin = dy;
					nearestPoint.x = ptFind.x;
					nearestPoint.y = ptFind.y;
				}
			}
		}
	}
}

float distanceLine2Line(Point2f l1A, Point2f l1B, Point2f l2A, Point2f l2B,
		bool isHorizontal) {

	vector<Point2f> l1Edges;
	l1Edges.push_back(l1A);
	l1Edges.push_back(l1B);

	vector<Point2f> l2Edges;
	l2Edges.push_back(l2A);
	l2Edges.push_back(l2B);

	Vector2F nearestPoint;

	// find the nearest edges of 2 line

	myGetNearestPoint(l1Edges, l2Edges, nearestPoint, isHorizontal);

	Vector2F linePoint(l2A.x, l2A.y);
	Vector2F v2(l2B.x - l2A.x, l2B.y - l2A.y);

	float dpl = pistancePoint2Line2D(nearestPoint, linePoint, v2);

	return dpl;

}

float distance2ParaellLine(Vec4f l1, Vec4f l2) {

	Vector2F v2(l2[2] - l2[0], l2[3] - l2[1]);

	float dpl = pistancePoint2Line2D(Vector2F(l1[0], l1[1]),
			Vector2F(l2[0], l2[1]), v2);

	return dpl;

}

bool isIncreaseLine(Vec4f line, bool isHorizontal) {
	if (isHorizontal) {
		float dy = line[3] - line[1];

		if (dy > -1) {
			return true;
		} else if (dy < 1) {
			return false;
		}
	} else {
		float dx = line[2] - line[0];

		if (dx > -1) {
			return true;
		} else if (dx < 1) {
			return false;
		}
	}
	return false;
}

void detectLines(Mat img, vector<Vec4f> &linesP) {
	vector<Vec4f> horizontalLinesP;
	vector<Vec4f> verticalLinesP;

	HoughLinesP(img, horizontalLinesP, (float) rho / 10, CV_PI / 180, thresh,
			minLineLength, maxLineGap);

	linesP = horizontalLinesP;
	linesP.insert(linesP.end(), verticalLinesP.begin(), verticalLinesP.end());
}

void removedNoiseLines(vector<Vec4f> &lineGroup, vector<DLine> d_LineGroup,
		float paraellRate) {

	vector<Vec4f> lineGroupCheck = lineGroup;
	lineGroup.clear();
	// sort line
	sort(d_LineGroup.begin(), d_LineGroup.end(), DLineSort());

	// get the 10% longest line
	size_t longestLinesAmount = d_LineGroup.size() / 5;

	for (size_t i = 0; i < longestLinesAmount; i++) {
		lineGroup.push_back(d_LineGroup[i].line);
	}

	d_LineGroup.erase(d_LineGroup.begin(),
			d_LineGroup.begin() + longestLinesAmount - 1);

	// remove the long line that's not paraell and near with others

	// find the line that paraell with that long line
	size_t lineGroupCheckSize = d_LineGroup.size();

	for (size_t i = 0; i < lineGroupCheckSize; i++) {
		Vec4f lineCheck = d_LineGroup[i].line;

		for (size_t j = 0; j < longestLinesAmount; j++) {
			Vec4f line = lineGroup[j];

			float d = distance2ParaellLine(lineCheck, line);
			if (isParaell(lineCheck, line, paraellRate) && d < 100) {

				lineGroup.push_back(d_LineGroup[i].line);
				break;
			}

		}
	}
}

void mergeDuplicateLines(vector<Vec4f> &lines, bool isHorizontal, float rate) {
	Vec4f l1, l2;
	Point2f l1A, l1B, l2A, l2B;

	for (size_t i = 0; i < lines.size() - 1; i++) {
		l1 = lines[i];
		l1A.x = l1[0];
		l1A.y = l1[1];
		l1B.x = l1[2];
		l1B.y = l1[3];

		for (size_t j = 0; j < lines.size(); j++) {

			if (i != j) {
				l2 = lines[j];
				l2A.x = l2[0];
				l2A.y = l2[1];
				l2B.x = l2[2];
				l2B.y = l2[3];

				float dot;

				// Check to find duplicate lines has intersection
				if (isOverlapAABB(l1, l2, 5)) {
					getDot(l1, l2, dot);

					if (isIntersecting(l1, l2)) {

						if (checkPositiveNumber(dot, 1, rate)) {
							// get the new line
							lines[i] = mergeLines(l1A, l1B, l2A, l2B,
									isHorizontal);

							// remove duplicate line
							lines.erase(lines.begin() + j);
							i--;
							break;
						}
					} else {
						// Check to find duplicate lines has no intersection
						// if |dot| ~ 1 --> duplicate OR paraell
						if (checkPositiveNumber(dot, 1,
								rate) /*~ alpha = 0 / 180*/) {
							// find distance from a point on line a --> line b

							float dpl = distanceLine2Line(l1A, l1B, l2A, l2B,
									isHorizontal);

							// detect the duplicate lines
							if (dpl < 2) {
								lines[i] = mergeLines(l1A, l1B, l2A, l2B,
										isHorizontal);

								// remove duplicate line
								lines.erase(lines.begin() + j);
								i--;
								break;
							}
						}
					}
				}
			}
		}
	}
}

void extendLines(vector<Vec4f> &lines, int factor, bool isHorizontal) {
	size_t vecSize = lines.size();
	for (size_t i = 0; i < vecSize; i++) {
		Vec4f line = lines[i];
		float x1 = line[0];
		float y1 = line[1];
		float x2 = line[2];
		float y2 = line[3];

		float dx = x2 - x1;
		float dy = y2 - y1;

		if (isHorizontal) { // extend X
			if (dx > 0) {
				x1 -= factor;
				x2 += factor;
			} else {
				x1 += factor;
				x2 -= factor;
			}
			Vec2f lineFunct = getLineFunction(line);
			float a = lineFunct[0];
			float b = lineFunct[1];

			y1 = a * x1 + b;
			y2 = a * x2 + b;

		} else { // extend y

			if (dy > 0) {
				y1 -= factor;
				y2 += factor;
			} else {
				y1 += factor;
				y2 -= factor;
			}
			if (dx != 0) {
				Vec2f lineFunct = getLineFunction(line);
				float a = lineFunct[0];
				float b = lineFunct[1];

				x1 = (y1 - b) / a;
				x2 = (y2 - b) / a;
			}
		}
		lines[i] = Vec4f(x1, y1, x2, y2);
	}
}

void getWidthLengthPow(vector<Point2f> rectEdges, float &widthPow,
		float &lengthPow) {
	float ABpow = getDistancePointsPow(rectEdges[0], rectEdges[1]);
	float ADpow = getDistancePointsPow(rectEdges[0], rectEdges[3]);

	if (ABpow > ADpow) {
		widthPow = ADpow;
		lengthPow = ABpow;
	} else {
		widthPow = ABpow;
		lengthPow = ADpow;
	}
}

void getLinesIntersectionPoints(vector<Vec4f> linesHor, vector<Vec4f> linesVer,
		vector<InterPoint2f> &iPts, float perpendRate, float overlapLength,
		bool isIncludeOnLine) {
	size_t linesHorSize = linesHor.size();
	size_t linesVerSize = linesVer.size();
	for (size_t i = 0; i < linesHorSize; i++) {
		Vec4f l1 = linesHor[i];
		int res;
		float x1, y1, x2, y2, x3, y3, x4, y4, xI, yI;
		x1 = l1[0];
		y1 = l1[1];
		x2 = l1[2];
		y2 = l1[3];
		for (size_t j = 0; j < linesVerSize; j++) {
			Vec4f l2 = linesVer[j];
			x3 = l2[0];
			y3 = l2[1];
			x4 = l2[2];
			y4 = l2[3];
			if (isOverlapAABB(l1, l2, overlapLength)) {

				// find perpendicular line
				if (isPerpendicular(l1, l2, perpendRate)) {
					res = twoLineIntersection(x1, y1, x2, y2, x3, y3, x4, y4,
							xI, yI);

					if (isIncludeOnLine) {
						if (res == 3) {
							// 3 : ok, intersection is on both segment
							InterPoint2f iPt(Point2f(xI, yI), linesHor[i],
									linesVer[j]);
							iPts.push_back(iPt);
						}
					}

					if (res == 2 || res == 1 || res == 0) {
						// 2 : intersection is on second segment only
						// 1 : intersection is on first segment only
						InterPoint2f iPt(Point2f(xI, yI), linesHor[i],
								linesVer[j]);
						iPts.push_back(iPt);

					}
				}
			}
		}
	}

	// sort points
	sort(iPts.begin(), iPts.end(), PointSort());
}

void getLinesIntersectionPoints(vector<Vec4f> linesHor, vector<Vec4f> linesVer,
		vector<Point2f> &ptGroup, float perpendRate, float overlapLength,
		bool isIncludeOnLine) {
	size_t linesHorSize = linesHor.size();
	size_t linesVerSize = linesVer.size();
	for (size_t i = 0; i < linesHorSize; i++) {
		Vec4f l1 = linesHor[i];
		int res;
		float x1, y1, x2, y2, x3, y3, x4, y4, xI, yI;
		x1 = l1[0];
		y1 = l1[1];
		x2 = l1[2];
		y2 = l1[3];
		for (size_t j = 0; j < linesVerSize; j++) {
			Vec4f l2 = linesVer[j];
			x3 = l2[0];
			y3 = l2[1];
			x4 = l2[2];
			y4 = l2[3];
			if (isOverlapAABB(l1, l2, overlapLength)) {

				// find perpendicular line
				if (isPerpendicular(l1, l2, perpendRate)) {
					res = twoLineIntersection(x1, y1, x2, y2, x3, y3, x4, y4,
							xI, yI);

					if (isIncludeOnLine) {
						if (res == 3) {
							// 3 : ok, intersection is on both segment
							ptGroup.push_back(Point2f(xI, yI));
						}
					}

					if (res == 2 || res == 1 || res == 0) {
						// 2 : intersection is on second segment only
						// 1 : intersection is on first segment only
						ptGroup.push_back(Point2f(xI, yI));
					}
				}
			}
		}
	}
}

vector<InterPoint2f> removeNoisePoints(vector<InterPoint2f> iPtsIn,
		float focusLength, int minRectWidthPow) {
	vector<InterPoint2f> iPtsRemoved;
	// add focus point
	size_t pointsSize = iPtsIn.size();
	for (size_t i = 0; i < pointsSize; i++) {
		bool isFocus = true;
		InterPoint2f ip1 = iPtsIn[i];
		for (size_t j = i + 1; j < pointsSize; j++) {
			InterPoint2f ip2 = iPtsIn[j];
			float dPow = getDistancePointsPow(ip1.pt, ip2.pt);
			if (dPow > 1 && dPow < minRectWidthPow) {
				isFocus = false;
				break;
			}
		}
		if (isFocus) {
			iPtsRemoved.push_back(ip1);
		}
	}

	return iPtsRemoved;
}

void findWidthOfRects(vector<InterPoint2f> iPtsFilted, float &widthPow,
		int minRectWidtPow) {

	// get the nearest distance
	size_t pointsSize = iPtsFilted.size();
	vector<float> nearestDistPowVec;
	vector<int> nearestDistPowNumb;
	for (size_t i = 0; i < pointsSize; i++) {
		InterPoint2f ip1 = iPtsFilted[i];
		float nearestDistPow = FLT_MAX;

		for (size_t j = 0; j < pointsSize; j++) {
			if (i != j) {
				InterPoint2f ip2 = iPtsFilted[j];
				float dPow = getDistancePointsPow(ip1.pt, ip2.pt);
				if (dPow < nearestDistPow && dPow > minRectWidtPow) {
					nearestDistPow = dPow;
				}
			}
		}

		// add and count
		bool isIncluded = false;
		size_t dSize = nearestDistPowVec.size();
		int position = -1;
		for (size_t k = 0; k < dSize; k++) {
			if (checkPositiveNumber(nearestDistPow, nearestDistPowVec[k],
					1.01)) {
				isIncluded = true;
				position = k;
				break;
			}
		}
		if (!isIncluded) {
			nearestDistPowVec.push_back(nearestDistPow);
			nearestDistPowNumb.push_back(1);
		} else {
			nearestDistPowVec[position] = (nearestDistPowVec[position]
					+ nearestDistPow) / 2;

			nearestDistPowNumb[position]++;

		}
	}

	// get the most common nearest distance
	size_t dSize = nearestDistPowNumb.size();

	float maxNumb = 0;
	for (size_t i = 0; i < dSize; i++) {
		float numbTest = nearestDistPowNumb[i];
		if (numbTest > maxNumb) {
			maxNumb = numbTest;
			widthPow = nearestDistPowVec[i];
		}
	}
}

void rotateRectModelTops(vector<InterPoint2f> rectModelTops, float cos,
		float sin, vector<InterPoint2f> &rectModelTopsRotated) {

	InterPoint2f o = rectModelTops[0];

	for (int i = 1; i < 4; i++) {
		InterPoint2f p = rectModelTops[i];
		InterPoint2f protated;

		// x′ = xcosθ − ysinθ
		// y′ = ycosθ + xsinθ
		protated.pt.x = (p.pt.x - o.pt.x) * cos - (p.pt.y - o.pt.y) * sin;
		protated.pt.y = (p.pt.x - o.pt.x) * sin + (p.pt.y - o.pt.y) * cos;
		protated.pt.x += o.pt.x;
		protated.pt.y += o.pt.y;
		rectModelTopsRotated[i] = protated;
	}
}

void getRects(float rectWidthPow, float rectLengthPow,
		vector<InterPoint2f> iPts, size_t iPtsSize,
		vector<vector<InterPoint2f> > &rects, float lengthPowRatio,
		float diffPerpendRat) {
	for (size_t i = 0; i < iPtsSize; i++) {
		InterPoint2f a;
		vector<InterPoint2f> rectPeaks;
		a = iPts[i];

		rectPeaks.push_back(a);

		// find 2 next-to point of rectangle
		for (size_t j = 0; j < iPtsSize; j++) {

			if (j == i) {
				continue;
			}

			InterPoint2f b;
			b = iPts[j]; // [451, 266], 
						 // find the width
			float lengthPowAB = getDistancePointsPow(a.pt, b.pt);

			if ((b.pt.x - 5 > a.pt.x)
					&& checkPositiveNumber(lengthPowAB, rectLengthPow,
							lengthPowRatio) && isOnLine(b, a)
					&& rectPeaks.size() == 1) {
				bool isRect = false;
				rectPeaks.push_back(b);

				// find the height
				for (size_t l = 0; l < iPtsSize; l++) {
					if (l == i || l == j) {
						continue;
					}

					InterPoint2f d;
					d = iPts[l]; // [189, 366], [191, 376],

					Vec4f lAD(d.pt.x, d.pt.y, a.pt.x, a.pt.y);
					Vec4f lAB(b.pt.x, b.pt.y, a.pt.x, a.pt.y);

					float lengthPowAD = getDistancePointsPow(a.pt, d.pt);

					if ((d.pt.y - 5 > a.pt.y)
							&& checkPositiveNumber(lengthPowAD, rectWidthPow,
									lengthPowRatio) && !isOnLine(d, b)
							&& rectPeaks.size() == 2
							&& checkPositiveNumber(getDot(lAD, lAB), 0,
									diffPerpendRat)) {
						rectPeaks.push_back(d);

						// find the 4th point of rectangle
						for (size_t k = 0; k < iPtsSize; k++) {
							if (k == l || k == i || k == j) {
								continue;
							}

							InterPoint2f c;
							c = iPts[k];

							if ((k != j) && (c.pt != d.pt)) {
								Vec4f lCD(c.pt.x, c.pt.y, d.pt.x, d.pt.y);
								Vec4f lCB(c.pt.x, c.pt.y, b.pt.x, b.pt.y);

								if (rectPeaks.size() == 3 && isOnLine(c, d)
										&& isOnLine(c, b)
										&& checkPositiveNumber(getDot(lCD, lCB),
												0, diffPerpendRat)
										&& checkPositiveNumber(getDot(lAD, lCD),
												0, diffPerpendRat)) {
									rectPeaks.push_back(c);

									rects.push_back(rectPeaks);
									isRect = true;
									break;
								}
							}
						}

						if (isRect) {
							break;
						} else {				// found A, B, D but not found C
							rectPeaks.pop_back();		// remove D, refind D
						}
					}
				}

				if (isRect) {
					break;
				} else { // found A, B but not found D, C
					rectPeaks.pop_back(); // remove B, refind B
				}
			}
		}
	}
}

void findRects(vector<InterPoint2f> iPts,
		vector<vector<InterPoint2f> > &rectsIP, float widthPow, float lengthPow,
		float lengthPowRatio, float diffPerpendRate) {

	// Find the width length of rect
	size_t iPtsSize = iPts.size();

	if (iPtsSize == 0) {
		return;
	}

	cout << "Width: " << sqrt(widthPow) << endl;

	getRects(widthPow, lengthPow, iPts, iPtsSize, rectsIP, lengthPowRatio,
			diffPerpendRate);
}

void moveRectModelTops(vector<Point2f> rectModelTops, Point2f P1stMoved,
		vector<InterPoint2f> &rectModelTopsMoved) {

	Point2f P1st = rectModelTops[0];

	float dx = P1st.x - P1stMoved.x;
	float dy = P1st.y - P1stMoved.y;

	for (int i = 0; i < 4; i++) {
		Point2f O = rectModelTops[i];
		InterPoint2f Omoved(O.x - dx, O.y - dy);
		rectModelTopsMoved.push_back(Omoved);
	}
}

void findRectangleModel(vector<Vec4f> lineHor, vector<Vec4f> lineVer,
		vector<Point2f> &rectModelTops, float minWidthPow, float &widthPow,
		float &lengthPow, float equalRatio, float perpendDiff) {

	float widthPowCheck, lengthPowCheck;
	size_t lineHorSize = lineHor.size();
	size_t lineVerSize = lineVer.size();

	do {
		for (size_t i = 0; i < lineHorSize; i++) {
			Vec4f l1 = lineHor[i];

			for (size_t j = 0; j < lineVerSize; j++) {

				Vec4f l2 = lineVer[j];
				Point2f a;

				if (isIntersecting(l1, l2, a.x, a.y)
						&& checkPositiveNumber(getDot(l1, l2), 0,
								perpendDiff)) {

					for (size_t k = 0; k < lineVerSize; k++) {
						if (k == j) {
							continue;
						}

						Vec4f l3 = lineVer[k];
						Point2f b;

						if (isIntersecting(l1, l3, b.x, b.y)
								&& checkPositiveNumber(getDot(l1, l3), 0,
										perpendDiff)) {
							if (b.x - 5 <= a.x) {
								continue;
							}

							float lengthPowAB = getDistancePointsPow(a, b);

							for (size_t m = 0; m < lineHorSize; m++) {
								if (m == i) {
									continue;
								}

								Vec4f l4 = lineHor[m];
								Point2f c, d;

								if (isIntersecting(l3, l4, c.x, c.y)
										&& isIntersecting(l4, l2, d.x, d.y)
										&& checkPositiveNumber(getDot(l3, l4),
												0, perpendDiff)
										&& checkPositiveNumber(getDot(l4, l2),
												0, perpendDiff)

												) {
									float lengthPowAD = getDistancePointsPow(a,
											d);
									float lengthPowBC = getDistancePointsPow(b,
											c);
									float lengthPowCD = getDistancePointsPow(c,
											d);

									if (d.y - 5 <= a.y) {
										continue;
									}

									if (checkPositiveNumber(lengthPowAB,
											lengthPowCD, equalRatio)
											&& checkPositiveNumber(lengthPowAD,
													lengthPowBC, equalRatio)) {

										vector<Point2f> rectModelTopsCheck;
										rectModelTopsCheck.push_back(a);
										rectModelTopsCheck.push_back(b);
										rectModelTopsCheck.push_back(c);
										rectModelTopsCheck.push_back(d);
										getWidthLengthPow(rectModelTopsCheck,
												widthPowCheck, lengthPowCheck);

										if (widthPow > widthPowCheck * 1.01
												&& widthPowCheck > minWidthPow
												&& lengthPow
														> lengthPowCheck * 1.01
												&& checkTwoNumbers(
														lengthPowCheck
																/ widthPowCheck,
														rectLengthRatio, 1.2)) {
											rectModelTops = rectModelTopsCheck;
											widthPow = widthPowCheck;
											lengthPow = lengthPowCheck;
											cout << "widthPow" << sqrt(widthPow)
													<< endl;
											cout << "lengthPow"
													<< sqrt(lengthPow) << endl;
										}
									}
								}
							}
						}
					}
				}
			}
		}

		perpendDiff += 0.01;

	} while (rectModelTops.size() == 0 && perpendDiff < 0.05);
}

void findRectsFromModel(vector<Point2f> rectModelTops,
		vector<InterPoint2f> ptGroup,
		vector<vector<InterPoint2f> > &rectTopsGroup, float distPowDiff,
		float lengthPowRatio) {
	if (rectModelTops.size() == 0) {
		return;
	}

	size_t ptGroupSize = ptGroup.size();
	float dABModelPow = getDistancePointsPow(rectModelTops[0],
			rectModelTops[1]);

	for (size_t i = 0; i < ptGroupSize; i++) {
		bool found = false;
		InterPoint2f topA = ptGroup[i];

		// Move the rectModelTops to 1st point (based on the 1st rectModelTop)
		vector<InterPoint2f> rectModelTopsMatched;
		moveRectModelTops(rectModelTops, topA.pt, rectModelTopsMatched);
		InterPoint2f modelTopB = rectModelTopsMatched[1];

		for (size_t j = 0; j < ptGroupSize; j++) {

			InterPoint2f topB = ptGroup[j];

			if (topB.pt.x - 5 <= topA.pt.x) {
				continue;
			}

			float dABpow = getDistancePointsPow(topA.pt, topB.pt);

			if (isOnLine(topA, topB)
					&& checkPositiveNumber(dABpow, dABModelPow,
							lengthPowRatio)) {
				Vec4f a(topA.pt.x, topA.pt.y, modelTopB.pt.x, modelTopB.pt.y);
				Vec4f b(topA.pt.x, topA.pt.y, topB.pt.x, topB.pt.y);
				float angle = getAngle(a, b);
				float cosAlpha = cos(angle);
				float sinAlpha = sin(angle);

				// match the 2nd rectModelTop to mach 2nd point and rotate 3rd and 4th rectModelTop belong to there 2nd
				rotateRectModelTops(rectModelTopsMatched, cosAlpha, sinAlpha,
						rectModelTopsMatched);
				modelTopB = rectModelTopsMatched[1];

				InterPoint2f modelTopC = rectModelTopsMatched[2];
				InterPoint2f modelTopD = rectModelTopsMatched[3];

				// find and check the 3rd and 4th point
				for (size_t k = 0; k < ptGroupSize; k++) {
					InterPoint2f topC = ptGroup[k];

					if (isOnLine(topB, topC)
							&& getDistancePointsPow(topC.pt, modelTopC.pt)
									< distPowDiff) {

						for (size_t m = 0; m < ptGroupSize; m++) {
							InterPoint2f topD = ptGroup[m];
							if (topD.pt.y - 5 <= topA.pt.y) {
								continue;
							}

							if (isOnLine(topC, topD) && isOnLine(topA, topD)
									&& getDistancePointsPow(topD.pt,
											modelTopD.pt) < distPowDiff) {
								vector<InterPoint2f> rectTops;
								rectTops.push_back(topA);
								rectTops.push_back(topB);
								rectTops.push_back(topC);
								rectTops.push_back(topD);
								rectTopsGroup.push_back(rectTops);
								found = true;
								break;
							}
						}

						if (found) {
							break;
						}

					}
				}

				if (found) {
					break;
				}
			}
		}
	}
}

void findRects3(vector<Vec4f> lineHor, vector<Vec4f> lineVer,
		vector<RectV> &rects, float equalRatio) {

	size_t lineHorSize = lineHor.size();
	size_t lineVerSize = lineVer.size();

	for (size_t i = 0; i < lineHorSize; i++) {
		Vec4f l1 = lineHor[i];

		for (size_t j = 0; j < lineVerSize; j++) {

			Vec4f l2 = lineVer[j];
			Point2f a;

			if (isIntersecting(l1, l2, a.x, a.y)) {

				for (size_t k = 0; k < lineVerSize; k++) {
					if (k == j) {
						continue;
					}

					Vec4f l3 = lineVer[k];
					Point2f b;

					if (isIntersecting(l1, l3, b.x, b.y)) {
						if (b.x - 5 <= a.x) {
							continue;
						}

						float lengthPowAB = getDistancePointsPow(a, b);

						for (size_t m = 0; m < lineHorSize; m++) {
							if (m == i) {
								continue;
							}

							Vec4f l4 = lineHor[m];
							Point2f c, d;

							if (isIntersecting(l3, l4, c.x, c.y)
									&& isIntersecting(l4, l2, d.x, d.y)) {
								float lengthPowAD = getDistancePointsPow(a, d);
								float lengthPowBC = getDistancePointsPow(b, c);
								float lengthPowCD = getDistancePointsPow(c, d);

								if (d.y - 5 <= a.y) {
									continue;
								}

								if (checkPositiveNumber(lengthPowAB,
										lengthPowCD, equalRatio)
										&& checkPositiveNumber(lengthPowAD,
												lengthPowBC, equalRatio)) {

									RectV newRect(a.x, a.y, b.x, b.y, c.x, c.y,
											d.x, d.y);

									rects.push_back(newRect);

								}
							}
						}
					}
				}
			}
		}
	}
}

void mergeRects(vector<vector<InterPoint2f> > &rectsGroup, float distPowDiff) {

	for (size_t i = 0; i < rectsGroup.size() - 1; i++) {
		vector<InterPoint2f> rect1 = rectsGroup[i];
		size_t rectsGroupSize = rectsGroup.size();

		for (size_t j = i + 1; j < rectsGroupSize; j++) {

			vector<InterPoint2f> rect2 = rectsGroup[j];

			if (i == j) {
				continue;
			}
			float dAPow = getDistancePointsPow(rect1[0].pt, rect2[0].pt);
			float dBPow = getDistancePointsPow(rect1[1].pt, rect2[1].pt);
			float dCPow = getDistancePointsPow(rect1[2].pt, rect2[2].pt);
			float dDPow = getDistancePointsPow(rect1[3].pt, rect2[3].pt);

			if (checkPositiveNumber(dAPow, 0, distPowDiff)
					&& checkPositiveNumber(dBPow, 0, distPowDiff)
					&& checkPositiveNumber(dCPow, 0, distPowDiff)
					&& checkPositiveNumber(dDPow, 0, distPowDiff)) {

				mergeTwoPoints(rect1[0], rect2[0]);
				mergeTwoPoints(rect1[1], rect2[1]);
				mergeTwoPoints(rect1[2], rect2[2]);
				mergeTwoPoints(rect1[3], rect2[3]);
				rectsGroup[i] = rect1;

				rectsGroup.erase(rectsGroup.begin() + j);
				i--;
				break;
			}
		}
	}
}

void convertRects(vector<vector<InterPoint2f> > rectsIP, vector<RectF> &rects) {
	// Convert rectEdges
	size_t rectsSize = rectsIP.size();
	for (size_t i = 0; i < rectsSize; i++) {
		vector<InterPoint2f> rectIn = rectsIP[i];
		RectF rect;
		rect.x1 = rectIn[0].pt.x;
		rect.y1 = rectIn[0].pt.y;

		rect.x2 = rectIn[1].pt.x;
		rect.y2 = rectIn[1].pt.y;

		rect.x3 = rectIn[2].pt.x;
		rect.y3 = rectIn[2].pt.y;

		rect.x4 = rectIn[3].pt.x;
		rect.y4 = rectIn[3].pt.y;

		rects.push_back(rect);
	}
}

void convertRects(vector<vector<Point2f> > rectGroup, vector<RectF> &rects) {
	// Convert rectEdges
	size_t rectsSize = rectGroup.size();
	for (size_t i = 0; i < rectsSize; i++) {
		vector<Point2f> rectIn = rectGroup[i];
		RectF rect;
		rect.x1 = rectIn[0].x;
		rect.y1 = rectIn[0].y;

		rect.x2 = rectIn[1].x;
		rect.y2 = rectIn[1].y;

		rect.x3 = rectIn[2].x;
		rect.y3 = rectIn[2].y;

		rect.x4 = rectIn[3].x;
		rect.y4 = rectIn[3].y;

		rects.push_back(rect);
	}
}

void removeNoiseLine(vector<Vec4f> &lineGroup,
		vector<vector<InterPoint2f> > rectGroup) {
	size_t rectGroupSize = rectGroup.size();
	for (size_t i = 0; i < lineGroup.size(); i++) {
		bool found = false;
		Vec4f lineCheck = lineGroup[i];

		for (size_t j = 0; j < rectGroupSize; j++) // Check that line with line in rect Group
				{
			vector<InterPoint2f> rect = rectGroup[j];

			for (size_t k = 0; k < 4; k++) {
				Vec4f line1 = rect[k].l1;
				Vec4f line2 = rect[k].l2;

				if (lineCheck == line1 || lineCheck == line2) {
					found = true;
					break;
				}
			}

			if (found) {
				break;
			}
		}

		if (!found) {
			lineGroup.erase(lineGroup.begin() + i);
			i--;
		}
	}
}

void findTopOf3Rects(vector<InterPoint2f> topGroup,
		vector<vector<InterPoint2f> > rectGroup, size_t rectGroupSize,
		vector<TopOf3Rect> &topOf3Rects) {
	size_t topGroupSize = topGroup.size();
	for (size_t i = 0; i < topGroupSize; i++) {
		InterPoint2f topCheck = topGroup[i];
		TopOf3Rect topOf3RectsCheck;
		topOf3RectsCheck.sharedTop = topCheck;
		bool found = false;

		for (size_t j = 0; j < rectGroupSize; j++) {
			vector<InterPoint2f> rect = rectGroup[j];

			for (size_t k = 0; k < 4; k++) {
				InterPoint2f top = rect[k];

				if (topCheck.pt == top.pt) {
					topOf3RectsCheck.sharedRectTop.push_back(rect);
					break;
				}
			}

			if (topOf3RectsCheck.sharedRectTop.size() == 3) {
				found = true;
				break;
			}
		}

		if (found) {
			topOf3Rects.push_back(topOf3RectsCheck);
		}
	}
}

void findTwoNearbyTop(vector<TopOf3Rect> topOf3Rects,
		vector<vector<TopOf3Rect> > &twoNearbyTopGroup, float widthPow,
		float lengthPow) {
	size_t topOf3RectsSize = topOf3Rects.size();
	if (topOf3RectsSize == 0) {
		return;
	}
}

void findTwoTopMissed(vector<vector<TopOf3Rect> > twoNearbyTopGroup,
		vector<vector<InterPoint2f> > &rectGroup) {
	size_t twoNearbyTopGroupSize = twoNearbyTopGroup.size();
	if (twoNearbyTopGroupSize == 0) {
		return;
	}

}

void findMissedRectGroup(float widthPow, float lengthPow,
		vector<InterPoint2f> topGroup,
		vector<vector<InterPoint2f> > &rectGroup) {
	size_t rectGroupSize = rectGroup.size();
	if (rectGroupSize == 0) {
		return;
	}

	// get top of EXACTLY  3 rects --> that top is on the missing rects
	vector<TopOf3Rect> topOf3Rects;
	findTopOf3Rects(topGroup, rectGroup, rectGroupSize, topOf3Rects);

	// find  2 nearby topof3rects (== width or length)
	vector<vector<TopOf3Rect> > twoNearbyTopGroup;
	findTwoNearbyTop(topOf3Rects, twoNearbyTopGroup, widthPow, lengthPow);

	// find Two Remain Top of rect Missed
	findTwoTopMissed(twoNearbyTopGroup, rectGroup);
}

// **************************************************************************************

void convertTwoNumber(float &a, float &b) {
	float x = a;
	a = b;
	b = x;
}

void rotateLines(vector<Vec4f> linesIn, vector<Vec4f> &linesOut, float angle) {
	linesOut.erase(linesOut.begin(), linesOut.end());

	size_t linesSize = linesIn.size();

	for (size_t i = 0; i < linesSize; i++) {
		Vec4f line = linesIn[i];

		convertTwoNumber(line[0], line[1]);
		convertTwoNumber(line[2], line[3]);

		linesOut.push_back(line);
	}
}

void drawPoints(vector<InterPoint2f> pointGroup, Mat src_ir, String imageName) {
	Mat imgPoints = src_ir;
	cvtColor(imgPoints, imgPoints, COLOR_GRAY2BGR);

	size_t iPtsSize = pointGroup.size();
	for (size_t i = 0; i < iPtsSize; i++) {
		circle(imgPoints, pointGroup[i].pt, 1, Scalar(0, 0, 255), 1, 8);
	}

	imwrite(imageName, imgPoints);

}

void drawPoints(vector<Point2f> pointGroup, Mat src_ir, String imageName) {
	Mat imgPoints = src_ir;
	cvtColor(imgPoints, imgPoints, COLOR_GRAY2BGR);

	size_t iPtsSize = pointGroup.size();
	for (size_t i = 0; i < iPtsSize; i++) {
		circle(imgPoints, pointGroup[i], 1, Scalar(0, 0, 255), 1, 8);
	}

	imwrite(imageName, imgPoints);

}

void drawModelRect(vector<Point2f> rect, Mat src_ir, String imageName) {
	if (rect.size() == 0) {
		return;
	}
	// draw the amount and position of solar panel

	Mat imgRects = src_ir;
	cvtColor(imgRects, imgRects, COLOR_GRAY2BGR);

	Point2f a = rect[0];
	Point2f b = rect[1];
	Point2f c = rect[2];
	Point2f d = rect[3];
	circle(imgRects, a, 3, Scalar(0, 0, 255), 1, 8);
	circle(imgRects, b, 3, Scalar(255, 0, 255), 1, 8);
	line(imgRects, a, b, Scalar(0, 0, 255), 1, LINE_AA);

	circle(imgRects, d, 3, Scalar(0, 255, 255), 1, 8);
	line(imgRects, a, d, Scalar(0, 0, 255), 1, LINE_AA);

	circle(imgRects, c, 3, Scalar(255, 0, 0), 1, 8);
	line(imgRects, d, c, Scalar(0, 0, 255), 1, LINE_AA);
	line(imgRects, b, c, Scalar(0, 0, 255), 1, LINE_AA);

	imwrite(imageName, imgRects);

}

void drawRects(vector<vector<InterPoint2f> > rectsIP, Mat src_ir,
		String imageName) {
	// draw the amount and position of solar panel
	size_t rectsSize = rectsIP.size();

	Mat imgRects = src_ir;
	cvtColor(imgRects, imgRects, COLOR_GRAY2BGR);

	for (size_t i = 0; i < rectsSize; i++) {
		vector<InterPoint2f> rectTops = rectsIP[i];
		putText(imgRects, (cv::String() + (i + 1)),
				Point(rectTops[0].pt.x,
						(rectTops[0].pt.y + rectTops[3].pt.y) / 2),
				FONT_HERSHEY_COMPLEX_SMALL, 0.5, cvScalar(0, 0, 250), 0.2,
				CV_AA);

		Point2f a = rectTops[0].pt;
		Point2f b = rectTops[1].pt;
		Point2f d = rectTops[2].pt;
		Point2f c = rectTops[3].pt;

		circle(imgRects, a, 3, Scalar(0, 0, 255), 1, 8);
		circle(imgRects, b, 3, Scalar(255, 0, 255), 1, 8);
		line(imgRects, a, b, Scalar(0, 0, 255), 1, LINE_AA);

		circle(imgRects, d, 3, Scalar(0, 255, 255), 1, 8);
		line(imgRects, a, d, Scalar(0, 0, 255), 1, LINE_AA);

		circle(imgRects, c, 3, Scalar(255, 0, 0), 1, 8);
		line(imgRects, d, c, Scalar(0, 0, 255), 1, LINE_AA);
		line(imgRects, b, c, Scalar(0, 0, 255), 1, LINE_AA);
	}

	imwrite(imageName, imgRects);

}

void drawRects(vector<RectF> rects, Mat src_ir, String imageName) {
	// draw the amount and position of solar panel
	size_t rectsSize = rects.size();

	Mat imgRects = src_ir;
	cvtColor(imgRects, imgRects, COLOR_GRAY2BGR);

	for (size_t i = 0; i < rectsSize; i++) {
		RectF rectTops = rects[i];

		Point2f a(rectTops.x1, rectTops.y1);
		Point2f b(rectTops.x2, rectTops.y2);
		Point2f c(rectTops.x3, rectTops.y3);
		Point2f d(rectTops.x4, rectTops.y4);

		putText(imgRects, (cv::String() + (i + 1)), Point(a.x, (a.y + d.y) / 2),
				FONT_HERSHEY_COMPLEX_SMALL, 0.5, cvScalar(0, 0, 250), 0.2,
				CV_AA);

		circle(imgRects, a, 3, Scalar(0, 0, 255), 1, 8);
		circle(imgRects, b, 3, Scalar(255, 0, 255), 1, 8);
		line(imgRects, a, b, Scalar(0, 0, 255), 1, LINE_AA);

		circle(imgRects, d, 3, Scalar(0, 255, 255), 1, 8);
		line(imgRects, a, d, Scalar(0, 0, 255), 1, LINE_AA);

		circle(imgRects, c, 3, Scalar(255, 0, 0), 1, 8);
		line(imgRects, d, c, Scalar(0, 0, 255), 1, LINE_AA);
		line(imgRects, b, c, Scalar(0, 0, 255), 1, LINE_AA);
	}

	imwrite(imageName, imgRects);

}

void drawRects(vector<RectV> rectsGroup, Mat src_ir, String imageName) {
	// draw the amount and position of solar panel
	size_t rectsSize = rectsGroup.size();
	cout << "Number of Solar Panel is: " << rectsSize << endl;

	Mat imgRects = src_ir;
	cvtColor(imgRects, imgRects, COLOR_GRAY2BGR);

	for (size_t i = 0; i < rectsSize; i++) {
		RectV rectTops = rectsGroup[i];
		putText(imgRects, (cv::String() + (i + 1)),
				Point(rectTops.a.x, (rectTops.a.y + rectTops.d.y) / 2),
				FONT_HERSHEY_COMPLEX_SMALL, 0.5, cvScalar(0, 0, 250), 0.2,
				CV_AA);

		Point2f a = rectTops.a;
		Point2f b = rectTops.b;
		Point2f c = rectTops.c;
		Point2f d = rectTops.d;

		circle(imgRects, a, 3, Scalar(0, 0, 255), 1, 8);
		circle(imgRects, b, 3, Scalar(255, 0, 255), 1, 8);
		line(imgRects, a, b, Scalar(0, 0, 255), 1, LINE_AA);

		circle(imgRects, d, 3, Scalar(0, 255, 255), 1, 8);
		line(imgRects, a, d, Scalar(0, 0, 255), 1, LINE_AA);

		circle(imgRects, c, 3, Scalar(255, 0, 0), 1, 8);
		line(imgRects, d, c, Scalar(0, 0, 255), 1, LINE_AA);
		line(imgRects, b, c, Scalar(0, 0, 255), 1, LINE_AA);
	}

	imwrite(imageName, imgRects);

}

void getPanels(cv::Mat src_ir, std::vector<RectF> &sRects,
		std::vector<cv::Vec4f> linesHorizontal,
		std::vector<cv::Vec4f> linesVertical) {
	// ************* remove Short Lines **************************

	vector<Vec4f> linesHorizontalLong = linesHorizontal;
	vector<Vec4f> linesVerticalLong = linesVertical;

	removeShortLines(linesHorizontal, minLineLengthPow);

	removeShortLines(linesVertical, minLineLengthPow);

#	ifdef PANEL_DETECTOR_DEBUG
	Mat imgLineRemoved = src_ir;
	cvtColor(imgLineRemoved, imgLineRemoved, COLOR_GRAY2BGR);

	drawLines(linesHorizontal, imgLineRemoved, "viet_imgLineRemoved.png");
	drawLines(linesVertical, imgLineRemoved, "viet_imgLineRemoved.png");
#	endif

	removeShortLines(linesHorizontalLong, minLineLongPow);
	removeShortLines(linesVerticalLong, minLineLongPow);

#	ifdef PANEL_DETECTOR_DEBUG
	Mat imgLineLong = src_ir;
	cvtColor(imgLineLong, imgLineLong, COLOR_GRAY2BGR);

	drawLines(linesHorizontalLong, imgLineLong, "viet_imgLine_Long.png");
	drawLines(linesVerticalLong, imgLineLong, "viet_imgLine_Long.png");
#	endif

	// ******************** Find Rectangle Model ****************************************
	vector<Point2f> rectModelTops;
	float widthPow = FLT_MAX;
	float lengthPow = FLT_MAX;

	findRectangleModel(linesHorizontalLong, linesVerticalLong, rectModelTops,
			minWidthPow, widthPow, lengthPow, 1.01, 0.01);

	if (rectModelTops.size() == 0) {
		return;
	}

#	ifdef PANEL_DETECTOR_DEBUG
	drawModelRect(rectModelTops, src_ir, "viet_ Rect_Model.png");
#	endif

	// ********* Find Intersection of perpendicular lines ************************************
	vector<InterPoint2f> iPts;
	getLinesIntersectionPoints(linesHorizontal, linesVertical, iPts, 0.2, 5,
			true);

#	ifdef PANEL_DETECTOR_DEBUG
	drawPoints(iPts, src_ir, "viet_Points.png");
# 	endif

	vector<vector<InterPoint2f> > rectsIP;

	// ******************** MODE 1: Find Rects based on Width and Length model ************************************
	findRects(iPts, rectsIP, widthPow, lengthPow, lengthPowRatio, 0.2);

#	ifdef PANEL_DETECTOR_DEBUG
	drawRects(rectsIP, src_ir, "viet_Rects.png");
#	endif

	// ********** convertRectEdges ******************************************
	convertRects(rectsIP, sRects);

}

bool haveLinesFunct(cv::Vec2f lineFunct, std::vector<cv::Vec2f> linesFunct,
		size_t &position, float aDiffRate, float bDiffRate) {
	size_t linesFunctSize = linesFunct.size();
	if (linesFunctSize == 0) {
		return false;
	} else {
		for (size_t i = 0; i < linesFunctSize; i++) {
			Vec2f lineFunctCheck = linesFunct[i];

			if (checkTwoNumbers(lineFunct[0], lineFunctCheck[0], aDiffRate)
					&& checkTwoNumbers(lineFunct[1], lineFunctCheck[1],
							bDiffRate)) {
				position = i;
				return true;
			}
		}
		return false;
	}
}
