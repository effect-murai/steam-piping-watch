/*
 * LicenseManager.cpp
 *
 *  Created on: 2020/12/04
 *      Author: Kiyoshi TASAKI <k.tasaki@effect-effect.com>
 */

#include "LicenseManager.h"

#include "Common/Http/HttpClient.h"
#include "picojson.h"
#include <StringUtils.h>

#include "debug.h"

typedef struct {
	LPTSTR url;
	void *data;
	size_t size;
} PostData;

inline picojson::value toUTF8StringValue(LPCTSTR str) {
	char *utf8 = toUTF8String(str);
	picojson::value value(utf8);
	delete utf8;
	return value;
}

LicenseManager::LicenseManager(LPCTSTR baseUrl) {
	this->baseUrl = baseUrl;
}

Future<bool> LicenseManager::regist(LPCTSTR company, LPCTSTR unit, LPCTSTR name,
		LPCTSTR email, LPCTSTR machineCode, ContractType contractType,
		LPCTSTR version) {
	picojson::object reqObj;
	reqObj["company"] = toUTF8StringValue(company);
	reqObj["unit"] = toUTF8StringValue(unit);
	reqObj["name"] = toUTF8StringValue(name);
	reqObj["email"] = toUTF8StringValue(email);
	reqObj["contract"] = picojson::value((double) contractType);
	reqObj["machineCode"] = toUTF8StringValue(machineCode);
	reqObj["version"] = toUTF8StringValue(version);

	std::string req = picojson::value(reqObj).serialize(true);

	return post(TEXT("regist"), req.data(), req.size());
}

Future<bool> LicenseManager::activate(LPCTSTR licenseKey, LPCTSTR machineCode) {
	picojson::object reqObj;
	reqObj["key"] = toUTF8StringValue(licenseKey);
	reqObj["associationText"] = toUTF8StringValue(machineCode);
	std::string req = picojson::value(reqObj).serialize(true);

	return post(TEXT("license/activate"), req.data(), req.size());
}

Future<bool> LicenseManager::check(LPCTSTR licenseKey, LPCTSTR machineCode) {
	picojson::object reqObj;
	reqObj["key"] = toUTF8StringValue(licenseKey);
	reqObj["associationText"] = toUTF8StringValue(machineCode);
	std::string req = picojson::value(reqObj).serialize(true);

	return post(TEXT("license/check"), req.data(), req.size());
}

Future<bool> LicenseManager::post(LPCTSTR path, const char *data, size_t size) {
	PostData *postData = (PostData*) malloc(sizeof(PostData));
#ifdef UNICODE
	postData->url = wcsdup((baseUrl + path).c_str());
#else
	postData->url = strdup((baseUrl + path).c_str());
#endif
	postData->data = malloc(size);
	memcpy(postData->data, data, size);
	postData->size = size;
	defineFutureRoutine(regist, bool, {
		PostData *postData = (PostData* )args;
		HttpClient client;
		HttpClient::HttpResponse response = client.post(postData->url, postData->data, postData->size);
		free(postData->url)
		;
		free(postData->data)
		;
		free(postData)
		;
		if (!response) {
			return false;
		}debug("Status:%d\n%S\n----------\n%s\n", response.status(),
				response.header().c_str(), (char *)response.data());
		picojson::value value
		;
		picojson::parse(value, (char* )response.data())
		;
		return value.contains("success")
				&& value.get("success").evaluate_as_boolean();
	});
	return Future<bool>(regist, postData);
}
