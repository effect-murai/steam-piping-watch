/*
 * guid.cpp
 *
 *  Created on: 2020/10/24
 *      Author: Kiyoshi Tasaki <k.tasaki@effect-effect.com>
 */

#include "guid.h"

Guid::GuidError::GuidError(const char *message) {
	this->message = message;
}
Guid::GuidError::~GuidError() _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT {
}

Guid::Guid(const std::wstring &guid) {
	RPC_STATUS result = UuidFromStringW((RPC_WSTR) guid.c_str(), &this->guid);
	if (result != RPC_S_OK) {
		throw GuidError("The string UUID is invalid.");
	}
}

std::wstring Guid::toString() const {
	const GUID *data = &this->guid;
	RPC_WSTR guid;
	UuidToStringW((GUID*) data, &guid);
	std::wstring result((LPCWSTR) guid);
	RpcStringFreeW(&guid);
	return result;
}
