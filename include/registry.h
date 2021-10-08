/*
 * registry.h
 *
 *  Created on: 2020/07/22
 *      Author: Kiyoshi Tasaki <k.tasaki@effect-effect.com>
 */

#ifndef REGISTRY_H_
#define REGISTRY_H_

#include <exception>
#include <string>
#include <vector>
#include <windows.h>

class Registry {
public:
	class Error: public std::exception {
	public:
		Error(const char *message, LONG errorCode) {
			this->errorCode = errorCode;
			this->message = message;
		}

		virtual const char*
		what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT {
			return message;
		}

	private:
		const char *message;
		LONG errorCode;
	};

	class Key {
	public:
		class Value {
		public:
			Value(Key *key, const std::wstring &value) :
					value(value) {
				this->key = key;
			}

			inline DWORD asDWord() {
				return key->registry->getDWord(key->subKey, value);
			}

			inline std::wstring asString() {
				return key->registry->getString(key->subKey, value);
			}

			inline operator DWORD() {
				return asDWord();
			}

			inline operator std::wstring() {
				return asString();
			}

		private:
			Key *key;
			std::wstring value;
		};

		Key(Registry *registry, const std::wstring &subKey) :
				subKey(subKey) {
			this->registry = registry;
		}

		inline Value get(const std::wstring &value) {
			return Value(this, value);
		}

		inline Value operator[](const std::wstring &value) {
			return Value(this, value);
		}

	private:
		Registry *registry;
		std::wstring subKey;
	};

	Registry(HKEY hKey) {
		this->hKey = hKey;
	}

private:
	HKEY hKey;

public:
	inline Key get(const std::wstring &subKey) {
		return Key(this, subKey);
	}

	inline Key operator[](const std::wstring &subKey) {
		return Key(this, subKey);
	}

	DWORD getDWord(const std::wstring &subKey, const std::wstring &value) const;

	void setDWord(const std::wstring &subKey, const std::wstring &name,
			const DWORD value);

	std::wstring getString(const std::wstring &subKey,
			const std::wstring &value) const;

	void setString(const std::wstring &subKey, const std::wstring &name,
			const std::wstring &value);

	std::vector<std::pair<std::wstring, DWORD> >
	enumValues(const std::wstring &subKey);

	void deleteKey(const std::wstring &subKey);
};

#endif /* REGISTRY_H_ */
