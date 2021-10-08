/*
 * tiff.c
 *
 *  Created on: 2021/08/10
 *      Author: Kiyoshi TASAKI <k.tasaki@effect-effect.com>
 */

#include "tiff.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "debug.h"

#ifdef _WIN32
#include <windows.h>
#endif

#define bool int
#define true 1
#define false 0

#pragma pack(push)
#pragma pack(1)

typedef struct {
	unsigned short tag;
	unsigned short type;
	unsigned int count;
	unsigned int value;
} TiffIFDTableEntry;

typedef struct {
	unsigned short count;
	TiffIFDTableEntry entries[];
} TiffIFDTable;

typedef struct {
	unsigned int numerator;
	unsigned int denominator;
} TiffRational;

#pragma pack(pop)

#define TIFF_IFD_ENTRY_TYPE_BYTE 0x0001
#define TIFF_IFD_ENTRY_TYPE_ASCII 0x0002
#define TIFF_IFD_ENTRY_TYPE_SHORT 0x0003
#define TIFF_IFD_ENTRY_TYPE_LONG 0x0004
#define TIFF_IFD_ENTRY_TYPE_RATIONAL 0x0005
#define TIFF_IFD_ENTRY_TYPE_SBYTE 0x0006
#define TIFF_IFD_ENTRY_TYPE_UNDEFINED 0x0007
#define TIFF_IFD_ENTRY_TYPE_SSHORT 0x0008
#define TIFF_IFD_ENTRY_TYPE_SLONG 0x0009
#define TIFF_IFD_ENTRY_TYPE_SRATIONAL 0x000a
#define TIFF_IFD_ENTRY_TYPE_FLOAT 0x000b
#define TIFF_IFD_ENTRY_TYPE_DOUBLE 0x000c

#define TIFF_IFD_GPS_VERSION_ID 0x0000
#define TIFF_IFD_GPS_LATITUDE_REF 0x0001
#define TIFF_IFD_GPS_LATITUDE 0x0002
#define TIFF_IFD_GPS_LONGITUDE_REF 0x0003
#define TIFF_IFD_GPS_LONGITUDE 0x0004
#define TIFF_IFD_GPS_ALTITUDE_REF 0x0005
#define TIFF_IFD_GPS_ALTITUDE 0x0006
#define TIFF_IFD_GPS_TIME_STAMP 0x0007
#define TIFF_IFD_GPS_MEASURE_MODE 0x000A
#define TIFF_IFD_GPS_SPEED_REF 0x000C
#define TIFF_IFD_GPS_SPEED 0x000D
#define TIFF_IFD_GPS_TRACK_REF 0x000E
#define TIFF_IFD_GPS_TRACK 0x000F
#define TIFF_IFD_GPS_IMG_DIRECTION_REF 0x0010
#define TIFF_IFD_GPS_IMG_DIRECTION 0x0011
#define TIFF_IFD_GPS_MAP_DATUM 0x0012
#define TIFF_IFD_GPS_PROCESSING_METHOD 0x001B
#define TIFF_IFD_GPS_DATE_STAMP 0x001D

#ifndef _WIN32
inline char *toMultiByte(const wchar_t *wcs) {
	size_t origSize = wcslen(wcs);
	size_t size = wcstombs(NULL, wcs, origSize);
	char *dest = (char *)malloc(size + 1);
	if (dest != NULL) {
		dest[size] = '\0';
		wcstombs(dest, wcs, origSize);
	}
	return dest;
}

/**
 * Open a file and create a new stream for it.
 *
 * This function is a possible cancellation point and therefore not
 * marked with __THROW.
 */
inline FILE *_wfopen(
	const wchar_t *__restrict__ __filename, const wchar_t *__restrict __modes) {
	char *path = toMultiByte(__filename);
	char *mode = toMultiByte(__modes);
	FILE *file = NULL;
	if (path != NULL && mode != NULL) {
		file = fopen(path, mode);
	}
	if (path != NULL) {
		free(path);
	}
	if (mode != NULL) {
		free(mode);
	}
	return file;
}
#endif // _WIN32

static __inline__ char* readFile(const wchar_t *path, size_t *size) {
	FILE *file = _wfopen(path, L"rb");
	if (file == NULL) {
		*size = 0;
		return NULL;
	}
	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	fseek(file, 0, SEEK_SET);
	char *buf = (char*) malloc(*size);
	if (buf == NULL) {
		fclose(file);
		*size = 0;
		return NULL;
	}
	fread(buf, *size, 1, file);
	fclose(file);
	return buf;
}

static __inline__ double rationalToDouble(const TiffRational *rational) {
	return (double) rational->numerator / rational->denominator;
}

static __inline__ const char* getString(const void *rawData,
		const TiffIFDTableEntry *entry) {
	if (entry->count > 4) {
		return (char*) rawData + entry->value;
	} else {
		return (char*) &entry->value;
	}
}

inline const unsigned char* getByte(const void *rawData,
		const TiffIFDTableEntry *entry) {
	if (entry->count > 4) {
		return (unsigned char*) rawData + entry->value;
	} else {
		return (unsigned char*) &entry->value;
	}
}

static __inline__ const unsigned short* getShort(const void *rawData,
		const TiffIFDTableEntry *entry) {
	if (entry->count > 4) {
		return (unsigned short*) ((char*) rawData + entry->value);
	} else {
		return (unsigned short*) &entry->value;
	}
}

static __inline__ const unsigned int* getLong(const void *rawData,
		const TiffIFDTableEntry *entry) {
	if (entry->count > 4) {
		return (unsigned int*) ((char*) rawData + entry->value);
	} else {
		return (unsigned int*) &entry->value;
	}
}

#ifdef _DEBUG

static __inline__ const char *getTiffIFDTypeName(unsigned short type) {
	switch (type) {
	case TIFF_IFD_ENTRY_TYPE_BYTE:
		// unsigned char N bytes
		return "byte";
	case TIFF_IFD_ENTRY_TYPE_ASCII:
		// null-terminated ascii string N bytes (N must include null)
		return "ascii";
	case TIFF_IFD_ENTRY_TYPE_SHORT:
		// unsigned short N * 2 bytes
		return "short";
	case TIFF_IFD_ENTRY_TYPE_LONG:
		// unsigned long N * 4 bytes
		return "long";
	case TIFF_IFD_ENTRY_TYPE_RATIONAL:
		// pair of unsigned longs N * 8 bytes
		return "rational";
	case TIFF_IFD_ENTRY_TYPE_SBYTE:
		// char N bytes
		return "sbyte";
	case TIFF_IFD_ENTRY_TYPE_UNDEFINED:
		// -- N bytes
		return "undefined";
	case TIFF_IFD_ENTRY_TYPE_SSHORT:
		// short N * 2 bytes
		return "sshort";
	case TIFF_IFD_ENTRY_TYPE_SLONG:
		// long N * 4 bytes
		return "slong";
	case TIFF_IFD_ENTRY_TYPE_SRATIONAL:
		// pair of longs N * 8 bytes
		return "rational";
	case TIFF_IFD_ENTRY_TYPE_FLOAT:
		// float N * 4 bytes
		return "float";
	case TIFF_IFD_ENTRY_TYPE_DOUBLE:
		// double N * 8 bytes
		return "double";
	default:
		return "unknown";
	}
}

static __inline__ const char *getTiffIFDTagName(unsigned short tag) {
	switch (tag) {
	case 0x0100:
		return "ImageWidth";
	case 0x0101:
		return "ImageLength";
	case 0x0102:
		return "BitsPerSample";
	case 0x0103:
		return "Compression";
	case 0x0106:
		return "PhotometricInterpretation";
	case 0x0111:
		return "StripOffsets";
	case 0x0116:
		return "RowsPerStrip";
	case 0x0117:
		return "StripByteCounts";
	case 0x011a:
		return "XResolution";
	case 0x011b:
		return "YResolution";
	case 0x0128:
		return "ResolutionUnit";
	case 0x010f:
		return "Make";
	case 0x0110:
		return "Model";
	case 0x0115:
		return "SamplesPerPixel";
	case 0x0129:
		return "PageNumber";
	case 0x0131:
		return "Software";
	case 0x0153:
		return "SampleFormat";
	case 0x02bc:
		return "XMPMetadata";
	case 0x8769:
	case 0x8825:
		return "_GPS_IFD";
	default:
		return "Unknown";
	}
}

static __inline__ const char *getGPSIFDTagName(unsigned short tag) {
	switch (tag) {
	case 0x0000:
		return "GPSVersionID";
	case TIFF_IFD_GPS_LATITUDE_REF:
		return "GPSLatitudeRef";
	case TIFF_IFD_GPS_LATITUDE:
		return "GPSLatitude";
	case TIFF_IFD_GPS_LONGITUDE_REF:
		return "GPSLongitudeRef";
	case TIFF_IFD_GPS_LONGITUDE:
		return "GPSLongitude";
	case TIFF_IFD_GPS_ALTITUDE_REF:
		return "GPSAltitudeRef";
	case TIFF_IFD_GPS_ALTITUDE:
		return "GPSAltitude";
	case TIFF_IFD_GPS_TIME_STAMP:
		return "GPSTimeStamp";
	case TIFF_IFD_GPS_MEASURE_MODE:
		return "GPSMeasureMode";
	case TIFF_IFD_GPS_SPEED_REF:
		return "GPSSpeedRef";
	case TIFF_IFD_GPS_SPEED:
		return "GPSSpeed";
	case TIFF_IFD_GPS_TRACK_REF:
		return "GPSTrackRef";
	case 0x000F:
		return "GPSTrack";
	case 0x0010:
		return "GPSImgDirectionRef";
	case 0x0011:
		return "GPSImgDirection";
	case 0x0012:
		return "GPSMapDatum";
	case 0x001B:
		return "GPSProcessingMethod";
	case 0x001D:
		return "GPSDateStamp";
	case 0x829d:
		return "FNumber";
	case 0x9003:
		return "DateTimeOriginal";
	case 0x920a:
		return "FocalLength";
	case 0x9291:
		return "SubSecTimeOriginal";
	case 0xa20e:
		return "FocalPlaneXResolution";
	case 0xa20f:
		return "FocalPlaneYResolution";
	case 0xa210:
		return "FocalPlaneResolutionUni";
	default:
		return "Unknown";
	}
}

#define showValue(buffer, pEntry, indentLevel, tagName)                        \
	{                                                                          \
		char indent[indentLevel + 1];                                          \
		for (int i = 0; i < indentLevel; i++) {                                \
			indent[i] = ' ';                                                   \
		}                                                                      \
		indent[indentLevel] = '\0';                                            \
		unsigned short tag = pEntry->tag;                                      \
		unsigned short type = pEntry->type;                                    \
		unsigned int count = pEntry->count;                                    \
		unsigned int value = pEntry->value;                                    \
		debug("%sTAG:   %s(0x%04x)", indent, tagName(tag), tag);               \
		debug("%sType:  %s(0x%04x)", indent, getTiffIFDTypeName(type), type);  \
		debug("%sCount: %d", indent, count);                                   \
		switch (type) {                                                        \
		case TIFF_IFD_ENTRY_TYPE_BYTE:                                         \
			if (count > 1) {                                                   \
				debug("%sValue: 0x%08x", indent, value);                       \
			} else {                                                           \
				debug("%sValue: %u", indent, value);                           \
			}                                                                  \
			break;                                                             \
		case TIFF_IFD_ENTRY_TYPE_ASCII:                                        \
			debug("%sValue: %s", indent, getString(buffer, pEntry));           \
			break;                                                             \
		case TIFF_IFD_ENTRY_TYPE_SHORT: {                                      \
			const unsigned short *value = getShort(buffer, pEntry);            \
			for (unsigned int j = 0; j < count; j++) {                         \
				debug("%sValue: %u", indent, value[j]);                        \
			}                                                                  \
		} break;                                                               \
		case TIFF_IFD_ENTRY_TYPE_LONG:                                         \
			if (tag == 0x0111) {                                               \
				debug("%sValue: 0x%08x", indent, value);                       \
			} else {                                                           \
				const unsigned int *value = getLong(buffer, pEntry);           \
				for (unsigned int j = 0; j < count; j++) {                     \
					debug("%sValue: %u", indent, value[j]);                    \
				}                                                              \
			}                                                                  \
			break;                                                             \
		case TIFF_IFD_ENTRY_TYPE_RATIONAL: {                                   \
			TiffRational *values = (TiffRational *)((char *)buffer + value);   \
			for (unsigned int j = 0; j < count; j++) {                         \
				debug("%sValue: %u / %u = %lg", indent, values[j].numerator,   \
					values[j].denominator, rationalToDouble(&values[j]));      \
			}                                                                  \
		} break;                                                               \
		default:                                                               \
			debug("%sValue: 0x%08x", indent, value);                           \
			break;                                                             \
		}                                                                      \
	}
#else
#define showValue(...)
#endif

static __inline__ void setDateTimeOriginal(const char *str, TiffInfo *info) {
	if (sscanf(str, "%4d:%4d:%4d %4d:%4d:%4d", &info->time.tm_year,
			&info->time.tm_mon, &info->time.tm_mday, &info->time.tm_hour,
			&info->time.tm_min, &info->time.tm_sec) == 6) {
		// 年を1900年起算に変更
		info->time.tm_year -= 1900;
		// 月を0起算に変更
		info->time.tm_mon -= 1;
	} else {
		memset(&(info->time), 0, sizeof(struct tm));
	}
}

static __inline__ double degreeRationalToDouble(const void *buf, unsigned int value) {
	TiffRational *values = (TiffRational*) ((char*) buf + value);
	double degrees = rationalToDouble(&values[0]);
	double minutes = rationalToDouble(&values[1]);
	double seconds = rationalToDouble(&values[2]);
	return degrees + minutes / 60 + seconds / 3600;
}

static __inline__ double getSubSecTimeOrdinal(const void *buf, unsigned int count,
		unsigned int value) {
	double subsec;
	if (count > 4) {
		subsec = atof((char*) buf + value);
	} else {
		subsec = atof((char*) &value);
	}
	double n = count - 1;
	return subsec * pow(10, 6 - n);
}

static __inline__ void parseExifMetadataIFD(const void *buf, unsigned int offset,
		TiffInfo *info) {
	TiffIFDTable *pIFD = (TiffIFDTable*) ((char*) buf + offset);
	debug("    GPS IFD: %08x", offset);
	unsigned short ifdCount = pIFD->count;
	debug("      Count: %d", ifdCount);
	TiffIFDTableEntry *pEntries = pIFD->entries;
	double latitude = 0.0, longitude = 0.0, altitude = 0.0;
	int latitudeRef = 0, longitudeRef = 0, altitudeRef = 0;
	for (int i = 0; i < ifdCount; i++) {
		debug("      Entry %d:", i);
		TiffIFDTableEntry *pEntry = &pEntries[i];
		showValue(buf, pEntry, 8, getGPSIFDTagName);
		switch (pEntry->tag) {
		case TIFF_IFD_GPS_LATITUDE_REF:
			if (pEntry->value == 'N') {
				latitudeRef = 1;
			} else if (pEntry->value == 'S') {
				latitudeRef = -1;
			}
			break;
		case TIFF_IFD_GPS_LONGITUDE_REF:
			if (pEntry->value == 'E') {
				longitudeRef = 1;
			} else if (pEntry->value == 'W') {
				longitudeRef = -1;
			}
			break;
		case TIFF_IFD_GPS_ALTITUDE_REF:
			altitudeRef = pEntry->value;
			break;
		case TIFF_IFD_GPS_LATITUDE:
			latitude = degreeRationalToDouble(buf, pEntry->value);
			break;
		case TIFF_IFD_GPS_LONGITUDE:
			longitude = degreeRationalToDouble(buf, pEntry->value);
			break;
		case TIFF_IFD_GPS_ALTITUDE:
			altitude = rationalToDouble(
					(TiffRational*) ((char*) buf + pEntry->value));
			break;
		case 0x9003:
			setDateTimeOriginal((const char*) buf + pEntry->value, info);
			break;
		case 0x9291:
			info->tm_usec = getSubSecTimeOrdinal(buf, pEntry->count,
					pEntry->value);
			break;
		}
	}

	info->gps.latitude = latitude * latitudeRef;
	info->gps.longitude = longitude * longitudeRef;
	if (info->gps.altitude == 0) {
		if (altitudeRef == 1) {
			info->gps.altitude = -altitude;
		} else {
			info->gps.altitude = altitude;
		}
	}
}

static __inline__ void setMetadataFromXMP(const char *xmp, unsigned int count,
		TiffInfo *info) {
	const char altitudeTag[] = "drone-dji:RelativeAltitude";
	unsigned int length = sizeof(altitudeTag) - 1;
	debug("Set %s", altitudeTag);
	unsigned int size = count - length;
	debug("size: %d", size);
	for (unsigned int i = 0; i < size; i++) {
		if (memcmp(altitudeTag, &xmp[i], length) == 0) {
			switch (xmp[i + length]) {
			case '=':
				// 属性形式の場合
				sscanf(&xmp[i + length + 1], "\"%lf\"", &info->gps.altitude);
				break;
			case '>':
				// タグ形式の場合
				sscanf(&xmp[i + length + 1], "%lf<", &info->gps.altitude);
				break;
			}
			break;
		}
	}
}

static __inline__ unsigned int parseTiffMetadataIFD(const void *buffer,
		unsigned int offset, TiffInfo *info) {
	TiffIFDTable *pIFD = (TiffIFDTable*) ((char*) buffer + offset);
	unsigned short ifdCount = pIFD->count;
	debug("  Count: %d", ifdCount);
	TiffIFDTableEntry *pEntries = pIFD->entries;
	for (int i = 0; i < ifdCount; i++) {
		debug("  Entry %d:", i);
		TiffIFDTableEntry *pEntry = &pEntries[i];
		showValue(buffer, pEntry, 4, getTiffIFDTagName);
		switch (pEntry->tag) {
		case 0x8825:
			parseExifMetadataIFD(buffer, pEntry->value, info);
			break;
		case 0x8769:
			parseExifMetadataIFD(buffer, pEntry->value, info);
			break;
		case 0x02bc:
			info->xmp = getString(buffer, pEntry);
			debug("\n--------\n%s\n--------", info->xmp);
			if (info->gps.altitude == 0) {
				setMetadataFromXMP(info->xmp, pEntry->count, info);
			}
			break;
		case 0x010f:
			info->manufacturer = getString(buffer, pEntry);
			break;
		case 0x0110:
			info->model = getString(buffer, pEntry);
			break;
		case 0x0111:
			info->imageData = ((unsigned char*) buffer + pEntry->value);
			break;
		case 0x0100:
			info->width = pEntry->value;
			break;
		case 0x0101:
			info->height = pEntry->value;
			break;
		case 0x0117:
			info->imageDataSize = pEntry->value;
			break;
		}
	}
	return *(unsigned int*) (&pIFD->entries[ifdCount]);
}

static __inline__ void parseTiffIFDTable(const void *buffer, TiffInfo *info) {
	unsigned int addrIFD = *(unsigned int*) ((char*) buffer + 4);
	debug("First IFD: 0x%08x", addrIFD);
	while (addrIFD != 0) {
		addrIFD = parseTiffMetadataIFD(buffer, addrIFD, info);
		debug("Next IFD: 0x%08x", addrIFD);
	}
}

bool readTiffFile(const wchar_t *fileName, TiffInfo *tiffInfo) {
#ifdef _DEBUG
	assert(sizeof(TiffIFDTableEntry) == 12);
#endif
	memset(tiffInfo, 0, sizeof(TiffInfo));
	size_t size;
	char *buffer = readFile(fileName, &size);
	if (buffer == NULL) {
		debug("Can't read file.");
		return false;
	}
	// 先頭2バイトを取得
	unsigned short tiffHeader = *(unsigned short*) buffer;
	if (tiffHeader == 0x4949) {
		debug("Tiff Header: 0x%04x", tiffHeader);
	} else if (tiffHeader == 0x4d4d) {
		debug("Can't support a big-endian format.");
		free(buffer);
		return false;
	} else {
		debug("Can't support a format other than TIFF.");
		free(buffer);
		return false;
	}
	// 次の2バイトを取得
	unsigned short constant = *(unsigned short*) ((char*) buffer + 2);
	if (constant == 0x002a) {
		debug("Constant: 0x%04x OK", constant);
		tiffInfo->rawData = (const unsigned char*) buffer;
		tiffInfo->imageDataSize = size;
		parseTiffIFDTable(tiffInfo->rawData, tiffInfo);
	} else {
		debug("Constant: 0x%04x NG", constant);
		free(buffer);
		return false;
	}
	return true;
}

void destroyTiffInfo(TiffInfo *tiffInfo) {
	if (tiffInfo->rawData != NULL) {
		free((void*) tiffInfo->rawData);
	}
	memset(tiffInfo, 0, sizeof(TiffInfo));
}
