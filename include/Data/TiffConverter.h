/*
 * TiffConverter.h
 *
 *  Created on: 2021/08/10
 *      Author: k.tasaki
 */

#ifndef DATA_TIFFCONVERTER_H_
#define DATA_TIFFCONVERTER_H_

namespace TiffConverter {

#define TIFF_CONVERTER_SUCCESS 0
#define TIFF_CONVERTER_OPEN_FOLDER_ERROR 1
#define TIFF_CONVERTER_NO_INFRARED_ERROR 2
#define TIFF_CONVERTER_TOO_MANY_INFRARED_ERROR 3
#define TIFF_CONVERTER_VISIBLE_COUNT_ERROR 4
#define TIFF_CONVERTER_INVALID_INFRARED_ERROR 5
#define TIFF_CONVERTER_INVALID_VISIBLE_ERROR 6
#define TIFF_CONVERTER_CREATE_FLIGHT_DATA_ERROR 7

int convert(const wchar_t *path, int *progress);

}

#endif /* DATA_TIFFCONVERTER_H_ */
