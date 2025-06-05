﻿#pragma once

#include "claujson.h"

namespace claujson {

	class alignas(64) Array {
	protected:
		my_vector<_Value> arr_vec;
		StructuredPtr parent;
		bool _is_virtual = false;
		uint8_t temp[8];

		static _Value data_null; // valid is false..
		static const uint64_t npos;
	public:
		using _ValueIterator = _Value*;// my_vector<_Value>::iterator;
		using _ConstValueIterator = const _Value*; // my_vector<_Value>::const_iterator;
	protected:
		//explicit Array(bool valid);
	public:
		friend class _Value;
		friend class Object;
		friend class PartialJson;
		friend class StructuredPtr;
		friend class LoadData2;

		_Value clone(Arena* pool) const;

		void set_parent(StructuredPtr p);

		[[nodiscard]]
		static _Value Make(Arena* pool);

		[[nodiscard]]
		static _Value MakeVirtual(Arena* pool);
	private:
		explicit Array();
	public:

		~Array();

		bool has_pool() const { return arr_vec.has_pool(); }
		bool is_object() const;
		bool is_array() const;

		uint64_t find(const _Value& value, uint64_t start = 0) const; // find without key`s converting ( \uxxxx )

		_Value& operator[](uint64_t idx);

		const _Value& operator[](uint64_t idx) const;

		const StructuredPtr get_parent() const;

	public:

		void reserve_data_list(uint64_t len); // if object, reserve key_list and value_list, if array, reserve value_list.

		// for valid with object or array or root.
		uint64_t size() const {
			return get_data_size();
		}
		bool empty() const {
			return 0 == get_data_size();
		}

		uint64_t get_data_size() const;

		_Value& get_value_list(uint64_t idx);
	private:
		_Value& get_key_list(uint64_t idx);
	public:
		const _Value& get_value_list(uint64_t idx) const;

		const _Value& get_key_list(uint64_t idx) const;

		const _Value& get_const_key_list(uint64_t idx);

		const _Value& get_const_key_list(uint64_t idx) const;

		void clear(uint64_t idx);

		bool is_virtual() const;

		void clear();

		_ValueIterator begin();
		_ValueIterator end();
		_ConstValueIterator begin() const;
		_ConstValueIterator end() const;

		bool add_element(Value val);
		bool assign_element(uint64_t idx, Value val);

		bool insert(uint64_t idx, Value val);

		void erase(const _Value& key, bool real = false);
		void erase(uint64_t idx, bool real = false);


	private:
		// here only used in parsing.

		void MergeWith(Array* j, int start_offset);
		void MergeWith(Object* j, int start_offset);
		void MergeWith(PartialJson* j, int start_offset);

		void add_item_type(int64_t key_buf_idx, int64_t key_next_buf_idx, int64_t val_buf_idx, int64_t val_next_buf_idx,
			char* buf, uint64_t key_token_idx, uint64_t val_token_idx);

		void add_item_type(int64_t val_buf_idx, int64_t val_next_buf_idx,
			char* buf, uint64_t val_token_idx, Arena* pool);

		void add_user_type(int64_t key_buf_idx, int64_t key_next_buf_idx, char* buf,
			_ValueType type, uint64_t key_token_idx, Arena* pool


		);

		//
		void add_user_type(_ValueType type, Arena* pool

		); // int type -> enum?


	};

}
