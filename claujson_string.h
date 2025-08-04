#pragma once

#include "claujson_internal.h"

namespace claujson {

	// sz`s type is uint32_t, not uint64_t.
	class alignas(32) String {
		friend class _Value;
	private: // do not change of order. do not add variable.
#define CLAUJSON_STRING_BUF_SIZE 11
		union {
			struct {
				char* str;
				uint32_t sz;
				_ValueType type; // STRING or SHORT_STRING or NOT_VALID ...
			};
			struct {
				char buf[CLAUJSON_STRING_BUF_SIZE];
				uint8_t buf_sz;
				_ValueType type_;
			};
		};
		Arena* pool = nullptr;
		uint8_t temp[8];
	public:
		static const uint64_t npos = -1;
	public:
		String& operator=(const String& other) = delete;

	protected:
		String(String&& other) noexcept {
			this->type = _ValueType::NONE;
			std::swap(this->str, other.str);
			std::swap(this->sz, other.sz);
			std::swap(this->type, other.type);
			std::swap(this->pool, other.pool);
		}

	public:

		explicit String(Arena* pool = nullptr) : pool(pool), type(_ValueType::NONE) {
			str = nullptr;
			sz = 0;
		}

		~String() {
			if (type == _ValueType::STRING && str && !pool) {
				delete[] str; 
			}
			else if (type == _ValueType::STRING && str) {
				pool->deallocate<char>(str, static_cast<uint64_t>(sz + 1));
			}
			str = nullptr;
			sz = 0;
			type = _ValueType::NONE;
		}


		String clone(Arena* pool) const {
			//std::cout << "called";
			if (is_valid() == false) { String temp; temp.type = _ValueType::NOT_VALID; return temp; }
			String obj;

			if (this->type == _ValueType::STRING) {
				obj.sz = this->sz;
				if (pool) {
					obj.str = (char*)pool->allocate<char>(sizeof(char) * (this->sz + 1));
				}
				else {
					obj.str = new (std::nothrow) char[this->sz + 1];
				}
				if (obj.str == nullptr) {
					log << warn << "new error";
					obj.type = _ValueType::ERROR;
					String result;
					result.type = _ValueType::ERROR;
					return result;
				}
				memcpy(obj.str, this->str, this->sz);
				obj.str[obj.sz] = '\0';
			}
			else if (this->type == _ValueType::SHORT_STRING) {
				obj.buf_sz = this->buf_sz;
				memcpy(obj.buf, this->buf, CLAUJSON_STRING_BUF_SIZE);
			}

			obj.type = this->type;

			return obj;
		}

		String& operator=(String&& other) noexcept {
			if (this->is_valid() == false || other.is_valid() == false || this == &other) { return *this; }
			std::swap(this->str, other.str);
			std::swap(this->sz, other.sz);
			std::swap(this->type, other.type);
			std::swap(this->pool, other.pool); // check!
			return *this;
		}

	private:
		explicit String(Arena* pool, const char* str) : pool(pool) {
			if (!str) { this->type = _ValueType::ERROR; return; }

			this->sz = Static_Cast<uint64_t, uint32_t>(strlen(str));
			if (this->sz < CLAUJSON_STRING_BUF_SIZE) {
				this->buf_sz = (uint8_t)this->sz;
				memcpy(this->buf, str, static_cast<uint64_t>(this->buf_sz));
				this->buf[(uint64_t)this->buf_sz] = '\0';
				this->type = _ValueType::SHORT_STRING;
			}
			else {
				if (pool) {
					this->str = (char*)pool->allocate<char>(sizeof(char)*(this->sz + 1));
				}
				else {
					this->str = new (std::nothrow) char[this->sz + 1];
				}
				
				if (this->str == nullptr) {
					log << warn << "new error";
					this->type = _ValueType::ERROR; return;
				}
				memcpy(this->str, str, this->sz);
				this->str[this->sz] = '\0';
				this->type = _ValueType::STRING;
			}
		}

		explicit String(Arena* pool, const char* str, uint32_t sz) : pool(pool) {
			if (!str) { this->type = _ValueType::ERROR; return; }

			this->sz = sz;
			if (this->sz < CLAUJSON_STRING_BUF_SIZE) {
				this->buf_sz = (uint8_t)this->sz;
				memcpy(this->buf, str, static_cast<uint64_t>(this->buf_sz));
				this->buf[(uint64_t)this->buf_sz] = '\0';
				this->type = _ValueType::SHORT_STRING;
			}
			else {
			//	this->buf_sz = 5; // (uint8_t)this->sz;
			//	memcpy(this->buf, str, static_cast<uint64_t>(this->buf_sz));
			//	this->buf[(uint64_t)this->buf_sz] = '\0';
			//	this->type = _ValueType::SHORT_STRING;
			//	return;
				if (pool) {
					this->str = (char*)pool->allocate<char>(sizeof(char) * (this->sz + 1));
				}
				else {
					this->str = new (std::nothrow) char[this->sz + 1];
				}
				
				if (this->str == nullptr) {
					this->type = _ValueType::ERROR;
					log << warn << "new error";
					return;
				}

				memcpy(this->str, str, this->sz);
				this->str[this->sz] = '\0';
				this->type = _ValueType::STRING;
			}
		}

	public:
		bool is_valid() const {
			return type != _ValueType::NOT_VALID && type != _ValueType::ERROR;
		}

		bool is_str() const {
			return type == _ValueType::STRING || type == _ValueType::SHORT_STRING;
		}

		char* data() {
			if (type == _ValueType::STRING) {
				return str;
			}
			else if (type == _ValueType::SHORT_STRING) {
				return buf;
			}
			else {
				return nullptr;
			}
		}

		const char* data() const {
			if (type == _ValueType::STRING) {
				return str;
			}
			else if (type == _ValueType::SHORT_STRING) {
				return buf;
			}
			else {
				return nullptr;
			}
		}
		uint64_t size() const {
			if (type == _ValueType::STRING) {
				return sz;
			}
			else if (type == _ValueType::SHORT_STRING) {
				return static_cast<uint64_t>(buf_sz);
			}
			else {
				return 0;
			}
		}

		// remove data.
		void clear() {
			if (type == _ValueType::STRING && str && !pool) {
				delete[] str;
			}
			else if (type == _ValueType::STRING && str) {
				pool->deallocate<char>(str, static_cast<uint64_t>(sz + 1));
			}
			sz = 0;
			str = nullptr;
			type = _ValueType::NONE;
		}

		bool operator<(const String& other) const {
			if (!this->is_valid() || !other.is_valid()) { return false; }
			return StringView(data(), size()) < StringView(other.data(), other.size());
		}
		bool operator<(const StringView other) const {
			if (!this->is_valid()) { return false; }
			return StringView(data(), size()) < other;
		}
		bool operator==(const StringView other) const {
			if (!this->is_valid()) { return false; }
			return StringView(data(), size()) == other;
		}

		bool operator==(const String& other) const {
			if (!this->is_valid() || !other.is_valid()) { return false; }
			return StringView(data(), size()) == StringView(other.data(), other.size());
		}

		std::string get_std_string(bool& fail) const {
			if (!is_str()) { fail = true; return std::string(); }
			fail = false;
			return std::string(data(), size());
		}
		StringView get_string_view(bool& fail) const {
			if (!is_str()) { fail = true; return StringView(); }
			fail = false;
			return StringView(data(), size());
		}

		uint64_t find(char ch, uint64_t start) const {
			const char* x = data();
			uint64_t sz = size();

			for (uint64_t i = start; i < sz; ++i) {
				if (x[i] == ch) {
					return i;
				}
			}
			return npos;
		}

		String substr(uint64_t start, uint64_t len) {
			return String(pool, data() + start, len);
		}
	private:
		// suppose str is valid utf-8 string!
		explicit String(Arena* pool, const std::string& str) : pool(pool) {
			if (str.size() <= CLAUJSON_STRING_BUF_SIZE) {
				memcpy(buf, str.data(), str.size());
				this->sz = Static_Cast<uint64_t, uint32_t>(str.size()); // chk..
				this->type = _ValueType::SHORT_STRING;
			}
			else {
				char* temp = nullptr;

				if (pool) {
					temp = (char*)pool->allocate<char>(sizeof(char) * (str.size() + 1));
				}
				else {
					temp = new (std::nothrow) char[str.size()];
				}

				if (!temp) {
					// log << warn ...
					this->type = _ValueType::NONE;
					return;
				}
				memcpy(temp, str.data(), str.size());
				this->str = temp;
				this->sz = Static_Cast<uint64_t, uint32_t>(str.size());
				this->type = _ValueType::STRING;
			}
		}
	};
}

