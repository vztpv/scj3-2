#pragma once

#include <iostream>
#include <memory>
#include <map>
#include <vector>
#include <list>
#include <string>
#include <set>
#include <fstream>
#include <cstring>
#include <cstdint> // uint64_t? int64_t?


template <class From, class To>
inline To Static_Cast(From x) {
	To temp = static_cast<To>(x);
	bool valid = static_cast<From>(temp) == x;
	if (!valid) {
		throw std::runtime_error("static cast error");
	}
	return temp;
}



#if __cpp_lib_string_view
#include <string_view>
using namespace std::literals::string_view_literals;
namespace claujson {
	using StringView = std::string_view;
}

#else

namespace claujson {
	class StringView {
	public:
		explicit StringView() : m_str(nullptr), m_len(0) {}

		StringView(const std::string& str) : m_str(str.data()), m_len(str.size()) {}
		explicit StringView(const char* str) : m_str(str) { m_len = strlen(str); }
		explicit StringView(const char* str, uint64_t len) : m_str(str), m_len(len) {}
		StringView(const StringView& other) {
			m_str = other.m_str;
			m_len = other.m_len;
		}

	public:
		const char* data() const {
			return m_str;
		}

		uint64_t size() const {
			return m_len;
		}

		uint64_t length() const {
			return m_len;
		}

		bool empty() const {
			return 0 == m_len;
		}

		StringView substr(uint64_t pos, uint64_t n) const {
			return StringView(m_str + pos, n);
		}

		const char& operator[](uint64_t idx) const {
			return m_str[idx];
		}

		// returns index;
		uint64_t find(const char ch, uint64_t start = 0) {
			for (uint64_t i = start; i < size(); ++i) {
				if (ch == (*this)[i]) {
					return i;
				}
			}
			return npos;
		}

		StringView& operator=(const StringView& other) {
			StringView temp(other);
			this->m_str = temp.m_str;
			this->m_len = temp.m_len;
			return *this;
		}
	private:
		const char* m_str;
		uint64_t m_len;
	public:
		static const uint64_t npos;

		friend std::ostream& operator<<(std::ostream& stream, const claujson::StringView& sv) {
			stream << sv.data();
			return stream;
		}

		bool operator==(const StringView view) {
			return this->compare(view) == 0;
		}

		bool operator!=(const StringView view) {
			return this->compare(view) != 0;
		}

		int compare(const StringView view) {
			uint64_t idx1 = 0, idx2 = 0;
			for (; idx1 < this->length() && idx2 < view.length(); ++idx1, ++idx2) {
				uint8_t diff = this->data()[idx1] - view.data()[idx2];
				if (diff < 0) {
					return -1;
				}
				else if (diff > 0) {
					return 1;
				}
			}
			if (idx1 < this->length()) {
				return 1;
			}
			else if (idx2 < view.length()) {
				return -1;
			}
			return 0;
		}

		bool operator<(const StringView view) {
			return this->compare(view) < 0;
		}
	};
}

claujson::StringView operator""sv(const char* str, uint64_t sz);
bool operator==(const std::string& str, claujson::StringView sv);



#endif



namespace claujson {

	// has static buf?
	template <class T, int SIZE = 1024>
	class Vector {
	public:
		class iterator {
		private:
			T* ptr;
		public:
			iterator(T* now) {
				ptr = now;
			}
		public:
			void operator++() {
				++ptr;
			}
			T* operator->() {
				return ptr;
			}
			const T* operator->() const {
				return ptr;
			}
			T& operator*() {
				return *ptr;
			}
			const T& operator*() const {
				return *ptr;
			}
			bool operator!=(const iterator& other) const {
				return this->ptr != other.ptr;
			}
		};
		class const_iterator {
		private:
			T* ptr;
		public:
			const_iterator(T* now) {
				ptr = now;
			}
		public:
			const T* operator->() const {
				return ptr;
			}
			const T& operator*() const {
				return *ptr;
			}
			bool operator!=(const iterator& other) const {
				return this->ptr != other.ptr;
			}
		};
	private:
		T buf[SIZE + 1];
		T* ptr = nullptr;
		uint32_t capacity = SIZE;
		uint32_t sz = 0;
		int type = 0;
	public:
		Vector() {
			//
		}
		~Vector() {
			if (type == 1 && ptr) {
				delete[] ptr;
				ptr = nullptr;
				sz = 0;
				type = 0;
			}
		}
		Vector(const Vector&) = delete;
		Vector& operator=(const Vector&) = delete;
		Vector(Vector&& other) noexcept {
			std::swap(ptr, other.ptr);
			std::swap(capacity, other.capacity);
			std::swap(sz, other.sz);
			std::swap(type, other.type);
			memcpy(buf, other.buf, SIZE * sizeof(T));
		}
		Vector& operator=(Vector&& other) noexcept {
			Vector temp(std::move(other));

			std::swap(this->ptr, temp.ptr);
			std::swap(this->capacity, temp.capacity);
			std::swap(this->sz, temp.sz);
			std::swap(this->type, temp.type);
			memcpy(this->buf, temp.buf, SIZE * sizeof(T));

			return *this;
		}
	public:
		iterator begin() {
			return iterator(type == 1 ? ptr : buf);
		}
		const_iterator begin() const {
			return iterator(type == 1 ? ptr : buf);
		}
		iterator end() {
			return iterator(type == 1 ? ptr + sz : buf + SIZE);
		}
		const_iterator end() const {
			return iterator(type == 1 ? ptr + sz : buf + SIZE);
		}
	public:
		void clear() {
			if (type == 1 && ptr) {
				delete[] ptr;
				ptr = nullptr;
			}
			capacity = SIZE;
			sz = 0;
			type = 0;
		}

		T& back() {
			if (type == 0) {
				return buf[sz - 1];
			}
			return ptr[sz - 1];
		}
		const T& back() const {
			if (type == 0) {
				return buf[sz - 1];
			}
			return ptr[sz - 1];
		}
		void pop_back() {
			if (empty() == false) {
				--sz;
			}
		}
		bool empty() const { return 0 == sz; }
		uint64_t size() const { return sz; }
		void push_back(T val) {
			if (type == 0) {
				buf[sz] = std::move(val);
				++sz;
				if (sz == SIZE) {
					if (ptr) {
						delete[] ptr; ptr = nullptr;
					}
					ptr = new (std::nothrow) T[SIZE * 2];
					if (!ptr) {
						throw ("new failed");
					}
					capacity = SIZE * 2;
					memcpy(ptr, buf, SIZE * sizeof(T));
					type = 1;
				}
			}
			else {
				if (sz < capacity) {
					ptr[sz] = std::move(val);
					++sz;
				}
				else {
					T* temp = new (std::nothrow) T[2 * capacity];
					if (!temp) {
						throw ("new failed");
					}
					memcpy(temp, ptr, sz * sizeof(T));
					capacity = capacity * 2;
					delete[] ptr;
					ptr = temp;
					ptr[sz] = std::move(val);
					++sz;
				}
			}
		}
	public:
		T& operator[](const uint64_t idx) {
			if (type == 0) {
				return buf[idx];
			}
			return ptr[idx];
		}
		const T& operator[](const uint64_t idx) const {
			if (type == 0) {
				return buf[idx];
			}
			return ptr[idx];
		}

	};

	class Log;

	template <class T>
	static void _print(Log& log, const T& val, const int op);

	class Log {
	public:
		class Info {
		public:
			friend std::ostream& operator<<(std::ostream& stream, const Info&) {
				stream << "[INFO] ";
				return stream;
			}
		};
		class Warning {
		public:
			friend std::ostream& operator<<(std::ostream& stream, const Warning&) {
				stream << "[WARN] ";
				return stream;
			}
		};

		enum class Option { CONSOLE, FILE, CONSOLE_AND_FILE, NO_PRINT };
		class Option2 {
		public:
			static const int INFO = 1;
			static const int WARN = 2;
			static const int CLEAR = 0;
		};
	private:
		Option opt; // console, file, ...
		int opt2; // info, warn, ...
		int state; // 1 : info, 2 : warn. // default is info!
		std::string fileName;
	public:

		Log() : state(0), opt(Option::NO_PRINT), opt2(Option2::CLEAR), fileName("log.txt") {
			//
		}

	public:
		template <class T>
		friend void _print(Log& log, const T& val, const int op);

	public:

		Option option() const {
			return opt;
		}

		int option2() const {
			return opt2;
		}

		void console() {
			opt = Option::CONSOLE;
		}

		void file() {
			opt = Option::FILE;
		}

		void console_and_file() {
			opt = Option::CONSOLE_AND_FILE;
		}

		void no_print() {
			opt = Option::NO_PRINT;
			opt2 = Option2::CLEAR;
		}

		void file_name(const std::string& str) {
			fileName = str;
		}

		void info(bool only = false) {
			if (only) {
				opt2 = Option2::INFO;
			}
			else {
				opt2 = opt2 | Option2::INFO;
			}
		}
		void warn(bool only = false) {
			if (only) {
				opt2 = Option2::WARN;
			}
			else {
				opt2 = opt2 | Option2::WARN;
			}
		}
	};

	template <class T>
	static void _print(Log& log, const T& val, const int op) { // op : change_state, with op.

		if (op == 0 || op == 1) {
			log.state = op;
		}

		if (log.opt == Log::Option::CONSOLE || log.opt == Log::Option::CONSOLE_AND_FILE) {

			int count = 0;

			if ((log.opt2 & Log::Option2::INFO) && log.state == 0) {
				count = 1;
			}
			if ((log.opt2 & Log::Option2::WARN) && log.state == 1) {
				count = 1;
			}

			if (count) {
				std::cout << val;
			}
		}

		if (log.opt == Log::Option::FILE || log.opt == Log::Option::CONSOLE_AND_FILE) {
			std::ofstream outFile;
			outFile.open(log.fileName, std::ios::app);
			if (outFile) {
				int count = 0;

				if ((log.opt2 & Log::Option2::INFO) && log.state == 0) {
					count = 1;
				}
				if ((log.opt2 & Log::Option2::WARN) && log.state == 1) {
					count = 1;
				}

				if (count) {
					outFile << val;
				}
				outFile.close();
			}
		}
	}

	template <class T>
	inline Log& operator<<(Log& log, const T& val) {
		_print(log, val, -1);
		return log;
	}

	template<>
	inline Log& operator<<(Log& log, const Log::Info& x) {
		_print(log, x, 0);
		return log;
	}
	template<>
	inline Log& operator<<(Log& log, const Log::Warning& x) {
		_print(log, x, 1);
		return log;
	}

	extern Log::Info info;
	extern Log::Warning warn;
	extern Log log; // no static..
	// inline Error error;

	template <class T>
	using PtrWeak = T*;

	template <class T>
	using Ptr = std::unique_ptr<T>;
	// Ptr - use std::move


	enum class _ValueType : int32_t {
		NONE = 0, // chk 
		ARRAY, // ARRAY_OBJECT -> ARRAY, OBJECT
		OBJECT,
		PARTIAL_JSON,
		INT, UINT,
		FLOAT,
		BOOL,
		NULL_,
		STRING, SHORT_STRING,
		NOT_VALID,
		ERROR // private class?
	};

	template <class Key, class Data>
	class Pair {
	public:
		Key first = Key();
		Data second = Data();
	public:
		Pair() {}
		Pair(Key&& first, Data&& second) : first(std::move(first)), second(std::move(second)) {}
		Pair(const Key& first, Data&& second) : first((first)), second(std::move(second)) {}
		Pair(Key&& first, const Data& second) : first(std::move(first)), second((second)) {}
	};

	// memory_pool?
	class Arena {
		struct alignas(16) Node16 {
			uint64_t size;
			Node16* next;
		};
		struct alignas(32) Node32 {
			uint64_t size;
			Node32* next;
		};
		struct alignas(64) Node64 {
			uint64_t size;
			Node64* next;
		};

		struct Block {
			Block* next;
			uint64_t capacity;
			uint64_t offset;
			uint8_t* data;
			Node16* free_list16;
			Node32* free_list32;
			Node64* free_list64;

			Block(uint64_t cap)
				: next(nullptr), capacity(cap), offset(0), free_list16(nullptr),
				free_list32(nullptr), free_list64(nullptr) {
				//data = (uint8_t*)mi_malloc(sizeof(uint8_t) * capacity); // 
				data = new (std::nothrow) uint8_t[capacity];
			}

			~Block() {
				delete[] data;
			//	mi_free(data);
			}
		};

		Block* head[4];
		Block* rear[4];
		uint64_t defaultBlockSize;
	public:
		Arena(uint64_t initialSize = 1024 * 512 + 64)
			: defaultBlockSize(initialSize) {
			for (int i = 0; i < 4; ++i) {
				head[i] = (new (std::nothrow) Block(initialSize));
				rear[i] = head[i];
			}
		}

		Arena(const Arena&) = delete;
		Arena& operator=(const Arena&) = delete;

		static int64_t counter;
	private:
		
		// _Value
		template <class T>
		T* allocate16(uint64_t size) {
			Block* block = head[1];

			while (block) {
				if (block->free_list16) {
					Node16* before = nullptr;
					Node16* temp = block->free_list16;
					Node16* next = temp->next;
					while (temp) {
						next = temp->next;
						if (size <= temp->size) {
							T* ptr = reinterpret_cast<T*>(temp);
							if (size < temp->size) {
								uint64_t diff = temp->size - size;
								Node16* temp2 = temp + size / 16;
								temp2->size = diff / 16;
								temp2->next = next;
								if (!before) {
									block->free_list16 = temp2;
								}
								else {
									before->next = temp2;
								}
							}
							else {
								if (!before) {
									block->free_list16 = next;
								}
								else {
									before->next = temp->next;
								}
							}
							return ptr;
						}
						before = temp;
						temp = temp->next;
					}
				}

				if (block->offset + size <= block->capacity) {
					void* ptr = block->data + block->offset;
					uint64_t remain = block->capacity - size;
					if (!std::align(alignof(T), size, ptr, remain)) {
						block = block->next;
						continue;
					}
					uint64_t diff = ((uint8_t*)ptr - block->data);
					block->offset = diff + size;
					return reinterpret_cast<T*>(ptr);
				}
				block = block->next;
			}

			// allocate new block
			uint64_t newCap = std::max(defaultBlockSize, size + 64);
			Block* newBlock = new (std::nothrow) Block(newCap);
			if (!newBlock) {
				return nullptr;
			}
			counter++;
			uint64_t remain = newCap - newBlock->offset;
			void* ptr = newBlock->data + newBlock->offset;
			if (!std::align(alignof(T), size, ptr, remain)) {
				delete newBlock;
				return nullptr;
			}
			uint64_t diff = ((uint8_t*)ptr - newBlock->data);
			newBlock->offset = diff + size;

			newBlock->next = head[1];
			head[1] = newBlock;

			return reinterpret_cast<T*>(ptr);
		}
		
		template <class T>
		void deallocate16(T* ptr, uint64_t len) {
			uint64_t size = sizeof(T) * len;
			Block* block = head[1];
			while (block) {
				if ((uint8_t*)block->data <= (uint8_t*)ptr &&
					(uint8_t*)(ptr) + sizeof(T) * len <= (uint8_t*)(block->data) + block->capacity) {
					if ((uint8_t*)(ptr) + sizeof(T) * len == (uint8_t*)(block->data) + block->offset) {
						block->offset = (uint8_t*)ptr - (uint8_t*)block->data;
						//std::cout << "real_deallocated\n"; //
					}
					else {
						if (block->free_list16) {
							Node16* temp = block->free_list16->next;
							block->free_list16 = reinterpret_cast<Node16*>(ptr);
							block->free_list16->size = sizeof(T) * len;
							block->free_list16->next = temp;
						}
						else {
							block->free_list16 = reinterpret_cast<Node16*>(ptr);
							block->free_list16->size = sizeof(T) * len;
							block->free_list16->next = nullptr;
						}
					}
					return;
				}

				block = block->next;
			}

		}
		
		template <class T>
		void deallocate32(T* ptr, uint64_t len) {
			uint64_t size = sizeof(T) * len;
			Block* block = head[2];
			while (block) {
				if ((uint8_t*)block->data <= (uint8_t*)ptr &&
					(uint8_t*)(ptr) + sizeof(T) * len <= (uint8_t*)(block->data) + block->capacity) {
					if ((uint8_t*)(ptr) + sizeof(T) * len == (uint8_t*)(block->data) + block->offset) {
						block->offset = (uint8_t*)ptr - (uint8_t*)block->data;
						//std::cout << "real_deallocated\n"; //
					}
					else {
						if (block->free_list32) {
							Node32* temp = block->free_list32->next;
							block->free_list32 = reinterpret_cast<Node32*>(ptr);
							block->free_list32->size = sizeof(T) * len;
							block->free_list32->next = temp;
						}
						else {
							block->free_list32 = reinterpret_cast<Node32*>(ptr);
							block->free_list32->size = sizeof(T) * len;
							block->free_list32->next = nullptr;
						}
					}
					return;
				}

				block = block->next;
			}

		}
		template <class T>
		void deallocate64(T* ptr, uint64_t len) {
			uint64_t size = sizeof(T) * len;
			Block* block = head[3];
			while (block) {
				if ((uint8_t*)block->data <= (uint8_t*)ptr &&
					(uint8_t*)(ptr)+sizeof(T) * len <= (uint8_t*)(block->data) + block->capacity) {
					if ((uint8_t*)(ptr)+sizeof(T) * len == (uint8_t*)(block->data) + block->offset) {
						block->offset = (uint8_t*)ptr - (uint8_t*)block->data;
						//std::cout << "real_deallocated\n"; //
					}
					else {
						if (block->free_list64) {
							Node64* temp = block->free_list64->next;
							block->free_list64 = reinterpret_cast<Node64*>(ptr);
							block->free_list64->size = sizeof(T) * len;
							block->free_list64->next = temp;
						}
						else {
							block->free_list64 = reinterpret_cast<Node64*>(ptr);
							block->free_list64->size = sizeof(T) * len;
							block->free_list64->next = nullptr;
						}
					}
					return;
				}

				block = block->next;
			}

		}
		// String
		template <class T>
		T* allocate32(uint64_t size) {
			Block* block = head[2];

			while (block) {
				if (block->free_list32) {
					Node32* before = nullptr;
					Node32* temp = block->free_list32;
					Node32* next = temp->next;
					while (temp) {
						next = temp->next;
						if (size <= temp->size) {
							T* ptr = reinterpret_cast<T*>(temp);
							if (size < temp->size) {
								uint64_t diff = temp->size - size;
								Node32* temp2 = temp + size / 32;
								temp2->size = diff / 32;
								temp2->next = next;
								if (!before) {
									block->free_list32 = temp2;
								}
								else {
									before->next = temp2;
								}
							}
							else {
								if (!before) {
									block->free_list32 = next;
								}
								else {
									before->next = temp->next;
								}
							}
							return ptr;
						}
						before = temp;
						temp = temp->next;
					}
				}

				if (block->offset + size <= block->capacity) {
					void* ptr = block->data + block->offset;
					uint64_t remain = block->capacity - size;
					if (!std::align(alignof(T), size, ptr, remain)) {
						block = block->next;
						continue;
					}
					uint64_t diff = ((uint8_t*)ptr - block->data);
					block->offset = diff + size;
					return reinterpret_cast<T*>(ptr);
				}
				block = block->next;
			}

			// allocate new block
			uint64_t newCap = std::max(defaultBlockSize, size + 64);
			Block* newBlock = new (std::nothrow) Block(newCap);
			if (!newBlock) {
				return nullptr;
			}
			counter++;
			uint64_t remain = newCap - newBlock->offset;
			void* ptr = newBlock->data + newBlock->offset;
			if (!std::align(alignof(T), size, ptr, remain)) {
				delete newBlock;
				return nullptr;
			}
			uint64_t diff = ((uint8_t*)ptr - newBlock->data);
			newBlock->offset = diff + size;

			newBlock->next = head[2];
			head[2] = newBlock;

			return reinterpret_cast<T*>(ptr);
		}
		// Array or Object
		template <class T>
		T* allocate64(uint64_t size) {
			Block* block = head[3];

			while (block) {
				if (block->free_list64) {
					Node64* before = nullptr;
					Node64* temp = block->free_list64;
					Node64* next = temp->next;
					while (temp) {
						next = temp->next;
						if (size <= temp->size) {
							T* ptr = reinterpret_cast<T*>(temp);
							if (size < temp->size) {
								uint64_t diff = temp->size - size;
								Node64* temp2 = temp + size / 64;
								temp2->size = diff / 64;
								temp2->next = next;
								if (!before) {
									block->free_list64 = temp2;
								}
								else {
									before->next = temp2;
								}
							}
							else {
								if (!before) {
									block->free_list64 = next;
								}
								else {
									before->next = temp->next;
								}
							}
							return ptr;
						}
						before = temp;
						temp = temp->next;
					}
				}
				if (block->offset + size <= block->capacity) {
					void* ptr = block->data + block->offset;
					uint64_t remain = block->capacity - size;
					if (!std::align(alignof(T), size, ptr, remain)) {
						block = block->next;
						continue;
					}
					uint64_t diff = ((uint8_t*)ptr - block->data);
					block->offset = diff + size;
					return reinterpret_cast<T*>(ptr);
				}
				block = block->next;
			}

			// allocate new block
			uint64_t newCap = std::max(defaultBlockSize, size + 64);
			Block* newBlock = new (std::nothrow) Block(newCap);
			if (!newBlock) {
				return nullptr;
			}
			counter++;
			uint64_t remain = newCap - newBlock->offset;
			void* ptr = newBlock->data + newBlock->offset;
			if (!std::align(alignof(T), size, ptr, remain)) {
				delete newBlock;
				return nullptr;
			}
			uint64_t diff = ((uint8_t*)ptr - newBlock->data);
			newBlock->offset = diff + size;

			newBlock->next = head[3];
			head[3] = newBlock;

			return reinterpret_cast<T*>(ptr);
		}
	public:
		template <class T>
		T* allocate(uint64_t size, uint64_t align = alignof(T)) {
			
			if (sizeof(T) == 16) {
				return allocate16<T>(size);
			}
			if (sizeof(T) == 32) {
				return allocate32<T>(size);
			}
			if (sizeof(T) == 64) {
				return allocate64<T>(size);
			}

			{
				Block* block = head[0];

				while (block) {
					if (block->offset + size <= block->capacity) {
						void* ptr = block->data + block->offset;
						uint64_t remain = block->capacity - size;
						if (!std::align(alignof(T), size, ptr, remain)) {
							block = block->next;
							continue;
						}
						uint64_t diff = ((uint8_t*)ptr - block->data);
						block->offset = diff + size;
						return reinterpret_cast<T*>(ptr);
					}
					block = block->next;
				}
			}

			// allocate new block
			uint64_t newCap = std::max(defaultBlockSize, size + 64);
			Block* newBlock = new (std::nothrow) Block(newCap);
			if (!newBlock) {
				return nullptr;
			}
			counter++;

			newBlock->next = head[0];
			head[0] = newBlock;

			void* ptr = newBlock->data + newBlock->offset;
			uint64_t remain = newBlock->capacity - newBlock->offset;
			if (!std::align(alignof(T), size, ptr, remain)) {
				delete newBlock;
				return nullptr;
			}
			uint64_t diff = ((uint8_t*)ptr - newBlock->data);
			newBlock->offset += size;
			
			return reinterpret_cast<T*>(ptr);
		}

		// expand
		template <class T>
		void deallocate(T* ptr, uint64_t len) {		

			if (sizeof(T) == 16) {
				return deallocate16<T>(ptr, len);
			}
			if (sizeof(T) == 32) {
				return deallocate32<T>(ptr, len);
			}
			if (sizeof(T) == 64) {
				return deallocate64<T>(ptr, len);
			}

			return;
			// has bug!
			Block* block = head[0];

			while (block) {
				if ((uint8_t*)(ptr) + sizeof(T) * len == (uint8_t*)(block->data) + block->offset) {
					block->offset = (uint8_t*)ptr - (uint8_t*)block->data;
						//std::cout << "real_deallocated\n"; //
					return;
				}

				block = block->next;
			}
		}

		template<typename T, typename... Args>
		T* create(Args&&... args) {
			void* mem = allocate<T>(sizeof(T), alignof(T));
			return new (mem) T(std::forward<Args>(args)...);
		}

		void reset() {
			for (int i = 0; i < 4; ++i) {
				if (head[i]) {
					Block* block = head[i]->next;
					while (block) {
						Block* next = block->next;
						delete block;
						block = next;
					}
					head[i]->next = nullptr;
					head[i]->offset = 0;
					rear[i] = head[i];
				}
			}
		}

		~Arena() {
			reset();
			for (int i = 0; i < 4; ++i) {
				if (head[i]) {
					delete head[i];
				}
			}
		}

		// chk! when merge?
		void link_from(Arena& other) {
			for (int i = 0; i < 4; ++i) {
				if (!this->head[i]) {
					this->head[i] = other.head[i];
					this->rear[i] = other.rear[i];
				}
				else {
					this->rear[i]->next = other.head[i];
					this->rear[i] = other.rear[i];
				}
				other.head[i] = nullptr;
				other.rear[i] = nullptr;
			}
		}
	};

	template <class T>
	class Vector2 {
	private:
		T* m_arr = nullptr;
		uint64_t m_capacity = 0;
		uint64_t m_size = 0;
		Arena* pool = nullptr;
	public:
		Vector2() : pool(nullptr) {
			//
		}
		Vector2(uint64_t sz) : pool(nullptr) {
			m_size = sz;
			m_capacity = 2 * sz;
			m_arr = new T[m_capacity]();
		}
		Vector2(Arena* pool, uint64_t sz) : pool(pool) {
			m_size = sz;
			m_capacity = 2 * sz;
			if (pool) {
				m_arr = (T*)pool->allocate<T>(sizeof(T) * m_capacity, alignof(T));

				for (uint64_t i = 0; i < m_capacity; ++i) {
					new (&m_arr[i]) T();
				}
			}
			else {
				m_arr = new T[m_capacity]();
			}
		}
		Vector2(Arena* pool, uint64_t sz, uint64_t capacity) : pool(pool) {
			m_size = sz;
			m_capacity = capacity; 
			// todo - sz <= capacity!
			if (pool) {
				m_arr = (T*)pool->allocate<T>(sizeof(T) * m_capacity, alignof(T));

				for (uint64_t i = 0; i < m_capacity; ++i) {
					new (&m_arr[i]) T();
				}
			}
			else {
				m_arr = new T[m_capacity]();
			}
		}
		~Vector2() {
			if (m_arr && !pool) { delete[] m_arr; }
			else if (m_arr) {
				for (uint64_t i = 0; i < m_size; ++i) {
					m_arr[i].~T();
				}
			}
		}
		Vector2(const Vector2& other) {
			if (other.m_arr) {
				this->pool = other.pool;
				if (pool) {
					this->m_arr = (T*)pool->allocate<T>(sizeof(T) * other.m_capacity, alignof(T));
				}
				else {
					this->m_arr = new T[other.m_capacity]();
				}
				//this->m_arr = new T[other.m_capacity];
				this->m_capacity = other.m_capacity;
				this->m_size = other.m_size; 
				for (uint64_t i = 0; i < other.m_size; ++i) {
					new (&m_arr[i]) T(other.m_arr[i]);
				}
			}
		}
		Vector2(Vector2&& other) noexcept {
			std::swap(m_arr, other.m_arr);
			std::swap(m_capacity, other.m_capacity);
			std::swap(m_size, other.m_size);
			std::swap(pool, other.pool);
		}
	public:
		Vector2& operator=(const Vector2& other) {
			if (this == &other) { return *this; }

			if (other.m_arr) {
				if (this->m_arr && !this->pool) {
					delete[] this->m_arr; this->m_arr = nullptr;
				}
				else if (this->m_arr) {
					for (uint64_t i = 0; i < m_size; ++i) {
						m_arr[i].~T();
					}
					this->pool->deallocate(this->m_arr, this->m_capacity);
				}
				if (this->pool) {
					this->m_arr = pool->allocate<T>(sizeof(T) * other.m_capacity, alignof(T));
				}
				else {
					this->m_arr = new T[other.m_capacity]();
				}
				this->m_capacity = other.m_capacity;
				this->m_size = other.m_size;
				for (uint64_t i = 0; i < other.m_size; ++i) {
					new (&m_arr[i]) T(other.m_arr[i]);
				}
			}
			else {
				if (m_arr && !this->pool) { delete[] m_arr; }
				else if (m_arr) {
					for (uint64_t i = 0; i < m_size; ++i) {
						m_arr[i].~T();
					}
					this->pool->deallocate(this->m_arr, this->m_capacity);
				}
				m_arr = nullptr;
				m_capacity = 0;
				m_size = 0;
			}
			return *this;
		}
		void operator=(Vector2&& other) noexcept {
			if (this == &other) { return; }

			std::swap(m_arr, other.m_arr);
			std::swap(m_capacity, other.m_capacity);
			std::swap(m_size, other.m_size);
			std::swap(pool, other.pool);
		}
	public:
		void erase(T* p) {
			uint64_t idx = p - m_arr;

			for (uint64_t i = idx; i + 1 < m_size; ++i) {
				m_arr[i] = std::move(m_arr[i + 1]);
			}

			m_size--;
		}
		bool has_pool()const {
			return pool;
		}
		void insert(T* start, T* last) {
			uint64_t sz = m_size + (last - start);
			if (sz <= m_size) { return; }
			if (sz > m_capacity) {
				expand(2 * sz);
			}

			for (uint64_t i = m_size; i < sz; ++i) {
				m_arr[i] = std::move(start[i - m_size]);
			}
			m_size = sz;
		}

		void push_back(T x) {
			if (size() >= capacity()) {
				if (capacity() == 0) {
					expand(2);
				}
				else {
					expand(2 * capacity());
				}
			}

			m_arr[m_size++] = std::forward<T>(x);
		}
		void pop_back() {
			m_size--;
		}
		T& back() {
			return m_arr[m_size - 1];
		}
		const T& back() const {
			return m_arr[m_size - 1];
		}
		T& operator[](uint64_t idx) {
			return m_arr[idx];
		}
		const T& operator[](uint64_t idx) const {
			return m_arr[idx];
		}
		void reserve(uint64_t sz) {
			if (capacity() < sz) {
				expand(sz);
			}
		}
		bool empty() const { return 0 == m_size; }
		T* begin() { return m_arr; }
		T* end() { return m_arr + m_size; }
		const T* begin() const { return m_arr; }
		const T* end() const { return m_arr + m_size; }
		void clear() {
			m_size = 0;
		}
		uint64_t size() const { return m_size; }
		uint64_t capacity() const { return m_capacity; }

		void resize(uint64_t sz) {
			if (sz < m_capacity) {
				m_size = sz;
				return;
			}
			expand(sz * 2);
			m_size = sz;
		}
	private:
		void expand(uint64_t new_capacity) {
			if (pool) {
				T* temp = (T*)pool->allocate<T>(sizeof(T) * new_capacity, alignof(T));
				for (uint64_t i = 0; i < new_capacity; ++i) {
					new (temp + i) T();
					if (i < m_size) {
						temp[i] = std::move(m_arr[i]);
					}
				}
				for (uint64_t i = 0; i < m_size; ++i) {
					m_arr[i].~T();
				}
				pool->deallocate(m_arr, m_capacity);
				m_arr = temp;
			}
			else {
				T* temp = new (std::nothrow) T[new_capacity]();
				if (m_arr) {
					for (uint64_t i = 0; i < m_size; ++i) {
						temp[i] = std::move(m_arr[i]);
					}
					delete[] m_arr;
				}

				m_arr = temp;
			}
			m_capacity = new_capacity;
		}
	};

	template <class T>
	using std_vector = Vector2<T>;

} // end of claujson

