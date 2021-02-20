#pragma once
#include "../pch.h"
#include "BaseTelemVar.hpp"

#include <vector>

class StringTelemVar : public BaseTelemVar {
	public:

	//if truncate is set, all strings will be null-padded to preserve static frame length
	StringTelemVar(std::string name, scs_u32_t max_count, scs_u32_t* dynamic_count, size_t truncate_nullpad) :
		BaseTelemVar(name, max_count, dynamic_count, SCS_VALUE_TYPE_string), truncate_nullpad(truncate_nullpad), storage(max_count == SCS_U32_NIL ? 1 : max_count) {
		for (std::vector<char>& storage_elem : storage) {
			//init storage with empty strings
			std::fill_n(std::back_inserter(storage_elem), truncate_nullpad ? truncate_nullpad : 1, '\0');
		}
	}

	virtual ~StringTelemVar() {

	}

	virtual void write_to_buf(std::vector<char>& buffer) const {
		for (scs_u32_t i = 0; i < storage.size(); ++i) {
			if (dynamic_count && i >= *dynamic_count) {
				//write empty string as storage contents might be irrelevant
				std::fill_n(std::back_inserter(buffer), truncate_nullpad ? truncate_nullpad : 1, '\0');
			}
			else {
				buffer.insert(buffer.end(), storage[i].begin(), storage[i].end());
			}
		}
	}

	virtual void store_value(scs_value_t value, scs_u32_t index) {
		assert(value.type == SCS_VALUE_TYPE_string);

		if (index == SCS_U32_NIL) {
			index = 0;
		}

		if (index < storage.size()) {
			storage[index].clear();

			scs_string_t string = value.value_string.value;

			if (string) {
				size_t written = 0;

				//copy the string, if truncate_nullpad: at most truncate_nullpad bytes (incl nullchar)
				do {
					++written;
					if (truncate_nullpad && written == truncate_nullpad) {
						storage[index].push_back('\0');
						break;
					}

					storage[index].push_back(*string);
				}
				while (*string++);

				//if truncate_nullpad, fill remaining space with nullchars
				if (truncate_nullpad && written < truncate_nullpad) {
					std::fill_n(std::back_inserter(storage[index]), truncate_nullpad - written, '\0');
				}
			}
			else {
				std::fill_n(std::back_inserter(storage[index]), truncate_nullpad ? truncate_nullpad : 1, '\0');
			}
		}
	}

	virtual const void* get_val(scs_u32_t index) const {
		if (index == SCS_U32_NIL) {
			index = 0;
		}

		return &storage[index][0];
	}

	const size_t truncate_nullpad;

	protected:
	std::vector<std::vector<char>> storage;
};