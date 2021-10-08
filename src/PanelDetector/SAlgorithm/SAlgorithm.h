/*
 * SAlgorithm.h
 *
 *  Created on: 2020/04/09
 *      Author: k.tasaki
 */

#ifndef S_ALGORITHM_H_
#define S_ALGORITHM_H_

// khac biet % so voi gia tri trung binh
//  0 -> 0%
//  1 -> 100%
float diff2ValueAverage(float v1, float v2);

// khac biet % so voi gia tri max
//  0 -> 0%
//  1 -> 100%
float diff2ValueMax(float v1, float v2);

// khac biet % so voi gia tri max
// dung khi ca 2 deu khg am
//  0 -> 0%
//  1 -> 100%
float diff2PositiveValueMax(float v1, float v2);

// signed return
float diff2ValueSign(float v1, float v2);

#endif
