/*
 * version.h
 *
 *  Created on: 2020/05/18
 *      Author: k.tasaki
 */

#ifndef INCLUDE_VERSION_H_
#define INCLUDE_VERSION_H_

#define APP_STRINGIZE0(expr) #expr

#define APP_STRINGIZE(expr) APP_STRINGIZE0(expr)

/**
 * Major release version number.
 */
#define APP_RELEASE_MAJOR 6

/**
 * Minor release version number.
 */
#define APP_RELEASE_MINOR 1

/**
 * Tiny release version number.
 */
#define APP_RELEASE_TEENY 0

/**
 * Maintenance release version number.
 */
#define APP_RELEASE_MAINTENANCE 1

/**
 * The application version.
 */
#define APP_VERSION \
	APP_STRINGIZE(APP_RELEASE_MAJOR) "." \
	APP_STRINGIZE(APP_RELEASE_MINOR) "." \
	APP_STRINGIZE(APP_RELEASE_TEENY)

/*
 * Release file version.
 */
#define APP_RELEASE_FILE_VERSION \
	APP_RELEASE_MAJOR, APP_RELEASE_MINOR, \
	APP_RELEASE_TEENY, APP_RELEASE_MAINTENANCE

/*
 * Release year.
 */
#define APP_RELEASE_YEAR 2021

/*
 * Release month.
 */
#define APP_RELEASE_MONTH 1

/*
 * Release day.
 */
#define APP_RELEASE_DAY 14

/*
 * Release date as a string.
 */
#define APP_RELEASE_DATE    \
  APP_RELEASE_YEAR_STR "-"  \
  APP_RELEASE_MONTH_STR "-" \
  APP_RELEASE_DAY_STR
#define APP_RELEASE_YEAR_STR APP_STRINGIZE(APP_RELEASE_YEAR)
#if APP_RELEASE_MONTH < 10
#define APP_RELEASE_MONTH_STR "0" APP_STRINGIZE(APP_RELEASE_MONTH)
#else
#define APP_RELEASE_MONTH_STR APP_STRINGIZE(APP_RELEASE_MONTH)
#endif
#if APP_RELEASE_DAY < 10
#define APP_RELEASE_DAY_STR "0" APP_STRINGIZE(APP_RELEASE_DAY)
#else
#define APP_RELEASE_DAY_STR APP_STRINGIZE(APP_RELEASE_DAY)
#endif

/*
 * The year application was first created.
 */
#define APP_BIRTH_YEAR 2015

/*
 * The copyright information of the application.
 */
#define APP_COPYRIGHT \
	"Copyright (c) " \
	APP_STRINGIZE(APP_BIRTH_YEAR) "-" \
	APP_STRINGIZE(APP_RELEASE_YEAR) " "

#endif /* INCLUDE_VERSION_H_ */
