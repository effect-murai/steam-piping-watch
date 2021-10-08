/*
 * Author : Le Dung, FPT
 * Date : 2018-06-15
 * Last-modified by : Le Dung, FPT
 * Last-modified : 2018-06-15
 */

#include "HoughLine.h"

#include <algorithm>
#include <functional>
#include <assert.h>
#include <float.h>
#include <string.h>
#include "LinearRegression.h"

std::vector<float> gSins;
std::vector<float> gCoses;
int gAngleBegin = -45;
int gAngleEnd = 45;

struct LocalHoughPoint {
	int position;
	int accumNum;
};

struct ThetaGroup {
	int minTheta;
	int maxTheta;

	std::vector<LinePolar> lines;
};

bool accumCompare(LocalHoughPoint p1, LocalHoughPoint p2) {
	return p1.accumNum > p2.accumNum;
}

void initSinCosTable() {
	float angle;
	int len = gAngleEnd - gAngleBegin;
	for (int i = 0; i < len; i++) {
		angle = DEGREES_TO_RADIANS(gAngleBegin + i);
		gSins.push_back(sin(angle));
		gCoses.push_back(cos(angle));
	}
}

int getThetaDistance(LinePolar line, ThetaGroup group) {
	if (line.theta >= group.minTheta && line.theta <= group.maxTheta) {
		return 0;
	} else {
		return std::min(abs(line.theta - group.minTheta),
				abs(line.theta - group.maxTheta));
	}
}

void expandRange(ThetaGroup &group, LinePolar line) {
	group.minTheta = std::min(group.minTheta, (int) line.theta);
	group.maxTheta = std::max(group.maxTheta, (int) line.theta);
	group.lines.push_back(line);
}

std::vector<LinePolar> getHoughLine(std::vector<PointI> points,
		int maxNumLine) {
	if (gSins.empty()) {
		initSinCosTable();
	}

	// look for max
	PointI minPoint, maxPoint;
	minPoint.x = INT_MAX;
	minPoint.y = INT_MAX;
	for (size_t i = 0; i < points.size(); i++) {
		if (points[i].x > maxPoint.x) {
			maxPoint.x = points[i].x;
		}

		if (points[i].y > maxPoint.y) {
			maxPoint.y = points[i].y;
		}

		if (points[i].x < minPoint.x) {
			minPoint.x = points[i].x;
		}

		if (points[i].y < minPoint.y) {
			minPoint.y = points[i].y;
		}
	}

	int maxRho = sqrt(maxPoint.x * maxPoint.x + maxPoint.y * maxPoint.y);

	float rho;
	int rho_int;
	int pos, posAngle;
	int w = maxRho;
	int h = gAngleEnd - gAngleBegin;
	assert(w > 0);
	assert(h > 0);
	int *accumulator = (int*) malloc(w * h * sizeof(int));
	memset(accumulator, 0, w * h * sizeof(int));

	for (size_t i = 0; i < points.size(); i++) {
		for (int theta = gAngleBegin; theta < gAngleEnd; theta++) {
			posAngle = theta - gAngleBegin;
			rho = points[i].x * gCoses[posAngle]
					+ points[i].y * gSins[posAngle];
			rho_int = (int) (rho + 0.5f);

			if (rho_int >= 0 && rho_int < maxRho) {
				pos = posAngle * w + rho_int;
				accumulator[pos]++;
			}
		}
	}

	// look for local maximum
	int maxAcc = 0;

	int rad = 3;
	int fromX, toX;
	int fromY, toY;
	int pos1;
	bool isLocalMaxCan;
	int numLess;
	std::vector<LocalHoughPoint> hPoints;
	for (int y = 0; y < h; y++) {
		pos = y * w;
		fromY = std::max(0, y - rad);
		toY = std::min(h, y + rad);
		for (int x = 0; x < w; x++) {
			fromX = std::max(0, x - rad);
			toX = std::min(w, x + rad);

			isLocalMaxCan = true;
			numLess = 0;
			for (int y1 = fromY; y1 < toY; y1++) {
				pos1 = y1 * w + fromX - 1;
				for (int x1 = fromX; x1 < toX; x1++) {
					if (accumulator[pos1] > accumulator[pos]) {
						isLocalMaxCan = false;
						break;
					} else if (accumulator[pos1] < accumulator[pos]) {
						numLess++;
					}

					pos1++;
				}
			}

			if (isLocalMaxCan
					&& numLess > ((toX - fromX) * (toY - fromY) / 2)) {
				// this is just for test
				if (maxAcc < accumulator[pos]) {
					maxAcc = accumulator[pos];
				}

				LocalHoughPoint pt;
				pt.position = pos;
				pt.accumNum = accumulator[pos];
				hPoints.push_back(pt);
			}

			pos++;
		}
	}

	// sort
	std::sort(hPoints.begin(), hPoints.end(), accumCompare);
	size_t maxSize = std::min(hPoints.size(), (size_t) maxNumLine);

	std::vector<LinePolar> lines;
	for (size_t i = 0; i < maxSize; i++) {
		LinePolar line;
		line.rho = hPoints[i].position % w;
		line.theta = hPoints[i].position / w + gAngleBegin;
		lines.push_back(line);
	}

	// remove "same" lines
	int thetaExpand = 2;
	std::vector<int> sameRhos;

	LinePolar line;
	std::vector<LinePolar> best_lines;
	for (size_t i = 0; i < lines.size(); i++) {
		sameRhos.clear();
		sameRhos.push_back(i);
		for (size_t j = i + 1; j < lines.size(); j++) {
			if (lines[i].rho == lines[j].rho) {
				sameRhos.push_back(j);
			}
		}

		//
		std::vector<ThetaGroup> groups;
		int closest;
		int dis;
		int minDis;
		for (size_t l = 0; l < sameRhos.size(); l++) {
			line = lines[sameRhos[l]];
			minDis = INT_MAX;
			closest = -1;
			for (size_t gr = 0; gr < groups.size(); gr++) {
				dis = getThetaDistance(line, groups[gr]);
				if (dis < minDis) {
					minDis = dis;
					closest = gr;

					if (dis == 0) {
						break;
					}
				}
			}

			if (closest != -1 && minDis <= thetaExpand) {
				//add to group
				expandRange(groups[closest], line);
			} else {
				// create new group
				ThetaGroup group;
				group.minTheta = line.theta;
				group.maxTheta = line.theta;
				group.lines.push_back(line);
				groups.push_back(group);
			}
		}

		// take only 1 line from group
		for (size_t j = 0; j < groups.size(); j++) {
			if (groups[j].lines.size() == 0) {
				continue;
			}

			// just take average theta
			float sum = 0;
			for (size_t k = 0; k < groups[j].lines.size(); k++) {
				sum += groups[j].lines[k].theta;
			}

			line.theta = sum / groups[j].lines.size();
			line.rho = groups[j].lines[0].rho;
			best_lines.push_back(line);
		}

		// remove the
		int countDeleted = 0;
		for (size_t l = 0; l < sameRhos.size(); l++) {
			lines.erase(lines.begin() + (sameRhos[l] - countDeleted));
			countDeleted++;
		}

		i = -1;	// force to do at begining
	}

	free(accumulator);

	return best_lines;
}

bool getHoughLine(std::vector<PointI> points, float maxDis, float &error,
		float &a, float &b) {
	return false;
}

LineLinear convert2LinearLine(LinePolar l) {
	LineLinear line;

	float theta = DEGREES_TO_RADIANS(l.theta);
	float sinTheta = sin(theta);
	float cosTheta = cos(theta);
	if (sinTheta == 0) {
		// work around
		line.a = 1000.f;
		line.b = -line.a * (l.rho / cosTheta);
	} else {
		line.a = -cosTheta / sinTheta;
		line.b = l.rho / sinTheta;
	}

	return line;
}

void getError(std::vector<PointI> points, float a, float b, float maxDistance,
		float &error, std::vector<PointI> &good_points) {
	good_points.clear();

	float limDis2 = maxDistance * maxDistance;
	float dis;
	error = 0;
	for (size_t i = 0; i < points.size(); i++) {
		dis = squareDistancePointToLine(points[i].x, points[i].y, a, b);

		if (dis < limDis2) {
			good_points.push_back(points[i]);
		}

		error += dis;
	}

	error /= points.size();
}

bool getBestLine(std::vector<PointI> points, float maxDistance, float &error,
		float &a, float &b) {
	// get the candidates
	std::vector<LinePolar> hlines = getHoughLine(points, 3);

	// convert to linear
	std::vector<LineLinear> lines;
	for (size_t i = 0; i < hlines.size(); i++) {
		lines.push_back(convert2LinearLine(hlines[i]));
	}

	float tmp_error;
	int best = -1;
	float score;
	float bestScore = 0;
	std::vector<PointI> best_points;
	std::vector<PointI> tmp_points;
	for (size_t i = 0; i < lines.size(); i++) {
		getError(points, lines[i].a, lines[i].b, maxDistance, tmp_error,
				tmp_points);
		score = ((float) tmp_points.size() / points.size());

		if (score > bestScore) {
			bestScore = score;
			best = i;
			best_points = tmp_points;
		}
	}

	if (best == -1) {
		return false;
	}

	// get more exactly line
	bool ret = getLineT(best_points, maxDistance * 2.f, error, a, b);
	if (!ret) {
		return false;
	}

	if (best_points.size() < 3) {
		return false;
	}

	return true;
}
