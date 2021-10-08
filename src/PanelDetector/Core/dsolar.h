/*
 * Author : Le Dung , FPT
 * Date : 2018-06-15
 * Last-modified by : Le Dung, FPT
 * Last-modified : 2018-06-15
 */

#ifndef __DSOLAR__
#define __DSOLAR__

#include <vector>
#include <opencv2/opencv.hpp>
#include "dstruct.h"

void validateCells(unsigned char *image, int stride, int width, int height,
		std::vector<RectF> &rects);

void solarCanny(unsigned char *image, int stride, int width, int height,
		std::vector<cv::Vec4f> &linesV, std::vector<cv::Vec4f> &linesH);

void getPanels2(const std::vector<cv::Vec4f> &linesV,
		const std::vector<cv::Vec4f> &linesH, int width, int height,
		std::vector<RectF> &rects);

void detectSolarCell(unsigned char *image, int stride, int width, int height,
		std::vector<Rect3> &rects);

std::vector<Rect3> mergeRect(std::vector<Rect3> &rects);
#endif
