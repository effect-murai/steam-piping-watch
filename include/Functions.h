/*
 * Functions.h
 *
 *  Created on: 2016/03/09
 *      Author: PC-EFFECT-012
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include <windows.h>

inline void rectToPointArray(RECT *src, POINT *dest) {
	// 左上
	dest[0].x = src->left;
	dest[0].y = src->top;
	// 左下
	dest[1].x = src->left;
	dest[1].y = src->bottom;
	// 右下
	dest[2].x = src->right;
	dest[2].y = src->bottom;
	// 右上
	dest[3].x = src->right;
	dest[3].y = src->top;
}

inline void rectToPointArray2(RECT *src, POINT *dest) {
	dest[0].x = src->left;
	dest[0].y = src->top;
	dest[1].x = src->right;
	dest[1].y = src->bottom;
}

#include "Data/CommonData.h"
/**
 * ホットスポット番号を生成する
 */
inline HotspotNumber hotspotNo(int picNo, int ptNo) {
	HotspotNumber num;
	num.pictureNo = picNo;
	num.pointNo = ptNo;
	return num;
}

#endif /* FUNCTIONS_H_ */
