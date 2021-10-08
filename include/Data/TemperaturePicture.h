/*
 * TemperaturePicture.h
 *
 *  Created on: 2020/06/11
 *      Author: k.tasaki
 */

#ifndef DATA_TEMPERATUREPICTURE_H_
#define DATA_TEMPERATUREPICTURE_H_

#include <windows.h>
#include <gdiplus.h>

namespace TemperaturePicture {
enum InfraredImageType {
	GRAY_SCALE = 0,
};

Gdiplus::Bitmap* create(float *tempMap, int width, int height, double min,
		double max, int type);
}

#endif /* DATA_TEMPERATUREPICTURE_H_ */
