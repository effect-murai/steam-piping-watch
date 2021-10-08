/*
 * LicenseManager.h
 *
 *  Created on: 2020/12/01
 *      Author: Kiyoshi TASAKI <k.tasaki@effect-effect.com>
 */

#ifndef LICENSEMANAGER_H_
#define LICENSEMANAGER_H_

#include <windows.h>
#include <string>
#include "Common/Future.h"

#ifdef DEBUG
// デバッグ用
#	define LICENSE_SERVER_BASE_URL TEXT("http://localhost:8080/api/")
#else
// リリース用
#	define LICENSE_SERVER_BASE_URL TEXT("https://iot-ninja.com/api/")
#endif

/**
 * 契約種別
 */
enum ContractType {
	/**
	 * 月額利用プラン
	 */
	Subscription,
	/**
	 * 買取プラン(保守あり)
	 */
	Package,
	/**
	 * 買取プラン(保守なし)
	 */
	NoSupport,
};

class LicenseManager {
public:
	LicenseManager(LPCTSTR baseUrl);

	Future<bool> regist(LPCTSTR company, LPCTSTR unit, LPCTSTR name,
			LPCTSTR email, LPCTSTR machineCode, ContractType contractType,
			LPCTSTR version);
	Future<bool> activate(LPCTSTR licenseKey, LPCTSTR machineCode);
	Future<bool> check(LPCTSTR licenseKey, LPCTSTR machineCode);

private:
#ifdef UNICODE
	std::wstring baseUrl;
#else  /* !defined(UNICODE) */
	std::string baseUrl;
#endif /* UNICODE */
	Future<bool> post(LPCTSTR path, const char *data, size_t size);
};

#endif /* LICENSEMANAGER_H_ */
