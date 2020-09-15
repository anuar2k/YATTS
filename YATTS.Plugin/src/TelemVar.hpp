#pragma once
#include "pch.h"
#include "scssdk_value_sizes.h"
#include <string>
#include <vector>
#include <memory>
#include <iterator>

class TelemVar abstract {
	public:

	TelemVar(std::string name, scs_value_type_t type, scs_u32_t max_count = SCS_U32_NIL, scs_u32_t* dynamic_count = nullptr) :
		name(name), type(type), type_size(scssdk_value_sizes[type]), max_count(max_count), dynamic_count(dynamic_count) { }

	virtual ~TelemVar() { }

	//ensure that TelemVar isn't moved around the memory - a callback is registered to the object pointing at the address
	//disabled copy ctor makes it impossible to store the object directly in a container - use dynalloc
	TelemVar& operator=(TelemVar&) = delete;
	TelemVar(TelemVar&) = delete;

	//writes max_count * type_size bytes of stored data to the buffer; string is an exception, obviously
	//TODO: consider adding an endianness switch (little endian by default)
	virtual void write_to_buf(std::vector<char>& buffer) const abstract;

	//is it legal to call store_value on index that is unexistent in TelemVar's internal storage
	virtual void store_value(scs_value_t value, scs_u32_t index) abstract;

	//debug-only method
	virtual const void* get_val(scs_u32_t index) const abstract;

	const std::string name;
	const scs_value_type_t type;
	const size_t type_size;
	const scs_u32_t max_count;
	scs_u32_t* const dynamic_count;

	//this should be probably generated using a template to work with a plethora of other ptr types
	struct shared_ptrCmp {
		using is_transparent = void;

		bool operator()(std::shared_ptr<TelemVar> const& lhs, const char* const& rhs) const {
			if (!lhs) return bool(rhs);
			if (!rhs) return false;
			return lhs->name < rhs;
		}

		bool operator()(const char* const& lhs, std::shared_ptr<TelemVar> const& rhs) const {
			if (!lhs) return bool(rhs);
			if (!rhs) return false;
			return lhs < rhs->name;
		}

		bool operator()(std::shared_ptr<TelemVar> const& lhs, std::shared_ptr<TelemVar> const& rhs) const {
			if (!lhs) return bool(rhs);
			if (!rhs) return false;
			return lhs->name < rhs->name;
		}
	};
};

class ScalarTelemVar : public TelemVar {
	public:

	ScalarTelemVar(std::string name, scs_value_type_t type, scs_u32_t max_count = SCS_U32_NIL, scs_u32_t* dynamic_count = nullptr) :
		TelemVar(name, type, max_count, dynamic_count), storage(max_count == SCS_U32_NIL ? 1 : max_count) { }

	virtual ~ScalarTelemVar() { }

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

	protected:
	std::vector<scs_value_t> storage;
};

SCSAPI_VOID chan_callback(const scs_string_t name, const scs_u32_t index, const scs_value_t* const value, const scs_context_t context);

class ChannelTelemVar : public ScalarTelemVar {
	public:

	ChannelTelemVar(std::string name, scs_value_type_t type, scs_u32_t max_count = SCS_U32_NIL, scs_u32_t* dynamic_count = nullptr) :
		ScalarTelemVar(name, type, max_count, dynamic_count) { }

	virtual ~ChannelTelemVar() {
		unreg_callbacks();
	}

	//can be called multiple times to adjust the number of registered channels based on dynamic_count
	void reg_callbacks() {
		if (dynamic_count) {
			while (reg_chan_cnt < *dynamic_count && reg_chan_cnt < max_count) {
				reg_chan(name.c_str(), reg_chan_cnt, type, SCS_TELEMETRY_CHANNEL_FLAG_none, chan_callback, this);
				++reg_chan_cnt;
			}
			while (reg_chan_cnt > *dynamic_count && reg_chan_cnt > 0) {
				--reg_chan_cnt;
				unreg_chan(name.c_str(), reg_chan_cnt, type);
			}
		}
		else {
			if (reg_chan_cnt == 0) {
				if (max_count == SCS_U32_NIL) {
					reg_chan(name.c_str(), SCS_U32_NIL, type, SCS_TELEMETRY_CHANNEL_FLAG_none, chan_callback, this);
				}
				else {
					for (scs_u32_t i = 0; i < max_count; ++i) {
						reg_chan(name.c_str(), i, type, SCS_TELEMETRY_CHANNEL_FLAG_none, chan_callback, this);
					}
				}
				reg_chan_cnt = max_count;
			}
		}
	}

void unreg_callbacks() {
	if (reg_chan_cnt != 0) {
		if (max_count == SCS_U32_NIL) {
			unreg_chan(name.c_str(), SCS_U32_NIL, type);
		}
		else {
			while (reg_chan_cnt > 0) {
				--reg_chan_cnt;
				unreg_chan(name.c_str(), reg_chan_cnt, type);
			}
		}
	}
}

	static scs_telemetry_register_for_channel_t reg_chan;
	static scs_telemetry_unregister_from_channel_t unreg_chan;

	protected:
	scs_u32_t reg_chan_cnt = 0;
};

scs_telemetry_register_for_channel_t ChannelTelemVar::reg_chan = nullptr;
scs_telemetry_unregister_from_channel_t ChannelTelemVar::unreg_chan = nullptr;

SCSAPI_VOID chan_callback(const scs_string_t name, const scs_u32_t index, const scs_value_t* const value, const scs_context_t context) {
	ChannelTelemVar* const tv = static_cast<ChannelTelemVar*>(context);
	assert(value && tv && value->type == tv->type);

	#pragma warning(suppress: 6011)
	tv->store_value(*value, index);
}

class StringTelemVar : public TelemVar {
	public:

	//if truncate is set, all strings will be null-padded to preserve static frame length
	StringTelemVar(std::string name, size_t truncate_nullpad = 0, scs_u32_t max_count = SCS_U32_NIL, scs_u32_t* dynamic_count = nullptr) :
		TelemVar(name, SCS_VALUE_TYPE_string, max_count, dynamic_count), truncate_nullpad(truncate_nullpad), storage(max_count == SCS_U32_NIL ? 1 : max_count) {
		for (std::vector<char>& storage_elem : storage) {
			//init storage with empty strings
			std::fill_n(std::back_inserter(storage_elem), truncate_nullpad ? truncate_nullpad : 1, '\0');
		}
	}

	virtual ~StringTelemVar() { }

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
