/*
 * registry.cpp
 *
 *  Created on: 2020/07/24
 *      Author: Kiyoshi Tasaki <k.tasaki@effect-effect.com>
 */

#include "registry.h"

#include <stdio.h>
#include <windows.h>

#include <string>

#if !defined(__cplusplus) || __cplusplus < 201103L
#define nullptr (NULL)
#endif

DWORD Registry::getDWord(const std::wstring &subKey,
		const std::wstring &value) const {
	DWORD data;
	DWORD dataSize = sizeof(data);

	LONG retCode = ::RegGetValueW(hKey, subKey.c_str(), value.c_str(),
	RRF_RT_REG_DWORD, nullptr, &data, &dataSize);

	if (retCode != ERROR_SUCCESS) {
		throw Error("Cannot read DWORD from registry.", retCode);
	}

	return data;
}

void Registry::setDWord(const std::wstring &subKey, const std::wstring &name,
		const DWORD value) {
	HKEY hRegKey = NULL;
	DWORD dwDisp = 0;

	LONG retCode = RegCreateKeyExW(hKey, subKey.c_str(), 0, (LPWSTR) L"",
	REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
	NULL, &hRegKey, &dwDisp);
	if (retCode != ERROR_SUCCESS) {
		throw Error("Cannot create a new registry key.", retCode);
	}
	retCode = RegSetValueExW(hRegKey, name.c_str(), 0, REG_DWORD,
			(LPBYTE) &value, sizeof(DWORD));
	RegCloseKey(hRegKey);
	if (retCode != ERROR_SUCCESS) {
		throw Error("Cannot write DWORD to registry.", retCode);
	}
}

std::wstring Registry::getString(const std::wstring &subKey,
		const std::wstring &value) const {
	DWORD dataSize = 0;
	LONG retCode = ::RegGetValueW(hKey, subKey.c_str(), value.c_str(),
	RRF_RT_REG_SZ, nullptr, nullptr, &dataSize);

	if (retCode != ERROR_SUCCESS) {
		throw Error("Cannot read string from registry", retCode);
	}

	std::wstring data;
	data.resize(dataSize / sizeof(wchar_t));

	retCode = ::RegGetValueW(hKey, subKey.c_str(), value.c_str(), RRF_RT_REG_SZ,
	nullptr, &data[0], &dataSize);

	if (retCode != ERROR_SUCCESS) {
		throw Error("Cannot read string from registry", retCode);
	}

	DWORD stringLength = dataSize / sizeof(wchar_t) - 1;
	data.resize(stringLength);

	return data;
}

void Registry::setString(const std::wstring &subKey, const std::wstring &name,
		const std::wstring &value) {
	HKEY hRegKey = NULL;
	DWORD dwDisp = 0;

	LONG retCode = RegCreateKeyExW(hKey, subKey.c_str(), 0, (LPWSTR) L"",
	REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
	NULL, &hRegKey, &dwDisp);
	if (retCode != ERROR_SUCCESS) {
		throw Error("Cannot create a new registry key.", retCode);
	}
	retCode = RegSetValueExW(hRegKey, name.c_str(), 0, REG_SZ,
			(LPBYTE) value.c_str(), value.size() * sizeof(wchar_t));
	RegCloseKey(hRegKey);
	if (retCode != ERROR_SUCCESS) {
		throw Error("Cannot write DWORD to registry.", retCode);
	}
}

std::vector<std::pair<std::wstring, DWORD> > Registry::enumValues(
		const std::wstring &subKey) {
	HKEY hKeyResult;
	LONG retCode = ::RegOpenKeyW(hKey, subKey.data(), &hKeyResult);

	DWORD valueCount;
	DWORD maxValueNameLen;
	retCode = ::RegQueryInfoKeyW(hKeyResult,
	nullptr, // No user-defined class
			nullptr, // No user-defined class size
			nullptr, // Reserved
			nullptr, // No subkey count
			nullptr, // No subkey max length
			nullptr, // No subkey class length
			&valueCount, &maxValueNameLen,
			nullptr, // No max value length
			nullptr, // No security descriptor
			nullptr  // No last write time
			);

	if (retCode != ERROR_SUCCESS) {
		::RegCloseKey(hKeyResult);
		throw Error("Cannot query key info from the registry", retCode);
	}
	maxValueNameLen++;

	wchar_t nameBuffer[maxValueNameLen];

	std::vector<std::pair<std::wstring, DWORD> > values;

	for (DWORD index = 0; index < valueCount; index++) {
		// Call RegEnumValue to get data of current value ...
		DWORD valueNameLen = maxValueNameLen;
		DWORD valueType;
		retCode = ::RegEnumValueW(hKeyResult, index, nameBuffer, &valueNameLen,
		nullptr, // Reserved
				&valueType,
				nullptr, // Not interested in data
				nullptr  // Not interested in data size
				);

		values.push_back(
				std::make_pair(std::wstring(nameBuffer, valueNameLen),
						valueType));
	}

	::RegCloseKey(hKeyResult);
	return values;
}

void Registry::deleteKey(const std::wstring &subKey) {
	RegDeleteKeyW(hKey, subKey.c_str());
}
