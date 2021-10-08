/*
 * guid.h
 *
 *  Created on: 2020/07/23
 *      Author: Kiyoshi Tasaki <k.tasaki@effect-effect.com>
 */

#ifndef GUID_H_
#define GUID_H_

#include <exception>
#include <string>
#include <windows.h>

class Guid {
public:
	class GuidError: public std::exception {
	public:
		GuidError(const char *message);
		virtual ~GuidError() _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT;

	private:
		std::string message;
	};

	Guid(const std::wstring &guid);

	inline const unsigned char* data() const {
		return (const unsigned char*) &guid;
	}

	inline const size_t size() const {
		return sizeof(GUID);
	}

	inline unsigned char operator[](size_t index) const {
		return data()[index];
	}

	std::wstring toString() const;

private:
	GUID guid;
};

#endif /* GUID_H_ */
