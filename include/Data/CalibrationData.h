/*
 * CalibrationData.h
 *
 *  Created on: 2016/02/01
 *      Author: PC-EFFECT-011
 */

#ifndef CALIBRATIONDATA_H_
#define CALIBRATIONDATA_H_

#include "CommonData.h"

/**
 * 起動時キャリブレーションデータ
 */
class CalibrationData {
public:
	CalibrationData(const TCHAR *path);
	virtual ~CalibrationData(void);

	GPSPictureInfo& getAverage(void);
	double getGPSHeight(void);
	bool isAvailable(void);

private:
	int sumCount;
	GPSPictureInfo data;
	bool available;
};

#endif /* CALIBRATIONDATA_H_ */
