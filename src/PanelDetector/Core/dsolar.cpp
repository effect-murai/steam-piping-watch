/*
 * Author : Le Dung , FPT
 * Date : 2018-06-15
 * Last-modified by : Le Dung, FPT
 * Last-modified : 2018-06-15
 */

#include "dsolar.h"
#include "../Geometry/Vector2f.h"
#include "GlobalVar.h"
#include "../SAlgorithm/SAlgorithm.h"
#include "../Geometry/geometry_util.h"
#include "HoughLine.h"
#include "LinearRegression.h"

float g_disFromLine = 3;
float g_disFromLineRomove = 2;

int gMaxAngle = 15; // degree

int gStopDetect = 0;

struct EquiGroup {
	std::vector<int> members;
	float distance;
	float firstPos;
	float lastPos;
	float error;
	int getNumDistance() {
		return members.size() - 1;
	}
	int getNumError() {
		return members.size() - 2;
	}

};

struct LineSeg {
	Vector2F pt1, pt2;
	Vector2F direction;
	float error;
	float density;
	float length;
	std::vector<PointI> points;

	float minX, maxX;
	float minY, maxY;

	float a, b;

	LineSeg() {
		error = 0;
		density = 0;
		a = 0;
		b = 0;
		minX = 0;
		minY = 0;
		length = 0;
		maxX = 0;
		maxY = 0;
	}

	void calcDensity() {
		length = distance(pt1.x, pt1.y, pt2.x, pt2.y);
		density = points.size() / length;
	}

	void transpose() {
		pt1.transpose();
		pt2.transpose();
		direction.transpose();

		// no need so far to transpose points
		// points
	}

	void getExpandBuffer(float longExpand, float perpendExpand, Vector2F &p1,
			Vector2F &p2, Vector2F &p3, Vector2F &p4) {
		Vector2F vecPerpen;
		vecPerpen.x = direction.y;
		vecPerpen.y = -direction.x;

		Vector2F newPt1 = pt1 - direction * longExpand;
		p1 = newPt1 - vecPerpen * perpendExpand;
		p2 = newPt1 + vecPerpen * perpendExpand;

		Vector2F newPt2 = pt2 + direction * longExpand;
		p4 = newPt2 - vecPerpen * perpendExpand;
		p3 = newPt2 + vecPerpen * perpendExpand;
	}

	inline bool isBoxIntersect(const LineSeg &line) {
		if (minX > line.maxX) {
			return false;
		}
		if (maxX < line.minX) {
			return false;
		}
		if (minY > line.maxY) {
			return false;
		}
		if (maxY < line.minY) {
			return false;
		}

		return true;
	}

	// for simple line - 2 points only
	bool isSame2(const LineSeg &line) {
		if (points.size() != 2) {
			return false;
		}

		if (line.points.size() != 2) {
			return false;
		}

		if (points[0].isSame(line.points[0])
				&& points[1].isSame(line.points[1])) {
			return true;
		}

		if (points[0].isSame(line.points[1])
				&& points[1].isSame(line.points[0])) {
			return true;
		}

		return false;
	}

	float getSumError(float a, float b) const {
		float error = 0;
		for (size_t i = 0; i < points.size(); i++) {
			error += squareDistancePointToLine(points[i].x, points[i].y, a, b);
		}

		return error;
	}
};

struct LineSeg2: public LineSeg {
	lr_support lrSupporter;

	void calcSupport() {
		lrSupporter.calc(points);
	}
};

// use only for data type - uchar
bool cvMatFromBinary(unsigned char *data, int width, int height, cv::Mat &mat) {
	mat.release();

	int rows = height;
	int cols = width;
	int type = CV_8U;

	try {
		mat.create(rows, cols, type);
		memcpy((char*) (mat.data), data, mat.elemSize() * mat.total());
	} catch (...) {
		std::cout << "Create mat error" << std::endl;
		return false;
	}

	return true;
}

void printMyd(std::string pathIn, std::string pathOut, DPair **mydImage,
		int width, int height) {
	// test
	cv::Mat image = cv::imread(pathIn);

	int pos;
	for (int y = 0; y < height; y++) {
		pos = y * width;
		for (int x = 0; x < width; x++) {
			if (mydImage[pos]) {
				cv::circle(image, cv::Point(x, y), 1, cv::Scalar(0, 255, 255));
			}

			pos++;
		}
	}

	imwrite(pathOut, image);
}

void printLines(std::string pathIn, std::string pathOut,
		std::vector<LineSeg> lines) {
	// test
	cv::Mat image = cv::imread(pathIn);

	for (size_t i = 0; i < lines.size(); i++) {
		cv::line(image, cv::Point(lines[i].pt1.x, lines[i].pt1.y),
				cv::Point(lines[i].pt2.x, lines[i].pt2.y),
				cv::Scalar(0, 255, 255), 1);
	}

	imwrite(pathOut, image);
}

void printLines3(std::string pathIn, std::string pathOut,
		std::vector<LineSeg2> lines) {
	// test
	cv::Mat image = cv::imread(pathIn);

	for (size_t i = 0; i < lines.size(); i++) {
		cv::line(image, cv::Point(lines[i].pt1.x, lines[i].pt1.y),
				cv::Point(lines[i].pt2.x, lines[i].pt2.y),
				cv::Scalar(0, 255, 255), 1);
	}

	imwrite(pathOut, image);
}

void printMyd2(std::string pathIn, std::string pathOut, DPoint **mydImage,
		int width, int height) {
	// test
	cv::Mat image = cv::imread(pathIn);

	int pos;
	for (int y = 0; y < height; y++) {
		pos = y * width;
		for (int x = 0; x < width; x++) {
			if (mydImage[pos])
				cv::line(image, cv::Point(x, y), cv::Point(x, y),
						cv::Scalar(0, 255, 255), 1);

			pos++;
		}
	}

	imwrite(pathOut, image);
}

void printMyd3(std::string pathIn, std::string pathOut, unsigned char *mydImage,
		int width, int height) {
	// test
	cv::Mat image = cv::imread(pathIn);

	int pos;
	for (int y = 0; y < height; y++) {
		pos = y * width;
		for (int x = 0; x < width; x++) {
			if (mydImage[pos] > 0)
				cv::line(image, cv::Point(x, y), cv::Point(x, y),
						cv::Scalar(0, 255, 255), 1);

			pos++;
		}
	}

	imwrite(pathOut, image);
}

// point (x, y) lay on line, return the shortest distance to line
float shortDistance2Line(LineSeg line, float x, float y) {
	Vector2F vec1_point(x - line.pt1.x, y - line.pt1.y);
	float dot = Vector2F::dot(vec1_point, line.direction);
	if (dot > 0) {
		return distance(x, y, line.pt2.x, line.pt2.y);
	} else {
		return distance(x, y, line.pt1.x, line.pt1.y);
	}
}

// return the error : distance from intersection to line(s)
float twoLineIntersection(const LineSeg &line1, const LineSeg &line2,
		float &xIntersect, float &yIntersect) {
	int res = twoLineIntersection(line1.pt1.x, line1.pt1.y, line1.pt2.x,
			line1.pt2.y, line2.pt1.x, line2.pt1.y, line2.pt2.x, line2.pt2.y,
			xIntersect, yIntersect);

	switch (res) {
	case 3:
		return 0;
	case 2:
		// 2 : intersection is on second segment only
		return shortDistance2Line(line1, xIntersect, yIntersect);
	case 1:
		// 1 : intersection is on first segment only
		return shortDistance2Line(line2, xIntersect, yIntersect);
	case 0:
		// 0 : intersection is out of 2 segments
		return (shortDistance2Line(line1, xIntersect, yIntersect)
				+ shortDistance2Line(line2, xIntersect, yIntersect));
	default:
		// should not to be cause 2 line almost perpendicular
		assert(false);
		return FLT_MAX;
	}
}

float twoLineIntersection(Vector2F pt1, Vector2F pt2, Vector2F pt3,
		Vector2F pt4, float &xIntersect, float &yIntersect) {
	return twoLineIntersection(pt1.x, pt1.y, pt2.x, pt2.y, pt3.x, pt3.y, pt4.x,
			pt4.y, xIntersect, yIntersect);
}

void getTranspose(unsigned char *image, int width, int height,
		unsigned char *image_t) {
	int pos;
	for (int y = 0; y < width; y++) {
		pos = y * height;
		for (int x = 0; x < height; x++) {
			image_t[pos] = image[x * width + y];
			pos++;
		}
	}
}

void getCrop(unsigned char *image, int stride, int width, int height, int left,
		int top, int w, int h, unsigned char *image_crop) {
	// .....not check rect so far, do later

	for (int y = 0; y < h; y++) {
		memcpy(image_crop + y * w, image + (y + top) * stride + left, w);
	}
}

bool areWeHere(int i1, int i2, std::vector<int> array) {
	if (std::find(array.begin(), array.end(), i1) != array.end()
			&& std::find(array.begin(), array.end(), i2) != array.end()) {
		return true;
	}

	return false;
}

void estimateConnection(const LineSeg &line1, const LineSeg &line2,
		float &error, float &a, float &b) {
	std::vector<PointI> points = line1.points;
	points.insert(points.end(), line2.points.begin(), line2.points.end());

	if (!getLine(points, g_disFromLine, error, a, b)) {
		error = FLT_MAX;
	}
}

void estimateConnection2(const LineSeg &line1, const LineSeg &line2,
		float &error, float &a, float &b) {
	std::vector<PointI> points = line1.points;
	points.insert(points.end(), line2.points.begin(), line2.points.end());

	if (!getLineExclude(points, g_disFromLineRomove, error, a, b)) {
		error = FLT_MAX;
	}
}

void estimateConnection3(const LineSeg &line1, const LineSeg &line2,
		float &error, float &a, float &b) {
	std::vector<PointI> points = line1.points;
	points.insert(points.end(), line2.points.begin(), line2.points.end());

	if (!getBestLine(points, g_disFromLineRomove, error, a, b)) {
		error = FLT_MAX;
	}
}

void getError(std::vector<PointI> points1, std::vector<PointI> points2, float a,
		float b, float maxDistance, float &error,
		std::vector<PointI> &good_points) {
	std::vector<PointI> points = points1;
	points.insert(points.end(), points2.begin(), points2.end());

	getError(points, a, b, maxDistance, error, good_points);
}

// -1 : error
//  0 : new is best
//  1 : line1 is best
//  2 : line2 is best
int estimateConnection4(LineSeg2 *line1, LineSeg2 *line2, float &error,
		float &a, float &b) {
	if (line1->lrSupporter.sumX == 0) {
		line1->calcSupport();
	}

	if (line2->lrSupporter.sumX == 0) {
		line2->calcSupport();
	}

	lr_support ls = line1->lrSupporter + line2->lrSupporter;
	float pa, pb;
	ls.getLine((int) (line1->points.size() + line2->points.size()), pa, pb);

	int ret = -1;

	float dis_2 = g_disFromLineRomove * g_disFromLineRomove;

	float tmp_error;
	float score;
	float bestScore = 0;
	std::vector<PointI> best_points;
	std::vector<PointI> tmp_points;

	// danh gia line chung
	getError(line1->points, line2->points, pa, pb, g_disFromLineRomove,
			tmp_error, tmp_points);
	score = ((float) tmp_points.size()
			/ (line1->points.size() + line2->points.size())) * 0.65f
			+ ((dis_2 - tmp_error) / dis_2) * 0.35f;

	if (score > bestScore) {
		bestScore = score;
		best_points = tmp_points;
		error = tmp_error;
		a = pa;
		b = pb;
		ret = 0;
	}

	// danh gia theo line1
	getError(line1->points, line2->points, line1->a, line1->b,
			g_disFromLineRomove, tmp_error, tmp_points);
	score = ((float) tmp_points.size()
			/ (line1->points.size() + line2->points.size())) * 0.65f
			+ ((dis_2 - tmp_error) / dis_2) * 0.35f;

	if (score > bestScore) {
		bestScore = score;
		best_points = tmp_points;
		error = tmp_error;
		a = line1->a;
		b = line1->b;
		ret = 1;
	}

	// danh gia theo line2
	getError(line1->points, line2->points, line2->a, line2->b,
			g_disFromLineRomove, tmp_error, tmp_points);
	score = ((float) tmp_points.size()
			/ (line1->points.size() + line2->points.size())) * 0.65f
			+ ((dis_2 - tmp_error) / dis_2) * 0.35f;

	if (score > bestScore) {
		bestScore = score;
		best_points = tmp_points;
		error = tmp_error;
		a = line2->a;
		b = line2->b;
		ret = 2;
	}

	return ret;
}

void estimateConnection5(LineSeg2 *line1, LineSeg2 *line2, float &error,
		float &a, float &b) {
	if (line1->lrSupporter.sumX == 0) {
		line1->calcSupport();
	}

	if (line2->lrSupporter.sumX == 0) {
		line2->calcSupport();
	}

	lr_support ls = line1->lrSupporter + line2->lrSupporter;
	float pa, pb;
	ls.getLine((int) (line1->points.size() + line2->points.size()), pa, pb);

	error = line1->getSumError(pa, pb);
	error += line2->getSumError(pa, pb);
	error /= (line1->points.size() + line2->points.size());
}

void createLine(LineSeg &line, std::vector<PointI> points, float a, float b,// y = a*x + b
		float error) {
	assert(points.size() >= 2);
	Vector2F tmpDir = Vector2F::normalize(Vector2F(1, a));
	Vector2F pointOnLine(0, b);
	line.pt1 = projectionPointOnLine1(Vector2F(points[0].x, points[0].y),
			pointOnLine, tmpDir);
	line.pt2 = projectionPointOnLine1(
			Vector2F(points[points.size() - 1].x, points[points.size() - 1].y),
			pointOnLine, tmpDir);

	// get direction from 1 -> 2
	line.direction = Vector2F::normalize(line.pt2 - line.pt1);

	line.error = error;
	line.points = points;

	line.minX = std::min(line.pt1.x, line.pt2.x);
	line.maxX = std::max(line.pt1.x, line.pt2.x);
	line.minY = std::min(line.pt1.y, line.pt2.y);
	line.maxY = std::max(line.pt1.y, line.pt2.y);

	line.a = a;
	line.b = b;

	line.calcDensity();
}

LineSeg createLine(std::vector<PointI> points, float a, float b,// y = a*x + b
		float error) {
	assert(points.size() >= 2);
	LineSeg line;
	createLine(line, points, a, b, error);
	return line;
}

LineSeg2 createLine2(std::vector<PointI> points, float a, float b,// y = a*x + b
		float error) {
	assert(points.size() >= 2);
	LineSeg2 line;
	createLine(line, points, a, b, error);
	return line;
}

LineSeg createLine(PointI pt1, PointI pt2) {
	LineSeg line;

	line.pt1.x = pt1.x;
	line.pt1.y = pt1.y;

	line.pt2.x = pt2.x;
	line.pt2.y = pt2.y;

	// get direction from 1 -> 2
	line.direction = Vector2F::normalize(line.pt2 - line.pt1);

	line.error = 0;
	line.points.push_back(pt1);
	line.points.push_back(pt2);

	line.minX = std::min(line.pt1.x, line.pt2.x);
	line.maxX = std::max(line.pt1.x, line.pt2.x);
	line.minY = std::min(line.pt1.y, line.pt2.y);
	line.maxY = std::max(line.pt1.y, line.pt2.y);

	line.calcDensity();

	return line;
}

// connect to line1
// y = a*x + b
void connect2Line(LineSeg &line1, const LineSeg &line2, float a, float b,
		float error) {
	Vector2F orientDir =
			line1.points.size() > line2.points.size() ?
					line1.direction : line2.direction;

	// just sort all point again in the main direction
	std::vector<PointI> points = line1.points;
	points.insert(points.end(), line2.points.begin(), line2.points.end());
	std::vector<float> projDistances;
	float dot;
	for (size_t i = 0; i < points.size(); i++) {
		dot = Vector2F::dot(orientDir, Vector2F(points[i].x, points[i].y));
		projDistances.push_back(dot);
	}

	// simple sort
	PointI tmp_point;
	float tmp;
	for (size_t i = 0; i < points.size(); i++) {
		for (size_t j = i + 1; j < points.size(); j++) {
			if (projDistances[i] > projDistances[j]) {
				tmp = projDistances[i];
				projDistances[i] = projDistances[j];
				projDistances[j] = tmp;

				tmp_point = points[i];
				points[i] = points[j];
				points[j] = tmp_point;
			}
		}
	}

	createLine(line1, points, a, b, error);
}

// connect to line1
// y = a*x + b
void connect2Line2(LineSeg2 &line1, const LineSeg2 &line2, float a, float b,
		float error) {
	connect2Line(line1, line2, a, b, error);

	line1.lrSupporter += line2.lrSupporter;
}

void getLines(unsigned char *image, int width, int height, int fromX, int toX,
		int fromY, int toY) {
	int pos, pos1;
	int lineGap = 4;
	PointI point;
	std::vector<PointI> points;
	int fromY1, toY1;
	float a_temp, b_temp, error_temp, a, b, error;

	int offset_pos[] = { 0, -1, 1 };
	bool found;

	std::vector<LineSeg> lines;
	unsigned char *mark = new unsigned char[width * height];
	memset(mark, 0, width * height);

	for (int y = fromY; y < toY; y++) {
		pos = y * width + fromX - 1;
		for (int x = fromX; x < toX; x++) {
			pos++;
			if (image[pos] == 0) {
				continue;
			}

			if (mark[pos] == 1) {
				continue;
			}

			point.x = x;
			point.y = y;

			points.clear();
			points.push_back(point);

			while (true) {
				point = points[points.size() - 1];
				fromY1 = point.y + 1;
				toY1 = std::min(fromY1 + lineGap, height);

				found = false;
				for (int y1 = fromY1; y1 < toY1; y1++) {
					for (int i = 0; i < 3; i++) {
						int x1 = point.x + offset_pos[i];
						if (x1 < 0 || x1 >= width) {
							continue;
						}

						pos1 = y1 * width + x1;
						if (image[pos1] == 0) {
							continue;
						}

						// found the candidate point
						// get the line
						std::vector<PointI> temp_points = points;
						temp_points.push_back(PointI(x1, y1));
						if (!getLine(temp_points, g_disFromLine, error_temp,
								a_temp, b_temp)) {
							continue;
						}

						// add good point
						points.push_back(PointI(x1, y1));
						a = a_temp;
						b = b_temp;
						error = error_temp;

						mark[pos1] = 1;

						// break loops;
						y1 = 1000000000;

						found = true;

						break;
					}
				}

				if (!found) {
					// check if the line is good

					// form the line
					if (points.size() >= 2) {
						LineSeg line = createLine(points, a, b, error);
						lines.push_back(line);
					}

					break;
				}
			}
		}
	}

	delete[] mark;

	// get the main direction
	float longerComp = 0;
	Vector2F mainDirection;
	for (size_t i = 0; i < lines.size(); i++) {
		longerComp = std::max(fabs(lines[i].pt2.y - lines[i].pt1.y),
				fabs(lines[i].pt2.x - lines[i].pt1.x));
		mainDirection += lines[i].direction * longerComp;
	}

	mainDirection = Vector2F::normalize(mainDirection);

	// connect the lines
	bool foundConnection;
	Vector2F conDirection;
	float minX, minY, maxX, maxY;
	int horzExpand = 10;
	int vertExpand = 30;

	while (true) {
		foundConnection = false;
		for (size_t i = 0; i < lines.size(); i++) {
			// expand the box
			minX = lines[i].minX - horzExpand;
			maxX = lines[i].maxX + horzExpand;
			minY = lines[i].minY - vertExpand;
			maxY = lines[i].maxY + vertExpand;

			for (size_t j = i + 1; j < lines.size(); j++) {
				if (lines[j].maxX < minX || lines[j].minX > maxX) {
					continue;
				}
				if (lines[j].maxY < minY || lines[j].minY > maxY) {
					continue;
				}

				estimateConnection(lines[i], lines[j], error, a, b);

				if (error < g_disFromLine) {
					// connect
					connect2Line(lines[i], lines[j], a, b, error);

					// delete line j
					lines.erase(lines.begin() + j);
					j--;

					foundConnection = true;
				}
			}
		}

		if (!foundConnection) {
			break;
		}
	}

	// remove weak line
	for (size_t i = 0; i < lines.size(); i++) {
		if (lines[i].length < 4) {
			lines.erase(lines.begin() + i);
			i--;
			continue;
		}

		if (lines[i].density < 0.3f) {
			lines.erase(lines.begin() + i);
			i--;
		}
	}

	// remove not-dir line
	float dot;
	float dot25 = 0.9063f;
	for (size_t i = 0; i < lines.size(); i++) {
		dot = Vector2F::dot(mainDirection, lines[i].direction);
		if (dot < dot25) {
			lines.erase(lines.begin() + i);
			i--;
		}
	}

	// test
	cv::Mat test_image = cv::imread("solar_vert_binary.png");
	cv::Mat test_image2 = cv::imread("blurred.png");

	for (size_t i = 0; i < lines.size(); i++) {
		cv::line(test_image, cv::Point(lines[i].pt1.x, lines[i].pt1.y),
				cv::Point(lines[i].pt2.x, lines[i].pt2.y),
				cv::Scalar(0, 255, 255), 1);
		cv::line(test_image2, cv::Point(lines[i].pt1.x, lines[i].pt1.y),
				cv::Point(lines[i].pt2.x, lines[i].pt2.y),
				cv::Scalar(0, 255, 255), 1);
	}

#	ifdef PANEL_DETECTOR_DEBUG
	imwrite("solar_vert_binary_lines.png", test_image);
	imwrite("solar_vert_binary_lines2.png", test_image2);
#	endif
}

void getLines2(DPair **image, int width, int height, int fromX, int toX,
		int fromY, int toY) {
	int pos, pos1;
	int lineGap = 4;
	PointI point;
	std::vector<PointI> points;
	int fromY1, toY1;
	float a_temp, b_temp, error_temp, a, b, error;

	int offset_pos[] = { 0, -1, 1 };
	bool found;

	std::vector<LineSeg> lines;
	unsigned char *mark = new unsigned char[width * height];
	memset(mark, 0, width * height);

	for (int y = fromY; y < toY; y++) {
		pos = y * width + fromX - 1;
		for (int x = fromX; x < toX; x++) {
			pos++;
			if (image[pos] == NULL) {
				continue;
			}

			if (mark[pos] == 1) {
				continue;
			}

			point.x = x;
			point.y = y;

			points.clear();
			points.push_back(point);

			while (true) {
				point = points[points.size() - 1];
				fromY1 = point.y + 1;
				toY1 = std::min(fromY1 + lineGap, height);

				found = false;
				for (int y1 = fromY1; y1 < toY1; y1++) {
					for (int i = 0; i < 3; i++) {
						int x1 = point.x + offset_pos[i];
						if (x1 < 0 || x1 >= width) {
							continue;
						}

						pos1 = y1 * width + x1;
						if (image[pos1] == 0) {
							continue;
						}

						if (mark[pos1] == 1) {
							continue;
						}

						// found the candidate point
						// get the line
						std::vector<PointI> temp_points = points;
						temp_points.push_back(PointI(x1, y1));
						if (!getLine(temp_points, g_disFromLine, error_temp,
								a_temp, b_temp)) {
							continue;
						}

						// add good point
						points.push_back(PointI(x1, y1));
						a = a_temp;
						b = b_temp;
						error = error_temp;

						mark[pos1] = 1;

						// break loops;
						y1 = 1000000000;

						found = true;

						break;
					}
				}

				if (!found) {
					// check if the line is good

					// form the line
					if (points.size() >= 2) {
						LineSeg line = createLine(points, a, b, error);
						lines.push_back(line);
					}

					break;
				}
			}
		}
	}

	delete[] mark;

	// get the main direction
	float longerComp = 0;
	Vector2F mainDirection;
	for (size_t i = 0; i < lines.size(); i++) {
		longerComp = std::max(fabs(lines[i].pt2.y - lines[i].pt1.y),
				fabs(lines[i].pt2.x - lines[i].pt1.x));
		mainDirection += lines[i].direction * longerComp;
	}

	mainDirection = Vector2F::normalize(mainDirection);

	// connect the lines
	bool foundConnection;
	Vector2F conDirection;
	float minX, minY, maxX, maxY;
	int horzExpand = 10;
	int vertExpand = 30;

	while (true) {
		foundConnection = false;
		for (size_t i = 0; i < lines.size(); i++) {
			// expand the box
			minX = lines[i].minX - horzExpand;
			maxX = lines[i].maxX + horzExpand;
			minY = lines[i].minY - vertExpand;
			maxY = lines[i].maxY + vertExpand;

			for (size_t j = i + 1; j < lines.size(); j++) {
				if (lines[j].maxX < minX || lines[j].minX > maxX) {
					continue;
				}
				if (lines[j].maxY < minY || lines[j].minY > maxY) {
					continue;
				}

				estimateConnection(lines[i], lines[j], error, a, b);

				if (error < g_disFromLine) {
					// connect
					connect2Line(lines[i], lines[j], a, b, error);

					// delete line j
					lines.erase(lines.begin() + j);
					j--;

					foundConnection = true;
				}
			}
		}

		if (!foundConnection) {
			break;
		}
	}

	// remove weak line
	for (size_t i = 0; i < lines.size(); i++) {
		if (lines[i].length < 4) {
			lines.erase(lines.begin() + i);
			i--;
			continue;
		}

		if (lines[i].density < 0.3f) {
			lines.erase(lines.begin() + i);
			i--;
		}
	}

	// remove not-dir line
	float dot;
	float dot25 = 0.9063f;
	for (size_t i = 0; i < lines.size(); i++) {
		dot = Vector2F::dot(mainDirection, lines[i].direction);
		if (dot < dot25) {
			lines.erase(lines.begin() + i);
			i--;
		}
	}

	// test
	cv::Mat test_image = cv::imread("solar_vert_binary.png");
	cv::Mat test_image2 = cv::imread("blurred.png");

	for (size_t i = 0; i < lines.size(); i++) {
		cv::line(test_image, cv::Point(lines[i].pt1.x, lines[i].pt1.y),
				cv::Point(lines[i].pt2.x, lines[i].pt2.y),
				cv::Scalar(0, 255, 255), 1);
		cv::line(test_image2, cv::Point(lines[i].pt1.x, lines[i].pt1.y),
				cv::Point(lines[i].pt2.x, lines[i].pt2.y),
				cv::Scalar(0, 255, 255), 1);
	}

#	ifdef PANEL_DETECTOR_DEBUG
	imwrite("solar_vert_binary_lines.png", test_image);
	imwrite("solar_vert_binary_lines2.png", test_image2);
#	endif
}

bool isLineIntersecQuad(Vector2F p1, Vector2F p2,				// line
		Vector2F pQ1, Vector2F pQ2, Vector2F pQ3, Vector2F pQ4	// quad
		) {
	float xI, yI;
	if (twoLineIntersection(p1, p2, pQ1, pQ2, xI, yI) == 3) {
		return true;
	}

	if (twoLineIntersection(p1, p2, pQ2, pQ3, xI, yI) == 3) {
		return true;
	}

	if (twoLineIntersection(p1, p2, pQ3, pQ4, xI, yI) == 3) {
		return true;
	}

	if (twoLineIntersection(p1, p2, pQ4, pQ1, xI, yI) == 3) {
		return true;
	}

	return false;
}

bool isPotentialParallel(LineSeg line1, LineSeg line2, float lenExpand) {
	Vector2F buf1P1, buf1P2, buf1P3, buf1P4;
	Vector2F buf2P1, buf2P2, buf2P3, buf2P4;

	line1.getExpandBuffer(lenExpand, 3, buf1P1, buf1P2, buf1P3, buf1P4);
	line2.getExpandBuffer(lenExpand, 3, buf2P1, buf2P2, buf2P3, buf2P4);

	if (isLineIntersecQuad(buf1P1, buf1P2, buf2P1, buf2P2, buf2P3, buf2P4)) {
		return true;
	}

	if (isLineIntersecQuad(buf1P2, buf1P3, buf2P1, buf2P2, buf2P3, buf2P4)) {
		return true;
	}

	if (isLineIntersecQuad(buf1P3, buf1P4, buf2P1, buf2P2, buf2P3, buf2P4)) {
		return true;
	}

	if (isLineIntersecQuad(buf1P4, buf1P1, buf2P1, buf2P2, buf2P3, buf2P4)) {
		return true;
	}

	return false;
}

std::vector<LineSeg> getLines3(unsigned char *image, int width, int height) {
	int pos, pos1;
	int lineGap = 4;
	PointI point;
	std::vector<PointI> points;
	int fromY1, toY1;
	float a_temp, b_temp, error_temp, a, b, error;

	int offset_pos[] = { 0, -1, 1, -2, 2 };
	const int len = sizeof(offset_pos) / sizeof(int);
	bool found;

	std::vector<LineSeg> lines;
	unsigned char *mark = new unsigned char[width * height];
	memset(mark, 0, width * height);

	for (int y = 0; y < height; y++) {
		pos = y * width - 1;
		for (int x = 0; x < width; x++) {
			pos++;
			if (image[pos] == 0) {
				continue;
			}

			if (mark[pos] == 1) {
				continue;
			}

			point.x = x;
			point.y = y;

			points.clear();
			points.push_back(point);

			while (true) {
				point = points[points.size() - 1];
				fromY1 = point.y + 1;
				toY1 = std::min(fromY1 + lineGap, height);

				found = false;
				for (int y1 = fromY1; y1 < toY1; y1++) {
					for (int i = 0; i < len; i++) {
						int x1 = point.x + offset_pos[i];
						if (x1 < 0 || x1 >= width) {
							continue;
						}

						pos1 = y1 * width + x1;
						if (image[pos1] == 0) {
							continue;
						}

						if (mark[pos1] == 1) {
							continue;
						}

						// found the candidate point
						// get the line
						std::vector<PointI> temp_points = points;
						temp_points.push_back(PointI(x1, y1));
						if (!getLineT(temp_points, g_disFromLine, error_temp,
								a_temp, b_temp)) {
							continue;
						}

						// add good point
						points.push_back(PointI(x1, y1));
						a = a_temp;
						b = b_temp;
						error = error_temp;

						mark[pos1] = 1;

						// break loops;
						y1 = 1000000000;

						found = true;

						break;
					}
				}

				if (!found) {
					// check if the line is good

					// form the line
					if (points.size() >= 2) {
						LineSeg line = createLine(points, a, b, error);
						lines.push_back(line);
					}

					break;
				}
			}
		}
	}

	delete[] mark;

	// connect the lines
	bool foundConnection;
	Vector2F conDirection;
	float minX, minY, maxX, maxY;
	int vertExpand = (int) (height * 0.4f);
	int horzExpand = std::min((int) (width * 0.25f), 6);

	while (true) {
		foundConnection = false;
		for (size_t i = 0; i < lines.size(); i++) {
			// expand the box
			minX = lines[i].minX - horzExpand;
			maxX = lines[i].maxX + horzExpand;
			minY = lines[i].minY - vertExpand;
			maxY = lines[i].maxY + vertExpand;

			for (size_t j = i + 1; j < lines.size(); j++) {
				if (lines[j].maxX < minX || lines[j].minX > maxX) {
					continue;
				}
				if (lines[j].maxY < minY || lines[j].minY > maxY) {
					continue;
				}

				// check more
				if (!isPotentialParallel(lines[i], lines[j], vertExpand)) {
					continue;
				}

				estimateConnection3(lines[i], lines[j], error, a, b);

				if (error < g_disFromLine) {
					// connect
					connect2Line(lines[i], lines[j], a, b, error);

					// delete line j
					lines.erase(lines.begin() + j);
					j--;

					foundConnection = true;
				}
			}
		}

		if (!foundConnection) {
			break;
		}
	}

	// remove weak line
	for (size_t i = 0; i < lines.size(); i++) {
		if (lines[i].length < 4) {
			lines.erase(lines.begin() + i);
			i--;
			continue;
		}

		if (lines[i].density < 0.3f) {
			lines.erase(lines.begin() + i);
			i--;
		}
	}

	return lines;
}

std::vector<LineSeg> getLines4(unsigned char *image, int width, int height) {
	std::vector<LineSeg> lines;

	cv::Mat binMat;
	if (!cvMatFromBinary(image, width, height, binMat)) {
		return lines;
	}

	std::vector<cv::Vec4i> hlines;
	int numVote = std::max(height / 6, 6);
	double minLen = height / 4;
	HoughLinesP(binMat, hlines, 1, CV_PI / 180, numVote, minLen, 15);

	for (size_t i = 0; i < hlines.size(); i++) {
		LineSeg l;
		l.pt1.x = hlines[i][0];
		l.pt1.y = hlines[i][1];

		l.pt2.x = hlines[i][2];
		l.pt2.y = hlines[i][3];

		l.direction = Vector2F::normalize(l.pt2 - l.pt1);
		l.length = distance(l.pt1.x, l.pt1.y, l.pt2.x, l.pt2.y);

		lines.push_back(l);
	}

	return lines;
}

// A recursive binary search function. It returns 
// location of x in given array arr[l..r] is present, 
// otherwise -1
int binarySearch(const LineSeg *arr, int l, int r, int x) {
	if (r >= l) {
		int mid = l + (r - l) / 2;

		// If the element is present at the middle 
		// itself
		if (arr[mid].minX == x) {
			return mid;
		}

		// If element is smaller than mid, then 
		// it can only be present in left subarray
		if (arr[mid].minX > x) {
			return binarySearch(arr, l, mid - 1, x);
		}

		// Else the element can only be present
		// in right subarray
		return binarySearch(arr, mid + 1, r, x);
	}

	// We reach here when element is not 
	// present in array
	return -1;
}

int searchLinePos(const std::vector<LineSeg> lines, int beginFromX) {
	int pos = binarySearch(lines.data(), 0, lines.size() - 1, beginFromX);
	if (pos == -1) {
		return -1;
	}

	int newPos = 0;
	for (int i = pos; i >= 0; i--) {
		if (lines[pos].minX == beginFromX) {
			newPos = i;
		}

		break;
	}

	return newPos;
}

bool leftCompare(LineSeg l1, LineSeg l2) {
	return l1.pt1.x < l2.pt1.x;
}

std::vector<LineSeg> getLines5(unsigned char *image, int width, int height) {

	std::vector<LineSeg> lines;

	int offset_pos[] = { 1, width - 1, width, width + 1, };

	PointI offset_coord[] = { PointI(1, 0), PointI(-1, 1), PointI(0, 1), PointI(
			1, 1) };

	int sizeNeighbor = sizeof(offset_pos) / sizeof(int);

	int pos, pos1;
	for (int y = 0; y < height - 1; y++) {
		pos = y * width - 1;
		for (int x = 1; x < width - 1; x++) {
			pos++;
			if (image[pos] == 0) {
				// TODO: 元はNULLと比較
				continue;
			}

			for (int i = 0; i < sizeNeighbor; i++) {
				pos1 = pos + offset_pos[i];
				if (image[pos1] == 0) {
					continue;
				}

				// make line
				LineSeg line = createLine(PointI(x, y),
						PointI(x + offset_coord[i].x, y + offset_coord[i].y));
				lines.push_back(line);
			}
		}
	}

	// sort
	std::sort(lines.begin(), lines.end(), leftCompare);

	// connect the lines
	bool foundConnection;
	Vector2F conDirection;

	while (true) {
		foundConnection = false;
		for (size_t i = 0; i < lines.size(); i++) {
			for (size_t j = i + 1; j < lines.size(); j++) {
				if (lines[j].minX > lines[i].maxX) {
					break;
				}

				if (!lines[i].isBoxIntersect(lines[j])) {
					continue;
				}

				if (!lines[i].isSame2(lines[j])) {
					continue;
				}

				// thuong thi se chi co 1 thang trung voi no thoi
				lines.erase(lines.begin() + j);
				break;
			}
		}

		if (!foundConnection) {
			break;
		}
	}

	return lines;
}

std::vector<LineSeg2> getLines6(unsigned char *image, int width, int height) {
	int pos, pos1;
	int lineGap = 4;
	PointI point;
	std::vector<PointI> points;
	int fromY1, toY1;
	float a_temp, b_temp, error_temp, a, b, error;

	int offset_pos[] = { 0, -1, 1, -2, 2 };
	int len = sizeof(offset_pos) / sizeof(int);
	bool found;

	std::vector<LineSeg2> lines;
	unsigned char *mark = new unsigned char[width * height];
	memset(mark, 0, width * height);

	for (int y = 0; y < height; y++) {
		pos = y * width - 1;
		for (int x = 0; x < width; x++) {
			pos++;
			if (image[pos] == 0) {
				// TODO: 元はNULLと比較
				continue;
			}

			if (mark[pos] == 1) {
				continue;
			}

			point.x = x;
			point.y = y;

			points.clear();
			points.push_back(point);

			while (true) {
				point = points[points.size() - 1];
				fromY1 = point.y + 1;
				toY1 = std::min(fromY1 + lineGap, height);

				found = false;
				for (int y1 = fromY1; y1 < toY1; y1++) {
					for (int i = 0; i < len; i++) {
						int x1 = point.x + offset_pos[i];
						if (x1 < 0 || x1 >= width) {
							continue;
						}

						pos1 = y1 * width + x1;
						if (image[pos1] == 0) {
							continue;
						}

						if (mark[pos1] == 1) {
							continue;
						}

						// found the candidate point
						// get the line
						std::vector<PointI> temp_points = points;
						temp_points.push_back(PointI(x1, y1));
						if (!getLineT(temp_points, g_disFromLine, error_temp,
								a_temp, b_temp)) {
							continue;
						}

						// add good point
						points.push_back(PointI(x1, y1));
						a = a_temp;
						b = b_temp;
						error = error_temp;

						mark[pos1] = 1;

						// break loops;
						y1 = 1000000000;

						found = true;

						break;
					}
				}

				if (!found) {
					// check if the line is good

					// form the line
					if (points.size() >= 2) {
						LineSeg2 line = createLine2(points, a, b, error);
						lines.push_back(line);
					}

					break;
				}
			}
		}
	}

	delete[] mark;

	// sort
	std::sort(lines.begin(), lines.end(), leftCompare);

#	ifdef PANEL_DETECTOR_DEBUG
	printLines3("high_contrast.png", "lines0.png", lines);
	printLines3("high_contrast_t.png", "lines0_t.png", lines);
#	endif

	// connect the lines
	bool foundConnection;
	Vector2F conDirection;
	float minX, minY, maxX, maxY;
	int vertExpand;
	int horzExpand = 6;

	float dis_2 = g_disFromLineRomove * g_disFromLineRomove;

	while (true) {
		foundConnection = false;
		for (size_t i = 0; i < lines.size(); i++) {
			vertExpand = std::max(lines[i].points.size() / 2, (size_t) 10);

			// expand the box
			minX = lines[i].minX - horzExpand;
			maxX = lines[i].maxX + horzExpand;
			minY = lines[i].minY - vertExpand;
			maxY = lines[i].maxY + vertExpand;

			for (size_t j = i + 1; j < lines.size(); j++) {
				if (lines[j].minX - lines[i].maxX > 10) {
					break;
				}

				if (lines[j].maxX < minX || lines[j].minX > maxX) {
					continue;
				}
				if (lines[j].maxY < minY || lines[j].minY > maxY) {
					continue;
				}

				int res = estimateConnection4(&lines[i], &lines[j], error, a,
						b);
				if (error < dis_2) {
					if (res == 0 || res == 1 || res == 2) {
						// connect
						connect2Line2(lines[i], lines[j], a, b, error);

						// delete line j
						lines.erase(lines.begin() + j);
						j--;

						foundConnection = true;
					}
				}
			}
		}

		if (!foundConnection) {
			break;
		}
	}

	// remove weak line
	for (size_t i = 0; i < lines.size(); i++) {
		if (lines[i].length < 4) {
			lines.erase(lines.begin() + i);
			i--;
			continue;
		}

		if (lines[i].density < 0.3f) {
			lines.erase(lines.begin() + i);
			i--;
		}
	}

	return lines;
}

void getMydImage(unsigned char *image, int stride, int width, int height,
		int fromX, int toX, int fromY, int toY, unsigned char *mydBinImage) {
	std::vector<DPoint> hist;
	DPoint point;
	point.x = -1;
	point.y = 0;
	point.lenx = 0;

	int pos;
	int lenf = 4;

	int minLen = 3;

	float diffThreshold = 0.2f;
	float maxDistance = 60;
	float minDistance = 10;

	for (int y = fromY; y < toY; y++) {
		hist.clear();
		point.empty();
		pos = y * stride + fromX;
		for (int x = fromX; x < width - 1; x++) {
			if (image[pos + 1] > image[pos]) {
				if (point.type == 1) {
					point.y += image[pos + 1] - image[pos];
					point.lenx++;

					if (point.lenx < lenf) {
						point.yf = point.y;
						point.lenxf = point.lenx;
					} else {
						if (image[pos + 1] - image[pos - (lenf - 1)]
								> point.yf) {
							point.yf = image[pos + 1] - image[pos - (lenf - 1)];
							point.xf = x - (lenf - 1);
							point.lenxf = lenf;
						}
					}
				} else {
					if (point.type != 0 && point.lenxf >= minLen) {
						hist.push_back(point);
					}

					point.empty();
					point.type = 1;
					point.x = x;
					point.lenx++;
					point.y = image[pos + 1] - image[pos];
				}
			} else if (image[pos + 1] < image[pos]) {
				if (point.type == -1) {
					point.y += image[pos + 1] - image[pos];
					point.lenx++;

					if (point.lenx < lenf) {
						point.yf = point.y;
						point.lenxf = point.lenx;
					} else {
						if (image[pos + 1] - image[pos - (lenf - 1)]
								< point.yf) {
							point.yf = image[pos + 1] - image[pos - (lenf - 1)];
							point.xf = x - (lenf - 1);
							point.lenxf = lenf;
						}
					}
				} else {
					if (point.type != 0 && point.lenxf >= minLen) {
						hist.push_back(point);
					}

					point.empty();
					point.type = -1;
					point.x = x;
					point.lenx++;
					point.y = image[pos + 1] - image[pos];
				}
			} else {
				// ==
				if (point.type != 0 && point.lenxf >= minLen) {
					hist.push_back(point);
				}

				point.empty();
			}

			pos++;
		}

		if (point.type != 0 && point.lenxf >= minLen) {
			hist.push_back(point);
		}

		for (size_t i = 0; i < hist.size(); i++) {
			hist[i].yf /= hist[i].lenxf;
		}

		for (size_t i = 0; i < hist.size(); i++) {
			if (fabs(hist[i].yf) < 2) {
				hist.erase(hist.begin() + i);
				i--;
			}
		}

		// looking for good pair
		std::vector<DPair> pairs;
		float thres = 10.f;
		for (size_t i = 0; i < hist.size() - 1; i++) {
			if (hist[i].type < 0 && hist[i + 1].type > 0
					&& hist[i + 1].x - (hist[i].x + hist[i].lenx) < 2) {
				if (hist[i + 1].getposf() - hist[i].getposf() < 8.f) {
					DPair pair;
					pair.left = hist[i];
					pair.right = hist[i + 1];
					pairs.push_back(pair);
					i++;
				} else {
					if (hist[i].yf > thres) {
						DPair pair;
						pair.left = hist[i];
						pairs.push_back(pair);
					}

					if (hist[i + 1].yf > thres) {
						DPair pair;
						pair.right = hist[i + 1];
						pairs.push_back(pair);

						i++;
					}
				}
			} else if (fabs(hist[i].yf) > thres) {
				// should be kept
				if (hist[i].type == -1) {
					DPair pair;
					pair.left = hist[i];
					pairs.push_back(pair);
				} else if (hist[i].type == 1) {
					DPair pair;
					pair.right = hist[i];
					pairs.push_back(pair);
				}
			}
		}

		// looking for group of good pairs
		float dis;
		std::vector<EquiGroup> groups;

		float diff;
		bool found;
		for (size_t i = 0; i < pairs.size(); i++) {
			for (size_t j = i + 1; j < pairs.size(); j++) {
				// check if they exist somewhere already
				found = false;
				for (size_t k = 0; k < groups.size(); k++) {
					if (areWeHere(i, j, groups[k].members)) {
						found = true;
						break;
					}
				}

				if (found) {
					continue;
				}

				EquiGroup eGroup;
				eGroup.error = 0;
				eGroup.distance = pairs[j].getCenter1() - pairs[i].getCenter1();

				if (eGroup.distance < minDistance
						|| eGroup.distance > maxDistance) {
					continue;
				}

				eGroup.firstPos = pairs[i].getCenter1();
				eGroup.lastPos = pairs[j].getCenter1();
				eGroup.members.push_back(i);
				eGroup.members.push_back(j);

				for (size_t k = j + 1; k < pairs.size(); k++) {
					dis = pairs[k].getCenter1() - eGroup.lastPos;
					if (dis < minDistance || dis > maxDistance) {
						continue;
					}

					diff = diff2ValueSign(dis, eGroup.distance);
					if (fabs(diff) < diffThreshold
							&& k - eGroup.members[eGroup.members.size() - 1] < 3// not many other between them
									) {
						// push to group
						eGroup.lastPos = pairs[k].getCenter1();
						eGroup.distance = (eGroup.distance + dis * 2) / 3;
						eGroup.error = (eGroup.error * eGroup.getNumError()
								+ fabs(diff)) / (eGroup.getNumError() + 1);
						eGroup.members.push_back(k);
					} else if (diff > diffThreshold) {
						break;
					}
				}

				if (eGroup.members.size() > 2) {
					groups.push_back(eGroup);
				}
			}
		}

		// keep the the unique pair only
		std::vector<int> temp_pos;
		for (size_t i = 0; i < groups.size(); i++) {
			for (size_t j = 0; j < groups[i].members.size(); j++) {
				if (std::find(temp_pos.begin(), temp_pos.end(),
						groups[i].members[j]) == temp_pos.end()) {
					temp_pos.push_back(groups[i].members[j]);
				}
			}
		}

		std::vector<DPair> best_pairs;
		for (size_t i = 0; i < temp_pos.size(); i++) {
			best_pairs.push_back(pairs[temp_pos[i]]);
		}

		pos = y * width + fromX;
		for (size_t i = 0; i < best_pairs.size(); i++) {
			int tempPos = pos + (int) (best_pairs[i].getCenter1() + 0.5f);
			mydBinImage[tempPos] = 255;
		}

	}

}

// as above, but simpler
void getMydImage2(unsigned char *image, int stride, int width, int height,
		int fromX, int toX, int fromY, int toY, DPair **mydImage) {
	std::vector<DPoint> hist;
	DPoint point;
	point.x = -1;
	point.y = 0;
	point.lenx = 0;

	int pos;
	int lenf = 4;

	int minLen = 3;

	for (int y = fromY; y < toY; y++) {
		hist.clear();
		pos = y * stride + fromX;
		for (int x = fromX; x < width - 1; x++) {
			if (image[pos + 1] > image[pos]) {
				if (point.type == 1) {
					point.y += image[pos + 1] - image[pos];
					point.lenx++;

					if (point.lenx < lenf) {
						point.yf = point.y;
						point.lenxf = point.lenx;
					} else {
						if (image[pos + 1] - image[pos - (lenf - 1)]
								> point.yf) {
							point.yf = image[pos + 1] - image[pos - (lenf - 1)];
							point.xf = x - (lenf - 1);
							point.lenxf = lenf;
						}
					}
				} else {
					if (point.type != 0 && point.lenxf >= minLen) {
						hist.push_back(point);
					}

					point.empty();
					point.type = 1;
					point.x = x;
					point.lenx++;
					point.y = image[pos + 1] - image[pos];
				}
			} else if (image[pos + 1] < image[pos]) {
				if (point.type == -1) {
					point.y += image[pos + 1] - image[pos];
					point.lenx++;

					if (point.lenx < lenf) {
						point.yf = point.y;
						point.lenxf = point.lenx;
					} else {
						if (image[pos + 1] - image[pos - (lenf - 1)]
								< point.yf) {
							point.yf = image[pos + 1] - image[pos - (lenf - 1)];
							point.xf = x - (lenf - 1);
							point.lenxf = lenf;
						}
					}
				} else {
					if (point.type != 0 && point.lenxf >= minLen) {
						hist.push_back(point);
					}

					point.empty();
					point.type = -1;
					point.x = x;
					point.lenx++;
					point.y = image[pos + 1] - image[pos];
				}
			} else {
				// ==
				if (point.type != 0 && point.lenxf >= minLen) {
					hist.push_back(point);
				}

				point.empty();
			}

			pos++;
		}

		if (point.type != 0 && point.lenxf >= minLen) {
			hist.push_back(point);
		}

		if (hist.size() < 2) {
			continue;
		}

		for (size_t i = 0; i < hist.size(); i++) {
			hist[i].yf /= hist[i].lenxf;
		}

		// looking for good pair
		std::vector<DPair> pairs;
		for (size_t i = 0; i < hist.size() - 1; i++) {
			if (hist[i].type < 0 && hist[i + 1].type > 0
					&& hist[i + 1].x - (hist[i].x + hist[i].lenx) < 2) {
				DPair pair;
				pair.left = hist[i];
				pair.right = hist[i + 1];
				pairs.push_back(pair);
			}
		}

		pos = y * width + fromX;
		for (size_t i = 0; i < pairs.size(); i++) {
			int tempPos = pos + (int) (pairs[i].getCenter() + 0.5f);

			mydImage[tempPos] = (DPair*) malloc(sizeof(DPair));
			mydImage[tempPos]->copy(pairs[i]);
		}

	}
}

// as above, but simpler, for low constrast
void getMydImage3(unsigned char *image, int stride, int width, int height,
		unsigned char *mydImage) {
	std::vector<DPoint> hist;
	DPoint point;
	point.x = -1;
	point.y = 0;
	point.lenx = 0;

	int pos;
	int lenf = 4;

	int minLen = 2;

	for (int y = 0; y < height; y++) {
		hist.clear();
		point.empty();
		pos = y * stride;
		for (int x = 0; x < width - 1; x++) {
			if (image[pos + 1] > image[pos]) {
				if (point.type == 1) {
					point.y += image[pos + 1] - image[pos];
					point.lenx++;

					if (point.lenx < lenf) {
						point.yf = point.y;
						point.lenxf = point.lenx;
					} else {
						if (image[pos + 1] - image[pos - (lenf - 1)]
								> point.yf) {
							point.yf = image[pos + 1] - image[pos - (lenf - 1)];
							point.xf = x - (lenf - 1);
							point.lenxf = lenf;
						}
					}
				} else {
					if (point.type != 0 && point.lenxf >= minLen) {
						hist.push_back(point);
					}

					point.empty();
					point.type = 1;
					point.x = x;
					point.lenx++;
					point.y = image[pos + 1] - image[pos];
				}
			} else if (image[pos + 1] < image[pos]) {
				if (point.type == -1) {
					point.y += image[pos + 1] - image[pos];
					point.lenx++;

					if (point.lenx < lenf) {
						point.yf = point.y;
						point.lenxf = point.lenx;
					} else {
						if (image[pos + 1] - image[pos - (lenf - 1)]
								< point.yf) {
							point.yf = image[pos + 1] - image[pos - (lenf - 1)];
							point.xf = x - (lenf - 1);
							point.lenxf = lenf;
						}
					}
				} else {
					if (point.type != 0 && point.lenxf >= minLen) {
						hist.push_back(point);
					}

					point.empty();
					point.type = -1;
					point.x = x;
					point.lenx++;
					point.y = image[pos + 1] - image[pos];
				}
			} else {
				// ==

				// keep track
				if (point.type != 0) {
					point.lenx++;
				}
			}

			pos++;
		}

		if (point.type != 0 && point.lenxf >= minLen) {
			hist.push_back(point);
		}

		for (size_t i = 0; i < hist.size(); i++) {
			hist[i].yf /= hist[i].lenxf;
		}

		for (size_t i = 0; i < hist.size(); i++) {
			if (fabs(hist[i].yf) < 0.8f) {
				hist.erase(hist.begin() + i);
				i--;
			}
		}

		if (hist.size() < 2) {
			continue;
		}

		// --------------looking for good pair
		std::vector<DPair> pairs;
		float diffPercent, diffValue;
		for (size_t i = 0; i < hist.size() - 1; i++) {
			if (hist[i].type < 0 && hist[i + 1].type > 0
					&& hist[i + 1].x - (hist[i].x + hist[i].lenx) < 2) {
				int keep = 0;
				diffPercent = diff2ValueMax(fabs(hist[i].yf),
						fabs(hist[i + 1].yf));
				if (diffPercent < 0.5f) {
					keep = 1;
				} else {
					diffValue = fabs(hist[i].yf + hist[i + 1].yf);
					if (diffValue < 2.5f) {
						keep = 1;
					} else {
						keep = 2;
					}
				}

				if (keep == 1) {
					DPair pair;
					pair.left = hist[i];
					pair.right = hist[i + 1];

					pairs.push_back(pair);
				} else if (keep == 2) {
					DPair pair;
					if (fabs(hist[i].yf) > hist[i + 1].yf) {
						pair.left = hist[i];
					} else {
						pair.right = hist[i + 1];
					}

					pairs.push_back(pair);
				}
			}
		}

		// if found only 1 pair
		if (pairs.size() == 1) {
			// try to look for the othe candidate
			int best = -1;
			float bestScore = 0;
			for (size_t i = 0; i < hist.size(); i++) {
				if (hist[i].x < pairs[0].left.x) {
					if (hist[i].type != 1) {
						continue;
					}

					if (hist[i].yf > bestScore) {
						bestScore = hist[i].yf;
						best = i;
					}
				} else if (hist[i].x > pairs[0].right.x) {
					if (hist[i].type != -1) {
						continue;
					}

					if (-hist[i].yf > bestScore) {
						bestScore = -hist[i].yf;
						best = i;
					}
				}
			}

			if (best != -1) {
				DPair pair;
				if (hist[best].x < pairs[0].left.x) {
					pair.right = hist[best];
				} else {
					pair.left = hist[best];
				}

				pairs.push_back(pair);
			}
		}

		pos = y * width;
		for (size_t i = 0; i < pairs.size(); i++) {
			int tempPos = pos + (int) (pairs[i].getCenter1() + 0.5f);
			mydImage[tempPos] = 255;
		}

	}
}

// as above, for higher constrast than above
void getMydImage4(unsigned char *image, int stride, int width, int height,
		unsigned char *mydImage, float minDis) {
	std::vector<DPoint> hist;
	DPoint point;
	point.x = -1;
	point.y = 0;
	point.lenx = 0;

	int pos;
	int lenf = 4;

	int minLen = 2;

	for (int y = 0; y < height; y++) {
		hist.clear();
		point.empty();
		pos = y * stride;
		for (int x = 0; x < width - 1; x++) {
			if (image[pos + 1] > image[pos]) {
				if (point.type == 1) {
					point.y += image[pos + 1] - image[pos];
					point.lenx++;

					if (point.lenx < lenf) {
						point.yf = point.y;
						point.lenxf = point.lenx;
					} else {
						if (image[pos + 1] - image[pos - (lenf - 1)]
								> point.yf) {
							point.yf = image[pos + 1] - image[pos - (lenf - 1)];
							point.xf = x - (lenf - 1);
							point.lenxf = lenf;
						}
					}
				} else {
					if (point.type != 0 && point.lenxf >= minLen) {
						hist.push_back(point);
					}

					point.empty();
					point.type = 1;
					point.x = x;
					point.lenx++;

					point.xf = x;

					point.y = image[pos + 1] - image[pos];
				}
			} else if (image[pos + 1] < image[pos]) {
				if (point.type == -1) {
					point.y += image[pos + 1] - image[pos];
					point.lenx++;

					if (point.lenx < lenf) {
						point.yf = point.y;
						point.lenxf = point.lenx;
					} else {
						if (image[pos + 1] - image[pos - (lenf - 1)]
								< point.yf) {
							point.yf = image[pos + 1] - image[pos - (lenf - 1)];
							point.xf = x - (lenf - 1);
							point.lenxf = lenf;
						}
					}
				} else {
					if (point.type != 0 && point.lenxf >= minLen) {
						hist.push_back(point);
					}

					point.empty();
					point.type = -1;
					point.x = x;
					point.lenx++;
					point.y = image[pos + 1] - image[pos];
				}
			} else {
				// ==
				if (point.type != 0 && point.lenxf >= minLen) {
					hist.push_back(point);
				}

				point.empty();
			}

			pos++;
		}

		if (point.type != 0 && point.lenxf >= minLen) {
			hist.push_back(point);
		}

		for (size_t i = 0; i < hist.size(); i++) {
			hist[i].yf /= hist[i].lenxf;
		}

		float weakThr = 3.f;
		for (size_t i = 0; i < hist.size(); i++) {
			if (fabs(hist[i].yf) < weakThr) {
				hist.erase(hist.begin() + i);
				i--;
			}
		}

		if (hist.size() < 2) {
			continue;
		}

		std::vector<DPair> pairs;
		float thres = 10.f;
		for (size_t i = 0; i < hist.size() - 1; i++) {
			if (hist[i].type < 0 && hist[i + 1].type > 0
					&& hist[i + 1].x - (hist[i].x + hist[i].lenx) < 2) {

				DPair pair;
				pair.left = hist[i];
				pair.right = hist[i + 1];
				pairs.push_back(pair);

				i++;
			} else if (fabs(hist[i].yf) > thres) {
				// should be kept
				if (hist[i].type == -1) {
					DPair pair;
					pair.left = hist[i];
					pairs.push_back(pair);
				} else if (hist[i].type == 1) {
					DPair pair;
					pair.right = hist[i];
					pairs.push_back(pair);
				}
			}
		}

		// if found only 1 pair
		if (pairs.size() == 1) {
			// try to look for the other candidate
			int best = -1;
			int where = -1;
			float bestScore = 0;
			for (size_t i = 0; i < hist.size(); i++) {
				if (hist[i].x < pairs[0].getValidLeft()) {
					if (hist[i].type != 1) {
						continue;
					}

					if (hist[i].yf > bestScore) {
						bestScore = hist[i].yf;
						best = i;
						where = 0;
					}
				} else if (hist[i].x > pairs[0].getValidRight()) {
					if (hist[i].type != -1) {
						continue;
					}

					if (-hist[i].yf > bestScore) {
						bestScore = -hist[i].yf;
						best = i;
						where = 1;
					}
				}
			}

			if (best != -1) {
				DPair pair;
				if (hist[best].x < pairs[0].left.x) {
					pair.right = hist[best];
				} else {
					pair.left = hist[best];
				}

				if (where == 0) {
					pairs.insert(pairs.begin(), pair);
				} else {
					pairs.push_back(pair);
				}
			}
		} else if (pairs.size() == 0) {
			// try to look for best pair
			int best_i = -1, best_j = -1;
			float bestScore = 0, score;
			for (size_t i = 0; i < hist.size(); i++) {
				if (hist[i].type != 1) {
					continue;
				}

				for (size_t j = i + 1; j < hist.size(); j++) {
					if (hist[j].type != -1) {
						continue;
					}

					score = hist[i].yf - hist[j].yf;

					if (score > bestScore) {
						bestScore = score;
						best_i = i;
						best_j = j;
					}
				}
			}

			if (best_i != -1) {
				DPair pairL;
				pairL.right = hist[best_i];
				pairs.push_back(pairL);

				DPair pairR;
				pairR.left = hist[best_j];
				pairs.push_back(pairR);
			}
		}

		// --------------looking best 2 pairs
		if (pairs.size() > 2) {
			int best_i = -1, best_j;
			float score, bestScore = 0;
			for (size_t i = 0; i < pairs.size(); i++) {
				if (pairs[i].right.type == 0) {
					continue;
				}

				score = pairs[i].getYf();
				for (size_t j = i + 1; j < pairs.size(); j++) {
					if (pairs[j].left.type == 0) {
						continue;
					}

					if (pairs[j].getCenterLeft2() - pairs[i].getCenterRight2()
							< minDis) {
						continue;
					}

					if (score + pairs[j].getYf() > bestScore) {
						bestScore = score + pairs[j].getYf();
						best_i = i;
						best_j = j;

					}
				}
			}

			if (best_i != -1 && pairs[best_i].right.type != 0
					&& pairs[best_j].left.type != 0) {
				pos = y * width;
				int tempPos = pos
						+ (int) (pairs[best_i].getCenterRight2() + 0.5f);
				mydImage[tempPos] = 255;

				tempPos = pos + (int) (pairs[best_j].getCenterLeft2() + 0.5f);
				mydImage[tempPos] = 255;
			}
		} else if (pairs.size() == 2) {
			// just keep
			pos = y * width;
			int tempPos;

			if (pairs[0].isGoodPair()) {
				tempPos = pos + (int) (pairs[0].getCenterRight2() + 0.5f);
				mydImage[tempPos] = 255;
			} else {
				if (pairs[0].left.type != 0) {
					tempPos = pos + (int) (pairs[0].left.getposf() + 0.5f);
					mydImage[tempPos] = 255;
				} else if (pairs[0].right.type != 0) {
					tempPos = pos + (int) (pairs[0].right.getposf() + 0.5f);
					mydImage[tempPos] = 255;
				}
			}

			if (pairs[1].isGoodPair()) {
				tempPos = pos + (int) (pairs[1].getCenterLeft2() + 0.5f);
				mydImage[tempPos] = 255;
			} else {
				if (pairs[1].left.type != 0) {
					tempPos = pos + (int) (pairs[1].left.getposf() + 0.5f);
					mydImage[tempPos] = 255;
				} else if (pairs[1].right.type != 0) {
					tempPos = pos + (int) (pairs[1].right.getposf() + 0.5f);
					mydImage[tempPos] = 255;
				}
			}
		} else if (pairs.size() == 1) {
			float bestScore = 0;
			int best = -1;
			for (size_t i = 0; i < hist.size(); i++) {
				if (hist[i].x == pairs[0].left.x
						|| hist[i].x == pairs[0].right.x) {
					continue;
				}

				if (bestScore < fabs(hist[i].yf)) {
					bestScore = fabs(hist[i].yf);
					best = i;
				}
			}

			if (best != -1) {
				pos = y * width;
				int tempPos;
				if (pairs[0].getCenter() < hist[best].getposf()) {
					tempPos = pos + (int) (pairs[0].getCenterRight2() + 0.5f);
					mydImage[tempPos] = 255;
				} else {
					tempPos = pos + (int) (pairs[0].getCenterLeft2() + 0.5f);
					mydImage[tempPos] = 255;
				}

				tempPos = pos + (int) (hist[best].getposf() + 0.5f);
				mydImage[tempPos] = 255;
			}
		}
	}
}

void getMydImage5(unsigned char *image, int stride, int width, int height,
		unsigned char *mydBinImage) {
	std::vector<DPoint> hist;
	DPoint point;
	point.x = -1;
	point.y = 0;
	point.lenx = 0;

	int pos;
	int lenf = 4;

	int minLen = 3;

	for (int y = 0; y < height; y++) {
		hist.clear();
		point.empty();
		pos = y * stride;
		for (int x = 0; x < width - 1; x++) {
			if (image[pos + 1] > image[pos]) {
				if (point.type == 1) {
					point.y += image[pos + 1] - image[pos];
					point.lenx++;

					if (point.lenx < lenf) {
						point.yf = point.y;
						point.lenxf = point.lenx;
					} else {
						if (image[pos + 1] - image[pos - (lenf - 1)]
								> point.yf) {
							point.yf = image[pos + 1] - image[pos - (lenf - 1)];
							point.xf = x - (lenf - 1);
							point.lenxf = lenf;
						}
					}
				} else {
					if (point.type != 0 && point.lenxf >= minLen) {
						hist.push_back(point);
					}

					point.empty();
					point.type = 1;
					point.x = x;
					point.lenx++;
					point.y = image[pos + 1] - image[pos];
				}
			} else if (image[pos + 1] < image[pos]) {
				if (point.type == -1) {
					point.y += image[pos + 1] - image[pos];
					point.lenx++;

					if (point.lenx < lenf) {
						point.yf = point.y;
						point.lenxf = point.lenx;
					} else {
						if (image[pos + 1] - image[pos - (lenf - 1)]
								< point.yf) {
							point.yf = image[pos + 1] - image[pos - (lenf - 1)];
							point.xf = x - (lenf - 1);
							point.lenxf = lenf;
						}
					}
				} else {
					if (point.type != 0 && point.lenxf >= minLen) {
						hist.push_back(point);
					}

					point.empty();
					point.type = -1;
					point.x = x;
					point.lenx++;
					point.y = image[pos + 1] - image[pos];
				}
			} else {
				// ==
				if (point.type != 0 && point.lenxf >= minLen)
					hist.push_back(point);

				point.empty();
			}

			pos++;
		}

		if (point.type != 0 && point.lenxf >= minLen) {
			hist.push_back(point);
		}

		for (size_t i = 0; i < hist.size(); i++) {
			hist[i].yf /= hist[i].lenxf;
		}

		for (size_t i = 0; i < hist.size(); i++) {
			if (fabs(hist[i].yf) < 2) {
				hist.erase(hist.begin() + i);
				i--;
			}
		}

		// fix crash bug
		if (hist.size() < 2) {
			continue;
		}

		// looking for good pair
		std::vector<DPair> pairs;
		float thres = 5.f;
		for (size_t i = 0; i < hist.size() - 1; i++) {
			if (hist[i].type < 0 && hist[i + 1].type > 0
					&& hist[i + 1].x - (hist[i].x + hist[i].lenx) < 2) {
				if (hist[i + 1].getposf() - hist[i].getposf() < 8.f) {
					DPair pair;
					pair.left = hist[i];
					pair.right = hist[i + 1];
					pairs.push_back(pair);
					i++;
				} else {
					DPair pair1;
					pair1.left = hist[i];
					pairs.push_back(pair1);

					DPair pair2;
					pair2.right = hist[i + 1];
					pairs.push_back(pair2);

					i++;
				}
			} else if (fabs(hist[i].yf) > thres) {
				// should be kept
				if (hist[i].type == -1) {
					DPair pair;
					pair.left = hist[i];
					pairs.push_back(pair);
				} else if (hist[i].type == 1) {
					DPair pair;
					pair.right = hist[i];
					pairs.push_back(pair);
				}
			}
		}

		pos = y * width;
		for (size_t i = 0; i < pairs.size(); i++) {
			int tempPos = pos + (int) (pairs[i].getCenter1() + 0.5f);
			mydBinImage[tempPos] = 255;

		}

	}

}

float overlapLen(LineSeg line1, LineSeg line2) {
	return 0;
}

void keepBest2(std::vector<LineSeg> &lines, int wantedLength) {
	if (lines.size() < 3) {
		return;
	}

	std::vector<float> scores;
	float score, maxScore = 0;

	int best_i = 0, best_j = 0;

	for (size_t i = 0; i < lines.size(); i++) {
		for (size_t j = i + 1; j < lines.size(); j++) {
			float angle = angle2Line(lines[i].direction, lines[j].direction);
			if (angle > M_PI_2) {
				angle = M_PI - angle;
			}

			score = fabs(1 - angle / M_PI_2) * 0.7f
					+ ((lines[i].length + lines[j].length) / (2 * wantedLength))
							* 0.3f;

			if (maxScore < score) {
				maxScore = score;
				best_i = i;
				best_j = j;
			}
		}
	}

	std::vector<LineSeg> tmp_lines;
	tmp_lines.push_back(lines[best_i]);
	tmp_lines.push_back(lines[best_j]);
	lines = tmp_lines;
}

float diffWith90(Vector2F v1, Vector2F v2) {
	float angle = RADIANS_TO_DEGREES(angle2Line(v1, v2));
	return fabs(angle - 90);
}

bool checkCell(unsigned char *image, int stride, int width, int height,
		RectF &rect) {
	int expand = std::max(
			(int) (distance(rect.x1, rect.y1, rect.x3, rect.y3) / 3),
			(int) (distance(rect.x2, rect.y2, rect.x4, rect.y4) / 3));

	int minX = std::max(0, (int) std::min(rect.x1, rect.x4) - expand);
	int maxX = std::min(width, (int) std::max(rect.x2, rect.x3) + expand);

	expand = (int) (expand * 2.f / 3);
	int minY = std::max(0, (int) std::min(rect.y1, rect.y2) - expand);
	int maxY = std::min(height, (int) std::max(rect.y3, rect.y4) + expand);

	int w = maxX - minX;
	int h = maxY - minY;
	unsigned char *simage = (unsigned char*) malloc(w * h);
	getCrop(image, stride, width, height, minX, minY, w, h, simage);

#	ifdef PANEL_DETECTOR_DEBUG
	cv::Mat binMat;
	if (cvMatFromBinary(simage, w, h, binMat)) {
		imwrite("cut_binary.png", binMat);
	}
#	endif

	unsigned char *mydImage = (unsigned char*) malloc(w * h);
	memset(mydImage, 0, w * h);

	getMydImage4(simage, w, w, h, mydImage, (w - 2 * expand) / 2);

#	ifdef PANEL_DETECTOR_DEBUG
	printMyd3("cut_binary.png", "cut_binary_ed.png", mydImage, w, h);
#	endif

	std::vector<LineSeg> vertLines = getLines3(mydImage, w, h);

#	ifdef PANEL_DETECTOR_DEBUG
	printLines("cut_binary.png", "cut_binary_line.png", vertLines);
#	endif

	// -------------------------
	unsigned char *simage_t = (unsigned char*) malloc(w * h);
	getTranspose(simage, w, h, simage_t);

#	ifdef PANEL_DETECTOR_DEBUG
	cv::Mat binMat_t;
	if (cvMatFromBinary(simage_t, h, w, binMat_t)) {
		imwrite("cut_binary_t.png", binMat_t);
	}
#	endif

	unsigned char *mydImage_t = (unsigned char*) malloc(w * h);
	memset(mydImage_t, 0, w * h);

	getMydImage4(simage_t, h, h, w, mydImage_t, (h - 2 * expand) / 2);

#	ifdef PANEL_DETECTOR_DEBUG
	printMyd3("cut_binary_t.png", "cut_binary_t_ed.png", mydImage_t, h, w);
#	endif

	std::vector<LineSeg> horzLines = getLines3(mydImage_t, h, w);

#	ifdef PANEL_DETECTOR_DEBUG
	printLines("cut_binary_t.png", "cut_binary_t_line.png", horzLines);
#	endif

	free(simage);
	free(simage_t);

	free(mydImage);
	free(mydImage_t);

	if (vertLines.size() > 2) {
		keepBest2(vertLines, h);
	}

	if (horzLines.size() > 2) {
		keepBest2(horzLines, w);
	}

	// should be enough lines
	if (vertLines.size() != 2 || horzLines.size() != 2) {
		return false;
	}

	// should parallel
	float threshold = 0.9848f;	// 10 degree
	float dot = Vector2F::dot(vertLines[0].direction, vertLines[1].direction);
	if (dot < threshold) {
		return false;
	}

	dot = Vector2F::dot(horzLines[0].direction, horzLines[1].direction);
	if (dot < threshold) {
		return false;
	}

	// transpose horz line
	for (size_t i = 0; i < horzLines.size(); i++) {
		horzLines[i].transpose();
	}

	// vert and horz should perpendicular
	threshold = 0.2588f;	// 75 degree
	for (size_t i = 0; i < vertLines.size(); i++) {
		for (size_t j = 0; j < horzLines.size(); j++) {
			dot = Vector2F::dot(vertLines[i].direction, horzLines[j].direction);
			if (dot > threshold) {
				return false;
			}
		}
	}

	// angle sum error should be small enough
	float angleError = diffWith90(vertLines[0].direction,
			horzLines[0].direction)
			+ diffWith90(vertLines[0].direction, horzLines[1].direction)
			+ diffWith90(vertLines[1].direction, horzLines[0].direction)
			+ diffWith90(vertLines[1].direction, horzLines[1].direction);

	if (angleError > 40) {
		return false;
	}

	// find intersection
	float x1, y1, x2, y2, x3, y3, x4, y4;
	float error = twoLineIntersection(vertLines[0], horzLines[0], x1, y1);
	if (error == FLT_MAX) {
		return false;
	}

	error += twoLineIntersection(vertLines[0], horzLines[1], x4, y4);
	if (error == FLT_MAX) {
		return false;
	}

	error += twoLineIntersection(vertLines[1], horzLines[0], x2, y2);
	if (error == FLT_MAX) {
		return false;
	}

	error += twoLineIntersection(vertLines[1], horzLines[1], x3, y3);
	if (error == FLT_MAX) {
		return false;
	}

	float thr = distance(rect.x1, rect.y1, rect.x2, rect.y2) * 2 / 3;
	if (error > thr) {
		return false;
	}

	// update
	rect.x1 = minX + x1;
	rect.y1 = minY + y1;
	rect.x2 = minX + x2;
	rect.y2 = minY + y2;
	rect.x3 = minX + x3;
	rect.y3 = minY + y3;
	rect.x4 = minX + x4;
	rect.y4 = minY + y4;

	return true;
}

bool checkCell2(unsigned char *image, int stride, int width, int height,
		RectF rect, float aveW, float aveH, Rect3 &rectOut) {
	int minX = rect.x1;
	int maxX = std::min((int) rect.x2, width);
	int minY = rect.y1;
	int maxY = std::min((int) rect.y4, height);

	int w = maxX - minX;
	int h = maxY - minY;
	unsigned char *simage = (unsigned char*) malloc(w * h);
	getCrop(image, stride, width, height, minX, minY, w, h, simage);

#	ifdef PANEL_DETECTOR_DEBUG
	cv::Mat binMat;
	if (cvMatFromBinary(simage, w, h, binMat)) {
		imwrite("cut_binary.png", binMat);
	}
#	endif

	unsigned char *mydImage = (unsigned char*) malloc(w * h);
	memset(mydImage, 0, w * h);

	float minDis = 10;

	getMydImage4(simage, w, w, h, mydImage, minDis);

#	ifdef PANEL_DETECTOR_DEBUG
	printMyd3("cut_binary.png", "cut_binary_ed.png", mydImage, w, h);
#	endif

	std::vector<LineSeg> vertLines = getLines3(mydImage, w, h);

#	ifdef PANEL_DETECTOR_DEBUG
	printLines("cut_binary.png", "cut_binary_line.png", vertLines);
#	endif

	// -------------------------
	unsigned char *simage_t = (unsigned char*) malloc(w * h);
	getTranspose(simage, w, h, simage_t);

#	ifdef PANEL_DETECTOR_DEBUG
	cv::Mat binMat_t;
	if (cvMatFromBinary(simage_t, h, w, binMat_t)) {
		imwrite("cut_binary_t.png", binMat_t);
	}
#	endif

	unsigned char *mydImage_t = (unsigned char*) malloc(w * h);
	memset(mydImage_t, 0, w * h);

	getMydImage4(simage_t, h, h, w, mydImage_t, minDis);

#	ifdef PANEL_DETECTOR_DEBUG
	printMyd3("cut_binary_t.png", "cut_binary_t_ed.png", mydImage_t, h, w);
#	endif

	std::vector<LineSeg> horzLines = getLines3(mydImage_t, h, w);

#	ifdef PANEL_DETECTOR_DEBUG
	printLines("cut_binary_t.png", "cut_binary_t_line.png", horzLines);
#	endif

	free(simage);
	free(simage_t);
	free(mydImage);
	free(mydImage_t);

	if (vertLines.size() > 2) {
		keepBest2(vertLines, h);
	}

	if (horzLines.size() > 2) {
		keepBest2(horzLines, w);
	}

	// should be enough lines
	if (vertLines.size() != 2 || horzLines.size() != 2) {
		return false;
	}

	// should parallel
	float threshold = 0.9848f;	// 10 degree
	float dot = Vector2F::dot(vertLines[0].direction, vertLines[1].direction);
	if (dot < threshold) {
		return false;
	}

	dot = Vector2F::dot(horzLines[0].direction, horzLines[1].direction);
	if (dot < threshold) {
		return false;
	}

	// transpose horz line
	for (size_t i = 0; i < horzLines.size(); i++) {
		horzLines[i].transpose();
	}

	// vert and horz should perpendicular
	threshold = 0.2588f;	// 75 degree
	for (size_t i = 0; i < vertLines.size(); i++) {
		for (size_t j = 0; j < horzLines.size(); j++) {
			dot = Vector2F::dot(vertLines[i].direction, horzLines[j].direction);
			if (dot > threshold) {
				return false;
			}
		}
	}

	// angle sum error should be small enough
	float angleError = diffWith90(vertLines[0].direction,
			horzLines[0].direction)
			+ diffWith90(vertLines[0].direction, horzLines[1].direction)
			+ diffWith90(vertLines[1].direction, horzLines[0].direction)
			+ diffWith90(vertLines[1].direction, horzLines[1].direction);

	// TRACE("\nAngle Error = %f", angleError);
	if (angleError > 45) {
		return false;
	}

	// find intersection
	float x1, y1, x2, y2, x3, y3, x4, y4;
	float error = twoLineIntersection(vertLines[0], horzLines[0], x1, y1);
	if (error == FLT_MAX) {
		return false;
	}

	error += twoLineIntersection(vertLines[0], horzLines[1], x4, y4);
	if (error == FLT_MAX) {
		return false;
	}

	error += twoLineIntersection(vertLines[1], horzLines[0], x2, y2);
	if (error == FLT_MAX) {
		return false;
	}

	error += twoLineIntersection(vertLines[1], horzLines[1], x3, y3);
	if (error == FLT_MAX) {
		return false;
	}

	float thr;
	if (aveW == 0) {
		thr = distance(rect.x1, rect.y1, rect.x2, rect.y2) / 2;
	} else {
		thr = std::min(aveW, aveH) * 2 / 3;
	}

	if (error > thr) {
		return false;
	}

	int closeToMargin = 1;
	if (x1 < closeToMargin || x1 > w - closeToMargin) {
		return false;
	}
	if (x2 < closeToMargin || x2 > w - closeToMargin) {
		return false;
	}
	if (x3 < closeToMargin || x3 > w - closeToMargin) {
		return false;
	}
	if (x4 < closeToMargin || x4 > w - closeToMargin) {
		return false;
	}

	if (y1 < closeToMargin || y1 > h - closeToMargin) {
		return false;
	}
	if (y2 < closeToMargin || y2 > h - closeToMargin) {
		return false;
	}
	if (y3 < closeToMargin || y3 > h - closeToMargin) {
		return false;
	}
	if (y4 < closeToMargin || y4 > h - closeToMargin) {
		return false;
	}

	// update
	rectOut.x1 = minX + x1;
	rectOut.y1 = minY + y1;
	rectOut.x2 = minX + x2;
	rectOut.y2 = minY + y2;
	rectOut.x3 = minX + x3;
	rectOut.y3 = minY + y3;
	rectOut.x4 = minX + x4;
	rectOut.y4 = minY + y4;

	rectOut.parallelError = angle2Line(vertLines[0].direction,
			vertLines[1].direction)
			+ angle2Line(horzLines[0].direction, horzLines[1].direction);

	return true;
}

void validateCells(unsigned char *image, int stride, int width, int height,
		std::vector<RectF> &rects) {

	if (gValidateCell == 0) {
		return;
	}

	for (size_t i = 0; i < rects.size(); i++) {
		if (!checkCell(image, stride, width, height, rects[i])) {
			rects.erase(rects.begin() + i);

			i--;
		}
	}

	// remove abnormal rect by ratio
	float aveW = 0;
	float aveH = 0;

	float h = 0;
	float w = 0;
	float ratio;
	int count = 0;
	std::vector<float> widths;
	std::vector<float> heights;
	for (size_t i = 0; i < rects.size(); i++) {
		w = (distance(rects[i].x1, rects[i].y1, rects[i].x2, rects[i].y2)
				+ distance(rects[i].x3, rects[i].y3, rects[i].x4, rects[i].y4))
				/ 2;

		h = (distance(rects[i].x1, rects[i].y1, rects[i].x4, rects[i].y4)
				+ distance(rects[i].x2, rects[i].y2, rects[i].x3, rects[i].y3))
				/ 2;

		if (w > h) {
			ratio = w / h;
		} else {
			ratio = h / w;
		}

		if (fabs(ratio - gCellRatio) > gCellRatioThres) {
			rects.erase(rects.begin() + i);
			i--;
			continue;
		}

		widths.push_back(w);
		heights.push_back(h);

		aveW += w;
		aveH += h;

		count++;
	}

	aveW /= count;
	aveH /= count;

	// remove abnormal rect by size
	float thres = 0.25f;
	for (size_t i = 0; i < rects.size(); i++) {
		if (diff2ValueMax(aveW, widths[i]) > thres
				|| diff2ValueMax(aveH, heights[i]) > thres) {
			rects.erase(rects.begin() + i);
			widths.erase(widths.begin() + i);
			heights.erase(heights.begin() + i);
			i--;
		}
	}
}

void solarCanny(unsigned char *image, int stride, int width, int height,
		std::vector<cv::Vec4f> &linesV, std::vector<cv::Vec4f> &linesH) {
	unsigned char *mydBinImage = (unsigned char*) malloc(width * height);
	memset(mydBinImage, 0, width * height);
	getMydImage5(image, stride, width, height, mydBinImage);

#	ifdef PANEL_DETECTOR_DEBUG
	cv::Mat binMat;
	cvMatFromBinary(mydBinImage, width, height, binMat);
	imwrite("solar_canny.png", binMat);
#	endif

	std::vector<LineSeg2> lines = getLines6(mydBinImage, width, height);

	cv::Vec4f l;
	for (size_t i = 0; i < lines.size(); i++) {
		l[0] = lines[i].pt1.x;
		l[1] = lines[i].pt1.y;
		l[2] = lines[i].pt2.x;
		l[3] = lines[i].pt2.y;
		linesV.push_back(l);
	}

	// test
#	ifdef PANEL_DETECTOR_DEBUG
	printLines3("high_contrast.png", "lines.png", lines);
#	endif

	// -------------------------
	unsigned char *image_t = (unsigned char*) malloc(width * height);
	getTranspose(image, width, height, image_t);

#	ifdef PANEL_DETECTOR_DEBUG
	cv::Mat binMat_t;
	if (cvMatFromBinary(image_t, height, width, binMat_t)) {
		imwrite("high_contrast_t.png", binMat_t);
	}
#	endif

	unsigned char *mydBinImage_t = (unsigned char*) malloc(width * height);
	memset(mydBinImage_t, 0, width * height);
	getMydImage5(image_t, height, height, width, mydBinImage_t);

#	ifdef PANEL_DETECTOR_DEBUG
	cv::Mat binMat;
	cvMatFromBinary(mydBinImage_t, height, width, binMat);
	imwrite("solar_canny_t.png", binMat);
#	endif

	std::vector<LineSeg2> lines_t = getLines6(mydBinImage_t, height, width);

	for (size_t i = 0; i < lines_t.size(); i++) {
		l[0] = lines_t[i].pt1.y;
		l[1] = lines_t[i].pt1.x;
		l[2] = lines_t[i].pt2.y;
		l[3] = lines_t[i].pt2.x;
		linesH.push_back(l);
	}

	// test
#	ifdef PANEL_DETECTOR_DEBUG
	printLines3("high_contrast_t.png", "lines_t.png", lines_t);
#	endif

	free(mydBinImage);
}

struct rough_line {
	cv::Vec4f coord;
	Vector2F dir;

	float minX, maxX;
	float minY, maxY;

	rough_line(cv::Vec4f c) {
		coord = c;
		minX = std::min(c[0], c[2]);
		maxX = std::max(c[0], c[2]);
		minY = std::min(c[1], c[3]);
		maxY = std::max(c[1], c[3]);

		dir = Vector2F::normalize(Vector2F(c[2] - c[0], c[3] - c[1]));
	}
};

rough_line convert2Rough(cv::Vec4f l) {
	return rough_line(l);
}

bool leftCompare2(rough_line l1, rough_line l2) {
	return l1.coord[0] < l2.coord[0];
}

bool topCompare2(rough_line l1, rough_line l2) {
	return l1.coord[1] < l2.coord[1];
}

void saveCombineLines(std::vector<rough_line> vLines,
		std::vector<rough_line> hLines, std::string path) {
	// test
	cv::Mat image = cv::imread("high_contrast.png");

	for (size_t i = 0; i < vLines.size(); i++) {
		cv::line(image, cv::Point(vLines[i].coord[0], vLines[i].coord[1]),
				cv::Point(vLines[i].coord[2], vLines[i].coord[3]),
				cv::Scalar(0, 255, 255), 1);
	}

	for (size_t i = 0; i < hLines.size(); i++) {
		cv::line(image, cv::Point(hLines[i].coord[0], hLines[i].coord[1]),
				cv::Point(hLines[i].coord[2], hLines[i].coord[3]),
				cv::Scalar(0, 0, 255), 1);
	}

	imwrite(path, image);
}

int TwoLineIntersection(rough_line l1, rough_line l2, float &xIntersect,
		float &yIntersect) {
	return twoLineIntersection(l1.coord[0], l1.coord[1], l1.coord[2],
			l1.coord[3], l2.coord[0], l2.coord[1], l2.coord[2], l2.coord[3],
			xIntersect, yIntersect);
}

struct intersec_point {
	int x, y;
	Vector2F vDir;
	Vector2F hDir;
	int vIndex;
	int hIndex;

	intersec_point() {
		memset(this, 0, sizeof(intersec_point));
	}
};

void saveIntersection(intersec_point **image, int width, int height,
		std::string path) {
	// test
	cv::Mat cvImage = cv::imread("combine_lines.png");

	int pos;
	for (int y = 0; y < height; y++) {
		pos = y * width;
		for (int x = 0; x < width; x++) {
			if (image[pos]) {
				cv::circle(cvImage, cv::Point(x, y), 1, cv::Scalar(255, 0, 0));
			}

			pos++;
		}
	}

	imwrite(path, cvImage);
}

void extendLine(rough_line &line, int len) {
	Vector2F newP2 = Vector2F(line.coord[2], line.coord[3]) + line.dir * len;
	Vector2F newP1 = Vector2F(line.coord[0], line.coord[1]) - line.dir * len;

	line.coord[0] = newP1.x;
	line.coord[1] = newP1.y;
	line.coord[2] = newP2.x;
	line.coord[3] = newP2.y;
}

bool isOnLineH(const intersec_point &p1, const intersec_point &p2,
		float maxDot) {
	Vector2F v = Vector2F::normalize(Vector2F(p2.x - p1.x, p2.y - p1.y));
	float dot = Vector2F::dot(p1.hDir, v);

	return fabs(dot) > maxDot;
}

bool isOnLineV(const intersec_point &p1, const intersec_point &p2,
		float maxDot) {
	Vector2F v = Vector2F::normalize(Vector2F(p2.x - p1.x, p2.y - p1.y));
	float dot = Vector2F::dot(p1.vDir, v);

	return fabs(dot) > maxDot;
}

int lookRight(const intersec_point &me,
		const std::vector<intersec_point> &neighbors,
		const std::vector<int> dontCares, float maxDot) {
	int pos = -1;
	bool isInDontcare;
	int minDis2 = gMinLen * gMinLen;

	for (size_t i = 0; i < neighbors.size(); i++) {
		if (neighbors[i].x < me.x) {
			continue;
		}

		if (neighbors[i].x == me.x && neighbors[i].y == me.y) {
			continue;
		}

		isInDontcare = std::find(dontCares.begin(), dontCares.end(), i)
				!= dontCares.end();
		if (isInDontcare) {
			continue;
		}

		// on same line is first priority
		if (neighbors[i].hIndex == me.hIndex) {
			float dis2 = squaredDistance(me.x, me.y, neighbors[i].x,
					neighbors[i].y);
			if (dis2 < minDis2) {
				continue;
			}

			pos = i;
			break;
		}
	}

	if (pos == -1) {
		for (size_t i = 0; i < neighbors.size(); i++) {
			if (neighbors[i].x < me.x) {
				continue;
			}

			if (neighbors[i].x == me.x && neighbors[i].y == me.y) {
				continue;
			}

			isInDontcare = std::find(dontCares.begin(), dontCares.end(), i)
					!= dontCares.end();
			if (isInDontcare) {
				continue;
			}

			if (!isOnLineH(me, neighbors[i], maxDot)) {
				continue;
			}

			float dis2 = squaredDistance(me.x, me.y, neighbors[i].x,
					neighbors[i].y);
			if (dis2 < minDis2) {
				continue;
			}

			pos = i;
			break;
		}
	}

	return pos;
}

int lookDown(const intersec_point &me,
		const std::vector<intersec_point> &neighbors,
		const std::vector<int> dontCares, float maxDot) {
	int pos = -1;
	bool isInDontcare;

	for (size_t i = 0; i < neighbors.size(); i++) {
		if (neighbors[i].y < me.y) {
			continue;
		}

		if (neighbors[i].x == me.x && neighbors[i].y == me.y) {
			continue;
		}

		isInDontcare = std::find(dontCares.begin(), dontCares.end(), i)
				!= dontCares.end();
		if (isInDontcare) {
			continue;
		}

		// on same line is first priority
		if (neighbors[i].vIndex == me.vIndex) {
			pos = i;
			break;
		}
	}

	if (pos == -1) {
		for (size_t i = 0; i < neighbors.size(); i++) {
			if (neighbors[i].y < me.y) {
				continue;
			}

			if (neighbors[i].x == me.x && neighbors[i].y == me.y) {
				continue;
			}

			isInDontcare = std::find(dontCares.begin(), dontCares.end(), i)
					!= dontCares.end();
			if (isInDontcare) {
				continue;
			}

			if (!isOnLineV(me, neighbors[i], maxDot)) {
				continue;
			}

			pos = i;
			break;
		}
	}

	return pos;
}

bool checkRect(intersec_point p1, intersec_point p2, intersec_point p3,
		intersec_point p4, int landscape_mode, float &parallelError,
		float &perpendicularError) {
	// 12 parallel 34

	// 23 parallel 14

	// 12 perpendicular 23
	// 23 perpendicular 34
	// 34 perpendicular 12

	Vector2F v12 = Vector2F::normalize(Vector2F(p2.x - p1.x, p2.y - p1.y));
	Vector2F v23 = Vector2F::normalize(Vector2F(p3.x - p2.x, p3.y - p2.y));
	Vector2F v34 = Vector2F::normalize(Vector2F(p4.x - p3.x, p4.y - p3.y));
	Vector2F v41 = Vector2F::normalize(Vector2F(p1.x - p4.x, p1.y - p4.y));

	// should parallel
	float threshold = 0.9659f;	// 15 degree
	float dot = Vector2F::dot(v12, v34);
	if (fabs(dot) < threshold) {
		return false;
	}

	dot = Vector2F::dot(v23, v41);
	if (fabs(dot) < threshold) {
		return false;
	}

	// vert and horz should perpendicular
	threshold = 0.2588f;	// 75 degree

	dot = Vector2F::dot(v12, v23);
	if (dot > threshold) {
		return false;
	}

	dot = Vector2F::dot(v23, v34);
	if (dot > threshold) {
		return false;
	}

	dot = Vector2F::dot(v34, v41);
	if (dot > threshold) {
		return false;
	}

	dot = Vector2F::dot(v41, v12);
	if (dot > threshold) {
		return false;
	}

	parallelError = RADIANS_TO_DEGREES(
			angle2Line(v12, v34)) + RADIANS_TO_DEGREES(angle2Line(v23, v41));

	// angle sum error should be small enough
	perpendicularError = diffWith90(v12, v23) + diffWith90(v23, v34)
			+ diffWith90(v34, v41) + diffWith90(v41, v12);

	if (perpendicularError > 40) {
		return false;
	}

	return true;
}

struct sRect2: public RectF {
	float parallelError;
	float perpendicularError;
};

void getPanels2(const std::vector<cv::Vec4f> &linesV,
		const std::vector<cv::Vec4f> &linesH, int width, int height,
		std::vector<RectF> &rects) {
	rects.clear();

	// convert to rough line
	std::vector<rough_line> vLines;
	for (size_t i = 0; i < linesV.size(); i++) {
		vLines.push_back(convert2Rough(linesV[i]));
	}

	std::vector<rough_line> hLines;
	for (size_t i = 0; i < linesH.size(); i++) {
		hLines.push_back(convert2Rough(linesH[i]));
	}

	// extend line
	int expandLen = 6;
	for (size_t i = 0; i < vLines.size(); i++) {
		extendLine(vLines[i], expandLen);
	}

	for (size_t i = 0; i < hLines.size(); i++) {
		extendLine(hLines[i], expandLen);
	}

	// sort
	std::sort(vLines.begin(), vLines.end(), leftCompare2);
	std::sort(hLines.begin(), hLines.end(), topCompare2);

#	ifdef PANEL_DETECTOR_DEBUG
	saveCombineLines(vLines, hLines, "combine_lines.png");
#	endif

	size_t size = width * height;
	intersec_point **i_image = (intersec_point**) malloc(
			size * sizeof(intersec_point*));
	memset(i_image, 0, size * sizeof(intersec_point*));

	float xIntersect, yIntersect;
	int res, pos;
	int x, y;
	for (size_t v = 0; v < vLines.size(); v++) {
		for (size_t h = 0; h < hLines.size(); h++) {
			if (hLines[h].maxY < vLines[v].minY) {
				continue;
			}

			if (hLines[h].minY > vLines[v].maxY) {
				break;
			}

			if (hLines[h].maxX < vLines[v].minX) {
				continue;
			}

			if (hLines[h].minX > vLines[v].maxX) {
				continue;
			}

			res = TwoLineIntersection(vLines[v], hLines[h], xIntersect,
					yIntersect);

			if (res != 3) {
				continue;
			}

			x = (int) (xIntersect + 0.5f);
			if (x < 0 || x >= width) {
				continue;
			}

			y = (int) (yIntersect + 0.5f);
			if (y < 0 || y >= height) {
				continue;
			}

			intersec_point *pt = (intersec_point*) malloc(
					sizeof(intersec_point));
			pt->vDir = vLines[v].dir;
			pt->hDir = hLines[h].dir;
			pt->vIndex = v;
			pt->hIndex = h;
			pt->x = x;
			pt->y = y;

			pos = y * width + x;
			i_image[pos] = pt;
		}
	}

#	ifdef PANEL_DETECTOR_DEBUG
	saveIntersection(i_image, width, height, "intersection.png");
#	endif

	// look for rectangles
	int pos1;
	int fromX, toX, fromY, toY;

	intersec_point me, top_right_p, bottom_right_p, bottom_left_p;
	float dis1, dis2;
	float ratio;
	float inverseRatio = 1 / gCellRatio;
	float cellRatio;
	int cellMode;
	float parallelError;
	float perpendicularError;

	float maxDot = cos(DEGREES_TO_RADIANS(gMaxAngle));
	std::vector<sRect2> tmp_rects;
	for (int y = 0; y < height; y++) {
		pos = y * width - 1;
		for (int x = 0; x < width; x++) {
			pos++;

			if (i_image[pos] == NULL) {
				continue;
			}

			me = *i_image[pos];

			// get the neighbors
			fromX = std::max(0, x - gMaxLen);
			toX = std::min(width, x + gMaxLen);
			fromY = std::max(0, y - gMaxLen);
			toY = std::min(height, y + gMaxLen);

			std::vector<intersec_point> neighbors;
			for (int y1 = fromY; y1 < toY; y1++) {
				pos1 = y1 * width + fromX - 1;
				for (int x1 = fromX; x1 < toX; x1++) {
					pos1++;

					if (i_image[pos1] == NULL) {
						continue;
					}

					if (pos1 == pos) {
						continue;
					}

					neighbors.push_back(*i_image[pos1]);
				}
			}

			std::vector<int> dontCareTR;
			std::vector<int> dontCareBR;
			std::vector<int> dontCareBL;
			int tr;
			tmp_rects.clear();
			while (true) {
				// look for right
				int ret = lookRight(me, neighbors, dontCareTR, maxDot);

				// if N, add dont care this right
				if (ret == -1) {
					break;
				}

				tr = ret;
				top_right_p = neighbors[ret];

				dis1 = distance(me.x, me.y, top_right_p.x, top_right_p.y);

				// if Y, look for BR
				dontCareBR.clear();
				dontCareBR.push_back(tr);
				bool found = false;
				while (true) {
					ret = lookDown(top_right_p, neighbors, dontCareBR, maxDot);
					if (ret == -1) {
						break;
					}

					bottom_right_p = neighbors[ret];

					// check ratio
					dis2 = distance(bottom_right_p.x, bottom_right_p.y,
							top_right_p.x, top_right_p.y);

					// found 3 points

					// check ratio
					ratio = dis1 / dis2;

					if (dis1 > dis2) {
						// landscape
						cellMode = 0;
						cellRatio = gCellRatio;
					} else {
						// portrait
						cellMode = 1;
						cellRatio = inverseRatio;
					}

					if (fabs(ratio - cellRatio) > 0.2f) {
						dontCareBR.push_back(ret);
						continue;
					}

					found = true;
					break;
				}

				// if N, add dont care this right
				if (!found) {
					dontCareTR.push_back(tr);
					continue;
				}

				// if Y, look for BL
				found = false;
				dontCareBL.clear();
				while (true) {
					ret = lookDown(me, neighbors, dontCareBL, maxDot);
					if (ret == -1) {
						break;
					}

					bottom_left_p = neighbors[ret];

					// found 4 points
					if (!checkRect(me, top_right_p, bottom_right_p,
							bottom_left_p, cellMode, parallelError,
							perpendicularError)) {
						dontCareBL.push_back(ret);
						continue;
					}

					// good point
					sRect2 r;
					r.x1 = me.x;
					r.y1 = me.y;
					r.x2 = top_right_p.x;
					r.y2 = top_right_p.y;
					r.x3 = bottom_right_p.x;
					r.y3 = bottom_right_p.y;
					r.x4 = bottom_left_p.x;
					r.y4 = bottom_left_p.y;

					r.parallelError = parallelError;
					r.perpendicularError = perpendicularError;

					tmp_rects.push_back(r);

					found = true;
					break;
				}

				dontCareTR.push_back(tr);
			}

			// find best rect from this point
			// do later
			if (tmp_rects.size() > 0) {
				rects.insert(rects.end(), tmp_rects.begin(), tmp_rects.end());
			}
		}
	}

	for (size_t i = 0; i < size; i++) {
		if (i_image[i]) {
			free(i_image[i]);
		}
	}

	free(i_image);
}

void getSameAverageSize(std::vector<Rect3> *rects, float &aveW, float &aveH) {
	for (size_t i = 0; i < rects->size(); i++) {
		(*rects)[i].calcSize();
	}

	aveW = aveH = 0;
	float sumDetW = 0;
	float sumDetH = 0;
	float dW, dH;
	for (size_t i = 0; i < rects->size(); i++) {
		for (size_t j = i + 1; j < rects->size(); j++) {
			dW = 1 - diff2ValueMax((*rects)[i].width, (*rects)[j].width);
			sumDetW += 2 * dW;

			dH = 1 - diff2ValueMax((*rects)[i].height, (*rects)[j].height);
			sumDetH += 2 * dH;

			aveW += ((*rects)[i].width + (*rects)[j].width) * dW;
			aveH += ((*rects)[i].height + (*rects)[j].height) * dH;
		}
	}

	aveW /= sumDetW;
	aveH /= sumDetH;
}

int getViewMode(std::vector<Rect3> *rects, float aveW, float aveH) {
	int numLandscape = 0;
	int numPortrait = 0;
	float diff;
	float thres = 0.25f;
	for (size_t i = 0; i < rects->size(); i++) {
		diff = diff2ValueMax((*rects)[i].width, aveW);
		if (diff > thres) {
			continue;
		}

		diff = diff2ValueMax((*rects)[i].height, aveH);
		if (diff > thres) {
			continue;
		}

		// count++;
		if ((*rects)[i].viewMode == 0) {
			numLandscape++;
		} else {
			numPortrait++;
		}
	}

	return (numLandscape >= numPortrait ? 0 : 1);
}

void getSameAverageRatio(std::vector<Rect3> *rects, float &aveRatio) {
	for (size_t i = 0; i < rects->size(); i++) {
		(*rects)[i].calcRatio();
	}

	aveRatio = 0;
	float sumDetR = 0;
	float dR;
	for (size_t i = 0; i < rects->size(); i++) {
		for (size_t j = i + 1; j < rects->size(); j++) {
			dR = 1 - diff2ValueMax((*rects)[i].ratio, (*rects)[j].ratio);
			sumDetR += 2 * dR;

			aveRatio += ((*rects)[i].ratio + (*rects)[j].ratio) * dR;
		}
	}

	aveRatio /= sumDetR;
}

void saveImageWithRects2(std::string pathIn, std::vector<Rect3> rects,
		std::string pathOut) {
	cv::Mat image = cv::imread(pathIn);

	for (size_t i = 0; i < rects.size(); i++) {
		cv::line(image, cv::Point(rects[i].x1, rects[i].y1),
				cv::Point(rects[i].x2, rects[i].y2), cv::Scalar(0, 255, 255),
				1);
		cv::line(image, cv::Point(rects[i].x2, rects[i].y2),
				cv::Point(rects[i].x3, rects[i].y3), cv::Scalar(0, 255, 255),
				1);
		cv::line(image, cv::Point(rects[i].x3, rects[i].y3),
				cv::Point(rects[i].x4, rects[i].y4), cv::Scalar(0, 255, 255),
				1);
		cv::line(image, cv::Point(rects[i].x4, rects[i].y4),
				cv::Point(rects[i].x1, rects[i].y1), cv::Scalar(0, 255, 255),
				1);
	}

	imwrite(pathOut, image);
}

void saveImageWithRects3(std::string pathIn, std::vector<Rect3> rects,
		std::string pathOut) {
	cv::Mat image = cv::imread(pathIn);

	for (size_t i = 0; i < rects.size(); i++) {
		cv::line(image, cv::Point(rects[i].x1, rects[i].y1),
				cv::Point(rects[i].x2, rects[i].y2), cv::Scalar(0, 255, 255),
				1);
		cv::line(image, cv::Point(rects[i].x2, rects[i].y2),
				cv::Point(rects[i].x3, rects[i].y3), cv::Scalar(0, 255, 255),
				1);
		cv::line(image, cv::Point(rects[i].x3, rects[i].y3),
				cv::Point(rects[i].x4, rects[i].y4), cv::Scalar(0, 255, 255),
				1);
		cv::line(image, cv::Point(rects[i].x4, rects[i].y4),
				cv::Point(rects[i].x1, rects[i].y1), cv::Scalar(0, 255, 255),
				1);
	}

	imwrite(pathOut, image);
}

void saveImageWithRectsRotation(std::string pathIn, std::vector<Rect3> rects,
		std::string pathOut) {
	cv::Mat image = cv::imread(pathIn);

	for (size_t i = 0; i < rects.size(); i++) {
		cv::line(image, cv::Point(rects[i].x1, rects[i].y1),
				cv::Point(rects[i].x2, rects[i].y2), cv::Scalar(0, 255, 255),
				1);
		cv::line(image, cv::Point(rects[i].x2, rects[i].y2),
				cv::Point(rects[i].x3, rects[i].y3), cv::Scalar(0, 255, 255),
				1);
		cv::line(image, cv::Point(rects[i].x3, rects[i].y3),
				cv::Point(rects[i].x4, rects[i].y4), cv::Scalar(0, 255, 255),
				1);
		cv::line(image, cv::Point(rects[i].x4, rects[i].y4),
				cv::Point(rects[i].x1, rects[i].y1), cv::Scalar(0, 255, 255),
				1);

		cv::circle(image, cv::Point(rects[i].x1, rects[i].y1), 1,
				cv::Scalar(255, 0, 0));
		cv::circle(image,
				cv::Point((rects[i].x1 + rects[i].x2) / 2,
						(rects[i].y1 + rects[i].y2) / 2), 1,
				cv::Scalar(255, 0, 0));
	}

	imwrite(pathOut, image);
}

bool isGoodSize(Rect3 rect, float aveW, float aveH) {
	rect.calcSize();
	if (rect.width < gMinLen || rect.height < gMinLen) {
		return false;
	}

	if (rect.width > gMaxLen || rect.height > gMaxLen) {
		return false;
	}

	float diffW = diff2ValueMax(rect.width, aveW);
	float diffH = diff2ValueMax(rect.height, aveH);

	float thres = 0.5f;
	return diffW < thres && diffH < thres;
}

void removeBySize(std::vector<Rect3> &rects, float aveW, float aveH) {
	// remove abnormal rect by size
	float thres = 0.25f;
	for (size_t i = 0; i < rects.size(); i++) {
		if (diff2ValueMax(aveW, rects[i].width) > thres
				|| diff2ValueMax(aveH, rects[i].height) > thres) {
			rects.erase(rects.begin() + i);
			i--;
		}
	}
}

void removeByRatio(std::vector<Rect3> &rects, float aveRatio, float threshold) {
	float ratio;
	for (size_t i = 0; i < rects.size(); i++) {
		if (rects[i].width > rects[i].height) {
			ratio = rects[i].width / rects[i].height;
		} else {
			ratio = rects[i].height / rects[i].width;
		}

		if (fabs(ratio - aveRatio) > threshold) {
			rects.erase(rects.begin() + i);
			i--;
			continue;
		}
	}
}

bool isRectFoundInArea(Rect3 rect, std::vector<Rect3> *foundRects) {
	float w = rect.x2 - rect.x1;
	float h = rect.y3 - rect.y2;
	for (size_t i = 0; i < foundRects->size(); i++) {
		if (isInRect((*foundRects)[i].x1, (*foundRects)[i].y1, rect.x1, rect.y1,
				w, h)
				&& isInRect((*foundRects)[i].x2, (*foundRects)[i].y2, rect.x1,
						rect.y1, w, h)
				&& isInRect((*foundRects)[i].x3, (*foundRects)[i].y3, rect.x1,
						rect.y1, w, h)
				&& isInRect((*foundRects)[i].x4, (*foundRects)[i].y4, rect.x1,
						rect.y1, w, h))
			return true;
	}

	return false;
}

bool leftCompare3(Rect3 r1, Rect3 r2) {
	return r1.minX < r2.minX;
}

float areaOverlap(Rect3 r1, Rect3 r2) {
	// for easy, calc on bounding box
	float verlapArea = overlapArea2Rectangles(r1.minX, r1.minY,
			r1.maxX - r1.minX, r1.maxY - r1.minY, r2.minX, r2.minY,
			r2.maxX - r2.minX, r2.maxY - r2.minY);

	float area1 = (r1.maxX - r1.minX) * (r1.maxY - r1.minY);
	float area2 = (r2.maxX - r2.minX) * (r2.maxY - r2.minY);

	return verlapArea / std::max(area1, area2);
}

std::vector<Rect3> mergeRect(std::vector<Rect3> &rects) {
	std::sort(rects.begin(), rects.end(), leftCompare3);

	int interval = 10;
	float overlap;
	std::vector<Rect3> sameRects;
	std::vector<Rect3> resultRects;
	for (size_t i = 0; i < rects.size(); i++) {
		sameRects.clear();
		sameRects.push_back(rects[i]);

		float rectSquarePow = areaQuad(rects[i].x1, rects[i].y1, rects[i].x2,
				rects[i].y2, rects[i].x3, rects[i].y3, rects[i].x4,
				rects[i].y4);
		// -------------test---------------
		int xC = 510;
		int yC = 90;
		if (isInRect(xC, yC, rects[i].minX, rects[i].minY,
				rects[i].maxX - rects[i].minX, rects[i].maxY - rects[i].minY)) {
			xC++;
		}
		// -------------test---------------

		// find the "same" rests
		for (size_t j = i + 1; j < rects.size(); j++) {
			if (rects[j].minX > rects[i].maxX + interval) {
				break;
			}

			if (rects[j].maxX < rects[i].minX - interval) {
				continue;
			}

			if (rects[j].maxY < rects[i].minY - interval) {
				continue;
			}

			if (rects[j].minY > rects[i].maxY + interval) {
				continue;
			}

			overlap = areaOverlap(rects[i], rects[j]);
			if (overlap < 0.4f) {
				continue;
			}

			// check if the rect is twice bigger
			float rectCheckSquarePow = areaQuad(rects[j].x1, rects[j].y1,
					rects[j].x2, rects[j].y2, rects[j].x3, rects[j].y3,
					rects[j].x4, rects[j].y4);
			if (rectCheckSquarePow > rectSquarePow * minRectQuadRate) {

				rects.erase(rects.begin() + j);
				j--;
				continue;
			}

			sameRects.push_back(rects[j]);
			rects.erase(rects.begin() + j);
			j--;
		}

		// merger rect, average so far
		Rect3 rect;
		int size = sameRects.size();

		// find the best
		float area;
		int maxArea = 0;
		std::vector<float> areas;
		for (int j = 0; j < size; j++) {
			area = areaQuad(sameRects[j].x1, sameRects[j].y1, sameRects[j].x2,
					sameRects[j].y2, sameRects[j].x3, sameRects[j].y3,
					sameRects[j].x4, sameRects[j].y4);

			if (area > maxArea) {
				maxArea = area;
			}

			areas.push_back(area);
		}

		float score, bestScore = 0;
		int best = -1;
		for (int j = 0; j < size; j++) {
			score = (areas[j] / maxArea) * 0.6f
					+ (1 - sameRects[j].parallelError / M_PI_2) * 0.4f;
			if (score > bestScore) {
				bestScore = score;
				best = j;
			}
		}

		if (best != -1) {
			resultRects.push_back(sameRects[best]);
		}
	}

	return resultRects;
}

void detectSolarCell(unsigned char *image, int stride, int width, int height,
		std::vector<Rect3> &rects) {
	rects.clear();

	// rough detect
	std::vector<Rect3> rough_rects;
	Rect3 rect;
	int step = gMaxLen / 2;

	Rect3 rectTemp;
	std::vector<Rect3> rectTmps;
	for (int y = 0; y < height; y += step) {
		if (gStopDetect == 1) {
			return;
		}

		for (int x = 0; x < width; x += step) {
			rect.x1 = x;
			rect.y1 = y;
			rect.x2 = x + gMaxLen;
			rect.y2 = y;
			rect.x3 = x + gMaxLen;
			rect.y3 = y + gMaxLen;
			rect.x4 = x;
			rect.y4 = y + gMaxLen;

			if (checkCell2(image, stride, width, height, rect, 0, 0,
					rectTemp)) {
				rough_rects.push_back(rectTemp);
			}
		}
	}

#	ifdef PANEL_DETECTOR_DEBUG
	saveImageWithRects2("high_contrast.png", rough_rects, "rough_rects.png");
#	endif

	if (rough_rects.size() < 3) {
		return;
	}

	// get averge the size of "most same" rects
	float aveW, aveH;
	getSameAverageSize(&rough_rects, aveW, aveH);

	// Do detection again
	int stepW;
	int slideW;
	int stepH;
	int slideH;
	stepW = (int) (aveW / 2);
	slideW = stepW * 3.5;
	stepH = std::max((int) (stepW / gCellRatio), (int) (aveH / 2));
	slideH = stepH * 3.5;

	for (int y = 0; y < height; y += stepH) {
		if (gStopDetect == 1) {
			return;
		}

		for (int x = 0; x < width; x += stepW) {
			if (gStopDetect == 1) {
				return;
			}

			rect.x1 = x;
			rect.y1 = y;
			rect.x2 = x + slideW;
			rect.y2 = y;
			rect.x3 = x + slideW;
			rect.y3 = y + slideH;
			rect.x4 = x;
			rect.y4 = y + slideH;

			if (!checkCell2(image, stride, width, height, rect, aveW, aveH,
					rectTemp)) {
				continue;
			}

			rectTemp.calcSize();
			if (!isGoodSize(rectTemp, aveW, aveH)) {
				continue;
			}

			rectTmps.push_back(rectTemp);
		}
	}

	// get average again
	getSameAverageSize(&rectTmps, aveW, aveH);
	removeBySize(rectTmps, aveW, aveH);

	std::vector<Rect3> rectTmps2 = rectTmps;
	removeByRatio(rectTmps2, gCellRatio, 0.3f);

	float aveRatio;
	getSameAverageRatio(&rectTmps2, aveRatio);
	removeByRatio(rectTmps, aveRatio, 0.35f);

	if (gStopDetect == 1) {
		return;
	}

	// merge rects
	for (size_t i = 0; i < rectTmps.size(); i++) {
		rectTmps[i].normalizeRotation();
		rectTmps[i].normalizePosition();
		rectTmps[i].calcBound();
	}

	if (gStopDetect == 1) {
		return;
	}

	rectTmps = mergeRect(rectTmps);

	for (size_t i = 0; i < rectTmps.size(); i++) {
		rects.push_back(rectTmps[i]);
	}

#	ifdef PANEL_DETECTOR_DEBUG
	saveImageWithRects3("high_contrast.png", rects, "rough_rects_final.png");
#	endif
}
