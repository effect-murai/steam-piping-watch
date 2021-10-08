/*
 * HttpClient.cpp
 *
 *  Created on: 2020/12/01
 *      Author: Kiyoshi TASAKI <k.tasaki@effect-effect.com>
 */

#include "Common/Http/HttpClient.h"

#include <stdio.h>
#include <StringUtils.h>
#include <windows.h>

#include "def.h"
#include "url/url_parser.h"

#include "debug.h"

HttpClient::HttpClient() {
	hSession = NULL;

	// WinHTTP をサポートしているか確認
	if (!WinHttpCheckPlatform()) {
		debug("WinHTTP not supprted.\n");
		return;
	}

	// セッションハンドルを取得
	hSession = WinHttpOpen(L"WinHttp/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
	WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
}

HttpClient::~HttpClient() {
	if (hSession != NULL) {
		WinHttpCloseHandle(hSession);
	}
}

HttpClient::HttpResponse HttpClient::post(const String &url, const void *data,
		size_t size) {
	if (hSession == NULL) {
		// 無効なセッションハンドル。
		debug("Invalid Session Handler\n");
		return NULL;
	}

	// URLを解析する。
	struct parsed_url *parsed;
#ifdef UNICODE
	char *mbUrl = toUTF8String(url.c_str());
	parsed = parse_url(mbUrl);
	delete mbUrl;
#else
	parsed = parse_url(url.c_str());
#endif
	if (parsed == NULL) {
		debug("Invalid URL\n");
		return NULL;
	}

	// URLからプロトコルを取得する。
	bool https = stricmp(parsed->scheme, "https") == 0;

	// URLからポート番号を取得する。
	int port = 0;
	if (parsed->port != NULL) {
		port = atoi(parsed->port);
	} else if (https) {
		port = 443;
	} else {
		port = 80;
	}

	// HTTP サーバの指定
	wchar_t *host = fromUTF8String(parsed->host);
	HINTERNET hConnect = WinHttpConnect(hSession, host, port, 0);
	delete host;
	if (hConnect == NULL) {
		debug("WinHttpConnect: Error\n");
		parsed_url_free(parsed);
		return NULL;
	}

	// HTTP リクエストハンドルを作成
	wchar_t *path = fromUTF8String(parsed->path);
	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path, NULL,
	WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
			https ? WINHTTP_FLAG_SECURE : 0);
	delete path;
	if (hRequest == NULL) {
		debug("WinHttpOpenRequest: Error\n");
		parsed_url_free(parsed);
		WinHttpCloseHandle(hConnect);
		hConnect = NULL;
		return NULL;
	}

	if (!WinHttpAddRequestHeaders(hRequest,
			L"Content-Type: application/json; charset=utf-8", -1L,
			WINHTTP_ADDREQ_FLAG_ADD)) {
		debug("WinHttpAddRequestHeaders: Error\n");
		parsed_url_free(parsed);
		WinHttpCloseHandle(hConnect);
		hConnect = NULL;
		return NULL;
	}

	parsed_url_free(parsed);

	// Send a request.
	if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
	WINHTTP_NO_REQUEST_DATA, 0, size, 0)) {
		debug("WinHttpSendRequest: Error\n");
		WinHttpCloseHandle(hRequest);
		hRequest = NULL;
		WinHttpCloseHandle(hConnect);
		hConnect = NULL;
		return NULL;
	}

	// Write data to the server.
	DWORD dwBytesWritten;
	if (!WinHttpWriteData(hRequest, data, size, &dwBytesWritten)) {
		debug("WinHttpSendRequest: Error\n");
		WinHttpCloseHandle(hRequest);
		hRequest = NULL;
		WinHttpCloseHandle(hConnect);
		hConnect = NULL;
		return NULL;
	}

	// リクエスト終了
	if (!WinHttpReceiveResponse(hRequest, NULL)) {
		debug("WinHttpReceiveResponse: Error\n");
		WinHttpCloseHandle(hRequest);
		hRequest = NULL;
		WinHttpCloseHandle(hConnect);
		hConnect = NULL;
		return NULL;
	}

	HttpResponse request(hRequest);
	WinHttpCloseHandle(hRequest);
	hRequest = NULL;
	WinHttpCloseHandle(hConnect);
	hConnect = NULL;

	return request;
}

HttpClient::operator bool() {
	return hSession != NULL;
}

HttpClient::HttpResponse::HttpResponse(HINTERNET handle) {
	DWORD dwBuffSize = sizeof(WCHAR) * (256 + 1);

	//
	// Read all the headers.
	//
	if (!WinHttpQueryHeaders(handle, WINHTTP_QUERY_RAW_HEADERS_CRLF,
	WINHTTP_HEADER_NAME_BY_INDEX, NULL, &dwBuffSize,
	WINHTTP_NO_HEADER_INDEX)) {
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
			debug("Unexpected Error\n");
			return;
		}
	}

	// メモリの割り当て
	WCHAR *header = new WCHAR[(dwBuffSize + 1) / sizeof(WCHAR)];
	if (header == NULL) {
		debug("Couldn't allocate memory\n");
		return;
	}

	// ヘッダ値を取得
	if (!WinHttpQueryHeaders(handle, WINHTTP_QUERY_RAW_HEADERS_CRLF,
	WINHTTP_HEADER_NAME_BY_INDEX, header, &dwBuffSize,
	WINHTTP_NO_HEADER_INDEX)) {
		debug("WinHttpQueryHeaders Failed (%lu).\n", GetLastError());
	} else {
		scanString(header, TEXT("HTTP/1.1 %d"), &statusCode);
#ifdef UNICODE
		this->_header = header;
#else
		char *utf8Header = toUTF8String(header);
		this->header = utf8Header;
		delete utf8Header;
#endif
	}
	delete header;

	//
	// Read HTTP Body
	//
	DWORD availableSize = 16;
	char *buffer = NULL;
	do {
		// 利用可能なデータがあるかチェックする
		if (!WinHttpQueryDataAvailable(handle, &availableSize)) {
			debug("Error %lu in WinHttpQueryDataAvailable.\n", GetLastError());
		}

		if (availableSize == 0) {
			break;
		}

		// バッファの割り当て
		buffer = (char*) malloc(availableSize + 1);

		if (buffer == NULL) {
			debug("Out of memory\n");
			break;
		} else {
			DWORD downloadedSize = 0;

			// Read the data.
			ZeroMemory(buffer, availableSize + 1);

			while (availableSize > downloadedSize) {
				if (!WinHttpReadData(handle, buffer + downloadedSize,
						availableSize, &downloadedSize)) {
					debug("Error %lu in WinHttpReadData.\n", GetLastError());
					free(buffer);
					buffer = NULL;
					break;
				}
			}
			break;
		}
	} while (availableSize > 0);
	_data = buffer;
	_size = availableSize;
}

bool HttpClient::HttpResponse::operator!() {
	return !(bool) *this;
}

HttpClient::HttpResponse::operator bool() {
	return (statusCode / 100) == 2;
}
