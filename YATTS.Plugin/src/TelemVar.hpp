#pragma once
#include "pch.h"
#include "scssdk_value_sizes.h"
#include <string>
#include <vector>

class TelemVar abstract {
	public:

	TelemVar(std::string name, scs_value_type_t type, scs_u32_t max_count = SCS_U32_NIL, scs_u32_t* dynamic_count = nullptr) :
		name(name), type(type), type_size(scssdk_value_sizes[type]), max_count(max_count), dynamic_count(dynamic_count) { }

	virtual ~TelemVar() { }

	TelemVar& operator=(TelemVar&) = delete;
	TelemVar(TelemVar&) = delete;

	virtual size_t write_to_buf(std::vector<char>& buffer) abstract;

	virtual void store_value(scs_value_t value, scs_u32_t index) abstract;

	const std::string name;
	const scs_value_type_t type;
	const size_t type_size;
	const scs_u32_t max_count;
	scs_u32_t* const dynamic_count;
};

struct TelemVarPtrCmp {
	using is_transparent = void;

	bool operator()(TelemVar* const& lhs, const char* const& rhs) const {
		if (!lhs) return rhs;
		if (!rhs) return false;
		return lhs->name < rhs;
	}

	bool operator()(const char* const& lhs, TelemVar* const& rhs) const {
		if (!lhs) return rhs;
		if (!rhs) return false;
		return lhs < rhs->name;
	}

	bool operator()(TelemVar* const& lhs, TelemVar* const& rhs) const {
		if (!lhs) return rhs;
		if (!rhs) return false;
		return lhs->name < rhs->name;
	}
};

class ScalarTelemVar : public TelemVar {
	public:

	ScalarTelemVar(std::string name, scs_value_type_t type, scs_u32_t max_count = SCS_U32_NIL, scs_u32_t* dynamic_count = nullptr) :
		TelemVar(name, type, max_count, dynamic_count), storage(max_count == SCS_U32_NIL ? 1 : max_count) { }

	virtual ~ScalarTelemVar() { }

	virtual size_t write_to_buf(std::vector<char>& buffer) {
		for (scs_u32_t i = 0; i < storage.size(); i++) {
			if (dynamic_count && i >= *dynamic_count) {
				//write zeroes as stored data might be irrelevant
				for (size_t pos = 0; pos < type_size; ++pos) {
					buffer.push_back(0);
				}
			}
			else {
				char* data = reinterpret_cast<char*>(&storage[i].value_bool);
				for (size_t pos = 0; pos < type_size; ++pos) {
					buffer.push_back(data[pos]);
				}
			}
		}
		return type_size * max_count;
	}

	virtual void store_value(scs_value_t value, scs_u32_t index) {
		if (index == SCS_U32_NIL) {
			index = 0;
		}

		assert(value.type == type);
		assert(index < storage.size());

		storage[index] = value;
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

	void adjust_channels() {
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
					for (scs_u32_t i = 0; i < max_count; i++) {
						reg_chan(name.c_str(), i, type, SCS_TELEMETRY_CHANNEL_FLAG_none, chan_callback, this);
					}
				}
				reg_chan_cnt = max_count;
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

	StringTelemVar(std::string name, size_t truncate = 0, scs_u32_t max_count = SCS_U32_NIL, scs_u32_t* dynamic_count = nullptr) :
		TelemVar(name, SCS_VALUE_TYPE_string, max_count, dynamic_count), truncate(truncate), storage(max_count == SCS_U32_NIL ? 1 : max_count) {
		for (std::vector<char>& storage_elem : storage) {
			//init storage with empty strings
			storage_elem.push_back('\0');
		}
	}

	virtual ~StringTelemVar() { }

	virtual size_t write_to_buf(std::vector<char>& buffer) {
		size_t total_written = 0;

		for (scs_u32_t i = 0; i < storage.size(); ++i) {
			if (dynamic_count && i >= *dynamic_count) {
				//write empty string as storage contents might be irrelevant
				buffer.push_back('\0');
				++total_written;
			}
			else {
				for (char chr : storage[i]) {
					buffer.push_back(chr);
					++total_written;
				}
			}
		}

		return total_written;
	}

	virtual void store_value(scs_value_t value, scs_u32_t index) {
		if (index == SCS_U32_NIL) {
			index = 0;
		}

		assert(value.type == SCS_VALUE_TYPE_string);
		assert(index < storage.size());

		storage[index].clear();

		scs_string_t string = value.value_string.value;

		if (string) {
			size_t curr_written = 0;

			do {
				++curr_written;
				if (truncate && curr_written == truncate) {
					storage[index].push_back('\0');
					break;
				}

				storage[index].push_back(*string);
			}
			while (*string++);
		}
		else {
			storage[index].push_back('\0');
		}
	}

	const size_t truncate;

	protected:
	std::vector<std::vector<char>> storage;
};
