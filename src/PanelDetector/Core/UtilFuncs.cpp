#include "UtilFuncs.h"
#include <math.h>
#include <assert.h>

double sigmoid(double x) {
	return 1 / (1 + exp(-x));
}

double sigmoidDeriv(double x) {
	return exp(-x) / pow((1 + exp(-x)), 2);
}

double average(std::vector<float> values) {
	if (values.empty()) {
		return 0;
	}

	double sum = 0;
	for (size_t i = 0; i < values.size(); i++) {
		sum += values[i];
	}

	sum /= values.size();
	return sum;
}

// square of length of diff vector
double calcCost(std::vector<float> v1, std::vector<float> v2) {
	assert(v1.size() == v2.size());

	double sum = 0;
	for (size_t i = 0; i < v1.size(); i++) {
		sum += (v1[i] - v2[i]) * (v1[i] - v2[i]);
	}

	return sum;
}
