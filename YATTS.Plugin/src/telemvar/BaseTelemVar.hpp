#pragma once
#include "../pch.h"
#include "../utils/scssdk_value_sizes.h"
#include <string>

class BaseTelemVar abstract {
	public:

	BaseTelemVar(std::string name, scs_value_type_t type, scs_u32_t max_count = SCS_U32_NIL, scs_u32_t* dynamic_count = nullptr) :
		name(name), type(type), type_size(scssdk_value_sizes[type]), max_count(max_count), dynamic_count(dynamic_count) {

	}

	virtual ~BaseTelemVar() {

	}

	//ensure that TelemVar isn't moved around the memory - a callback is registered to the object pointing at the address
	//disabled copy ctor makes it impossible to store the object directly in a container - use dynalloc
	BaseTelemVar& operator=(BaseTelemVar&) = delete;
	BaseTelemVar(BaseTelemVar&) = delete;

	//writes max_count * type_size bytes of stored data to the buffer; string is an exception, obviously
	//TODO: consider adding an endianness switch (little endian by default)
	virtual void write_to_buf(std::vector<char>& buffer) const abstract;

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

		bool operator()(std::shared_ptr<BaseTelemVar> const& lhs, const char* const& rhs) const {
			if (!lhs) return bool(rhs);
			if (!rhs) return false;
			return lhs->name < rhs;
		}

		bool operator()(const char* const& lhs, std::shared_ptr<BaseTelemVar> const& rhs) const {
			if (!lhs) return bool(rhs);
			if (!rhs) return false;
			return lhs < rhs->name;
		}

		bool operator()(std::shared_ptr<BaseTelemVar> const& lhs, std::shared_ptr<BaseTelemVar> const& rhs) const {
			if (!lhs) return bool(rhs);
			if (!rhs) return false;
			return lhs->name < rhs->name;
		}
	};
};