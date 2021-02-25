#pragma once
#include "../pch.h"
#include "BaseTelemVar.hpp"

#include <vector>

class ScalarTelemVar : public BaseTelemVar {
	public:

	ScalarTelemVar(std::string name, scs_u32_t max_count, scs_u32_t* dynamic_count, scs_value_type_t type) :
		BaseTelemVar(name, max_count, dynamic_count, type), storage(max_count == SCS_U32_NIL ? 1 : max_count) {

	}

	virtual ~ScalarTelemVar() {

	}

	virtual void write_to_buf(std::vector<char>& buffer) const {
		for (scs_u32_t i = 0; i < storage.size(); ++i) {
			if (dynamic_count && i >= *dynamic_count) {
				//write zeroes as stored data might be irrelevant
				for (size_t pos = 0; pos < type_size; ++pos) {
					buffer.push_back(0);
				}
			}
			else {
				const char* data = reinterpret_cast<const char*>(&storage[i].value_bool.value);
				for (size_t pos = 0; pos < type_size; ++pos) {
					buffer.push_back(data[pos]);
				}
			}
		}
	}

	virtual void store_value(scs_value_t value, scs_u32_t index) {
		assert(value.type == type);

		if (index == SCS_U32_NIL) {
			index = 0;
		}

		if (index < storage.size()) {
			storage[index] = value;
		}
	}

	virtual const void* get_val(scs_u32_t index) const {
		if (index == SCS_U32_NIL) {
			index = 0;
		}

		return &storage[index].value_bool.value;
	}

	private:

	std::vector<scs_value_t> storage;
};