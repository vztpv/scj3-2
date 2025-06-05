﻿
#include "claujson.h"

namespace claujson {

	extern Log log;
	_Value Array::data_null{ nullptr, false }; // valid is false..
	const uint64_t Array::npos = -1; // 

	_Value Array::clone(Arena* pool) const {
		_Value result = Array::Make(pool);

		if (result.as_array() == nullptr) {
			return result;
		}

		uint64_t sz = this->get_data_size();
		for (uint64_t i = 0; i < sz; ++i) {
			auto x = this->get_value_list(i).clone(pool);
			result.as_array()->add_element(std::move(x));
		}

		return result;
	}


	void Array::set_parent(StructuredPtr p) {
		parent = p;
	}

	_Value Array::Make(Arena* pool) {
		Array* temp = nullptr;
		if (pool) {
			temp = (Array*)pool->allocate<Array>(sizeof(Array), alignof(Array)); // new (std::nothrow) Array();
			new (temp) Array();
			temp->arr_vec = my_vector<_Value>(pool, 0, 2);
		}
		else {
			temp = new (std::nothrow) Array();
		}
		if (temp == nullptr) {
			_Value v;
			v._type = _ValueType::ERROR;
			return v;
		}

		return _Value(temp);
	}

	_Value Array::MakeVirtual(Arena* pool) {
		Array* temp = nullptr;
		if (pool) {
			temp = (Array*)pool->allocate<Array>(sizeof(Array), alignof(Array)); // new (std::nothrow) Array();
			new (temp) Array();
			temp->arr_vec = my_vector<_Value>(pool, 0, 2);
		}
		else {
			temp = new (std::nothrow) Array();
		}

		if (temp == nullptr) {
			_Value v;
			v._type = _ValueType::ERROR;
			return v;
		}
		temp->_is_virtual = true;
		return _Value(temp);
	}

	Array::Array() {}


	Array::~Array() {
		//
	}

	bool Array::is_object() const {
		return false;
	}
	bool Array::is_array() const {
		return true;
	}

	uint64_t Array::find(const _Value& value, uint64_t start) const {
		uint64_t sz = size();
		for (uint64_t i = start; i < sz; ++i) {
			if (get_value_list(i) == value) {
				return i;
			}
		}
		return npos;
	}

	_Value& Array::operator[](uint64_t idx) {
		return this->get_value_list(idx);
	}

	const _Value& Array::operator[](uint64_t idx) const {
		return this->get_value_list(idx);
	}

	const StructuredPtr Array::get_parent() const {
		return this->parent;
	}

	uint64_t Array::get_data_size() const {
		return arr_vec.size();
	}

	_Value& Array::get_value_list(uint64_t idx) {
		return arr_vec[idx];
	}

	_Value& Array::get_key_list(uint64_t idx) {
		return data_null;
	}

	const _Value& Array::get_const_key_list(uint64_t idx) {
		return data_null;
	}

	const _Value& Array::get_const_key_list(uint64_t idx) const {
		return data_null;
	}


	const _Value& Array::get_value_list(uint64_t idx) const {
		return arr_vec[idx];
	}

	const _Value& Array::get_key_list(uint64_t idx) const {
		return data_null;
	}

	void Array::clear(uint64_t idx) {
		arr_vec[idx].clear(false);
	}

	bool Array::is_virtual() const {
		return _is_virtual;
	}
	void Array::clear() {
		arr_vec.clear();
	}

	void Array::reserve_data_list(uint64_t len) {
		arr_vec.reserve(len);
	}


	Array::_ValueIterator Array::begin() {
		return arr_vec.begin();
	}

	Array::_ValueIterator Array::end() {
		return arr_vec.end();
	}


	Array::_ConstValueIterator Array::begin() const {
		return arr_vec.begin();
	}

	Array::_ConstValueIterator Array::end() const {
		return arr_vec.end();
	}

	bool Array::add_element(Value val) {
		
		if (val.Get().is_array()) {
			val.Get().as_array()->set_parent(this);
		}
		else if (val.Get().is_object()) {
			val.Get().as_object()->set_parent(this);
		}

		arr_vec.push_back(std::move(val.Get()));

		return true;
	}

	bool Array::assign_element(uint64_t idx, Value val) {
		if (val.Get().is_array()) {
			val.Get().as_array()->set_parent(this);
		}
		else if (val.Get().is_object()) {
			val.Get().as_object()->set_parent(this);
		}

		arr_vec[idx] = (std::move(val.Get()));

		return true;
	}

	// idx range check?
	bool Array::insert(uint64_t idx, Value val) {
		if (!add_element(std::move(val))) {
			return false;
		}
		_Value temp = std::move(arr_vec.back());
		uint64_t sz = size();
		for (uint64_t i = sz - 1; i > idx; --i) {
			arr_vec[i] = std::move(arr_vec[i - 1]);
		}
		arr_vec[idx] = std::move(temp);

		return true;
	}


	void Array::erase(const _Value& key, bool real) {
		uint64_t idx = this->find(key);
		erase(idx, real);
	}

	void Array::erase(uint64_t idx, bool real) {

		if (real) {
			clean(arr_vec[idx]);
		}

		arr_vec.erase(arr_vec.begin() + idx);
	}


	void Array::MergeWith(Array* j, int start_offset) {
		auto* x = j;

		uint64_t len = j->get_data_size();
		for (uint64_t i = 0; i < len; ++i) {
			if (j->get_value_list(i).is_array()) {
				j->get_value_list(i).as_array()->set_parent(this);
			}
			else if (j->get_value_list(i).is_object()) {
				j->get_value_list(i).as_object()->set_parent(this);
			}
		}

		if (x->arr_vec.empty() == false) {
			arr_vec.insert((x->arr_vec.begin()) + start_offset,
				(x->arr_vec.end()));
		}
		else {

			log << info << "test3";
		}
	}
	void Array::MergeWith(Object* j, int start_offset) {
		ERROR("Array::MergeWith Error");
	}
	void Array::MergeWith(PartialJson* j, int start_offset) {
		auto* x = j;

		if (x->obj_data.empty() == false) { // not object?
			ERROR("partial json is not array");
		}

		uint64_t len = j->get_data_size();
		for (uint64_t i = 0; i < len; ++i) {
			if (j->get_value_list(i).is_array()) {
				j->get_value_list(i).as_array()->set_parent(this);
			}
			else if (j->get_value_list(i).is_object()) {
				j->get_value_list(i).as_object()->set_parent(this);
			}
		}

		if (x->virtualJson.is_structured()) {
			start_offset = 0;
		}

		if (x->arr_vec.empty() == false) {
			arr_vec.insert((x->arr_vec.begin()) + start_offset,
				(x->arr_vec.end()));
		}
		else {

			log << info << "test4";
		}
	}


	void Array::add_item_type(int64_t key_buf_idx, int64_t key_next_buf_idx, int64_t val_buf_idx, int64_t val_next_buf_idx,
		char* buf, uint64_t key_token_idx, uint64_t val_token_idx) {

		// error
		log << warn << "error..";
		ERROR("Error Array::add_item_type");
	}

	void Array::add_item_type(int64_t val_buf_idx, int64_t val_next_buf_idx,
		char* buf, uint64_t val_token_idx, Arena* pool) {

			{
				_Value temp2;
				bool e = false;
				claujson::Convert(pool, temp2, val_buf_idx, val_next_buf_idx, false, buf, val_token_idx, e);
				if (e) {

					ERROR("Error in add_item_type");
				}
				arr_vec.push_back(std::move(temp2));
			}
	}

	void Array::add_user_type(int64_t key_buf_idx, int64_t key_next_buf_idx, char* buf,
		_ValueType type, uint64_t key_token_idx, Arena* pool

	) {
		log << warn << "error";
		ERROR("Array::add_user_type1");
	}

	void Array::add_user_type(_ValueType type
		, Arena* pool
	) {

		if (type == _ValueType::OBJECT) {
			_Value json = Object::Make(pool); 
			// pool->create<Object>(); // new (std::nothrow) Object(
			
		//	);

			if (json.as_structured() == nullptr) {
				log << warn << "new error";
				return;
			}
		
			json.as_structured().set_parent(this);
			arr_vec.push_back(std::move(json));
	

		}
		else if (type == _ValueType::ARRAY) {
			_Value json = Array::Make(pool);
			// pool->create<Array>(); // new (std::nothrow) Array(

		//	);

			if (json.as_structured() == nullptr) {
				log << warn << "new error";
				return;
			}

			json.as_structured().set_parent(this);
			arr_vec.push_back(std::move(json));
		}
	}
	
}
