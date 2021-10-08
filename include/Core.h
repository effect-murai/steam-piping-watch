/*
 * Core.h
 *
 *  Created on: 2020/06/17
 *      Author: k.tasaki
 */

#ifndef CORE_H_
#define CORE_H_

#include <stdio.h>

void* allocateMemory(size_t size, const char *file, int line);

#define allocMemory(type, size) ((type *) allocateMemory(size, __FILE__, __LINE__))

#endif /* CORE_H_ */
