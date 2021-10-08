/*
 * PanelDetector.h
 *
 *  Created on: 2016/01/05
 *      Author: PC-EFFECT-012
 */

#ifndef PANELDETECTOR_H_
#define PANELDETECTOR_H_

#include <windows.h>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace PanelDetector {

void findSquares(const cv::Mat *image,
		std::vector<std::vector<cv::Point> > *squares);
void findPanels(const cv::Mat *image,
		std::vector<std::vector<cv::Point> > *panelArea,
		std::vector<std::vector<cv::Point> > *panels);
double getAngle(std::vector<std::vector<cv::Point> > *panels);

double getPanelAngle(LPCTSTR fileName);

/**
 * 指定画像内の太陽光パネルを検出します。
 *
 *　@param imageOrigin 対象の画像
 *　@param panels 検出したパネル
 */
void findSolarPanels(const cv::Mat &imageOrigin,
		std::vector<std::vector<cv::Point> > &panels);

/**
 * 指定画像内の太陽光パネルを検出します。
 *
 *　@param imageOrigin 対象の画像
 *　@param panels 検出したパネル
 *　@param detectMode 検出方法
 */
void findSolarPanels(const cv::Mat &imageOrigin,
		std::vector<std::vector<cv::Point> > &panels, int detectMode);

}

#endif /* PANELDETECTOR_H_ */
