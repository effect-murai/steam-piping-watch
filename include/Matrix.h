/*
 * Matrix.h
 *
 *  Created on: 2016/03/14
 *      Author: PC-EFFECT-002
 */

#ifndef MATRIX_H_
#define MATRIX_H_

template<typename _Tp>
class Matrix {
public:
	Matrix(int width, int height) :
			width(width), height(height) {
		array = new _Tp[width * height];
	}
	virtual ~Matrix(void) {
		delete array;
	}
	inline int getWidth(void) {
		return (int) width;
	}
	inline int getHeight(void) {
		return (int) height;
	}
	inline int arraySize(void) {
		return (int) (width * height);
	}
	inline void clear(void) {
		ZeroMemory(array, width * height * sizeof(_Tp));
	}
	inline void set(unsigned int x, unsigned int y, _Tp val) {
		array[x + y * width] = val;
	}
	inline _Tp get(unsigned int x, unsigned int y) {
		return array[x + y * width];
	}
	inline _Tp& at(unsigned int x, unsigned int y) {
		return array[x + y * width];
	}
private:
	_Tp *array;
	unsigned int width;
	unsigned int height;
};

#endif /* MATRIX_H_ */
