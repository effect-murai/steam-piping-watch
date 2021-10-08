/*
 * FlightData.h
 *
 *  Created on: 2016/02/01
 *      Author: PC-EFFECT-011
 */

#ifndef FLIGHTDATA_H_
#define FLIGHTDATA_H_

#include "CommonData.h"

/**
 * 飛行中結果データ
 */
class FlightData {
public:
	FlightData(const TCHAR *path);
	virtual ~FlightData(void);

	bool getCorrectedGPSData(GPSPictureInfo *info, struct tm *time,
			double speed, int points);
	GPSPictureInfo operator [](unsigned int id);
	double getGPSCardinalDirection(struct tm *time);
	bool isAvailable(void);
	int getCount(void);

private:
	std::vector<GPSPictureInfo> data;
	bool available;
};

#endif /* FLIGHTDATA_H_ */
