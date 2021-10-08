/*
 * HttpClient.h
 *
 *  Created on: 2020/12/01
 *      Author: Kiyoshi TASAKI <k.tasaki@effect-effect.com>
 */

#ifndef HTTPCLIENT_H_
#define HTTPCLIENT_H_

#include <stddef.h>
#include <windows.h>
#include <winhttp.h>

#include <string>

/**
 * HTTPクライアント.
 */
class HttpClient {
public:
#ifdef UNICODE
	typedef std::wstring String;
#else /* !defined(UNICODE) */
	typedef std::string String;
#endif /* UNICODE */
	class HttpResponse;

public:
	/**
	 * 新しいHTTPクライアントのインスタンスを生成します.
	 */
	HttpClient();
	~HttpClient();

	operator bool();

	void open();

	HttpResponse post(const String &path, const void *data, size_t size);

	class HttpResponse {
	public:
		HttpResponse(HINTERNET handle);

		inline void* data() const {
			return _data;
		}
		inline size_t size() const {
			return _size;
		}
		inline const String& header() const {
			return _header;
		}
		inline int status() const {
			return statusCode;
		}

		operator bool();

		bool operator!();

	private:
		int statusCode;

		String _header;

		void *_data;
		size_t _size;
	};

private:
	HINTERNET hSession;
};

#endif /* HTTPCLIENT_H_ */
