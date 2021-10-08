/*
 * UtilFuncs.h
 *
 *  Created on: 2020/04/09
 *      Author: k.tasaki
 */

#ifndef __UTIL_FUNCS__
#define __UTIL_FUNCS__

#include <vector>

double sigmoid(double x);
double sigmoidDeriv(double x);

double average(std::vector<float> values);

// square of length of diff vector
double calcCost(std::vector<float> v1, std::vector<float> v2);

#endif
