/*
 * LicenseInputWindow.cpp
 *
 *  Created on: 2020/11/05
 *      Author: Kiyoshi Tasaki <k.tasaki@effect-effect.com>
 */

#include "Protection.h"
#include "base64.h"
#include "registry.h"
#include "sha1.h"
#include <stdio.h>
#include "StringUtils.h"
#include <MainWindow.h>
#include <Window.h>
#include <EditBox.h>
#include <shlwapi.h>
#include "version.h"
#include <LicenseManager.h>
#include <SubWindows/ProgressDialog.h>

/**
 * インストールキーを取得する。
 */
static std::wstring getInstallKey() {
	try {
		return Registry(HKEY_LOCAL_MACHINE).get(L"SOFTWARE\\Yanai\\SolarCopter")[L"RegistrationCode"].asString();
	} catch (Registry::Error&) {
		return std::wstring();
	}
}

/**
 * インストールキーを登録する。
 */
bool registInstallKey(LPCTSTR licenseKey) {
	try {
		std::wstring installKey = licenseKey;
		Registry(HKEY_LOCAL_MACHINE).setString(L"SOFTWARE\\Yanai\\SolarCopter",
				L"RegistrationCode", installKey);
	} catch (Registry::Error&) {
		return false;
	}
	return true;
}

/**
 * インストールキーを削除する。
 */
void deleteInstallKey() {
	Registry(HKEY_LOCAL_MACHINE).deleteKey(L"SOFTWARE\\Yanai\\SolarCopter");
}

/**
 * インストールキーを確認する。
 */
bool checkInstallKey() {
	std::wstring installKey = getInstallKey();
	return !installKey.empty();
}

/**
 * ライセンス情報を確認する.
 */
bool checkLicense() {
	defineFutureRoutine(checkProc, bool,
			{
				std::wstring key = getInstallKeyInfo();
				std::wstring machineCode = getMachineGuid();
				LicenseManager licenseManager(LICENSE_SERVER_BASE_URL);
				return licenseManager.check(key.c_str(), machineCode.c_str()).get()
						|| licenseManager.check(key.c_str(),
								(machineCode + L"@" + getVersion()).c_str()).get();
			});
	Future<bool> checkResult(checkProc, NULL);
	ProgressDialog::show(checkResult);
	return checkResult.get();
}

/**
 * インストールキーを取得する。
 */
std::wstring getInstallKeyInfo() {
	std::wstring keyString;
	keyString = getInstallKey();

	return keyString;
}

/**
 * バージョン文字列を取得する。
 */
std::wstring getVersion() {
	LPTSTR wcsVersion = fromUTF8String(
	APP_STRINGIZE(APP_RELEASE_MAJOR) "."
	APP_STRINGIZE(APP_RELEASE_MINOR) "."
	APP_STRINGIZE(APP_RELEASE_TEENY));
	std::wstring version(wcsVersion);
	delete wcsVersion;
	return version;
}

/**
 * PCのGUIDを取得する。
 */
std::wstring getMachineGuid() {

	Guid guidData = Guid(
			Registry(HKEY_LOCAL_MACHINE).get(
					L"SOFTWARE\\Microsoft\\Cryptography")[L"MachineGuid"]);

	RPC_WSTR guidTemp;
	UuidToStringW((GUID*) &guidData, &guidTemp);

	std::wstring guidString;
	guidString = (WCHAR*) guidTemp;
	RpcStringFreeW(&guidTemp);

	return guidString;
}

/**
 * 管理者で実行されているかを確認する
 */
bool isAdministrator() {
	SID_IDENTIFIER_AUTHORITY authority = SECURITY_NT_AUTHORITY;
	PSID adminiGroup;
	BOOL isMember;

	if (!::AllocateAndInitializeSid(&authority, 2,
	SECURITY_BUILTIN_DOMAIN_RID,
	DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminiGroup))
		return FALSE;
	if (!::CheckTokenMembership(NULL, adminiGroup, &isMember)) {
		::FreeSid(adminiGroup);
		return FALSE;
	}
	::FreeSid(adminiGroup);
	return isMember;
}
