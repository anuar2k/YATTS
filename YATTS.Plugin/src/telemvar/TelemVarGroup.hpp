#pragma once
#include "pch.h"
#include "TelemVar.hpp"

#include <string>
#include <vector>
#include <set>
#include <memory>
#include <tuple>

class TelemVarGroup {
	public:

	TelemVarGroup(std::string name) : name(name) { 
	
	}

	~TelemVarGroup() { 
	
	}

	void insert(std::unique_ptr<BaseTelemVar> tv) {
		assert(config_vars_set.find(tv) == config_vars_set.end());
		BaseTelemVar* raw_ptr = tv.get();

		bool inserted = false;
		std::tie(std::ignore, inserted) = config_vars_set.insert(std::move(tv));

		if (inserted) {
			config_vars_vec.push_back(raw_ptr);
		}
	}

	void update_group(const scs_named_value_t* attributes) {
		while (attributes->name) {
			if (auto to_update_it = config_vars_set.find(attributes->name); to_update_it != config_vars_set.end()) {
				auto& to_update = *to_update_it;

				assert(attributes->value.type == to_update->type);
				to_update->store_value(attributes->value, attributes->index);
			}
			++attributes;
		}
	}

	void clear() {
		config_vars_vec.clear();
		config_vars_set.clear();
	}

	size_t size() const {
		return config_vars_vec.size();
	}

	BaseTelemVar* operator[](size_t pos) const {
		return config_vars_vec[pos];
	}

	const std::string name;

	//this should be probably generated using a template to work with a plethora of other ptr types
	struct unique_ptrCmp {
		using is_transparent = void;

		bool operator()(std::unique_ptr<TelemVarGroup> const& lhs, const char* const& rhs) const {
			if (!lhs) return bool(rhs);
			if (!rhs) return false;
			return lhs->name < rhs;
		}

		bool operator()(const char* const& lhs, std::unique_ptr<TelemVarGroup> const& rhs) const {
			if (!lhs) return bool(rhs);
			if (!rhs) return false;
			return lhs < rhs->name;
		}

		bool operator()(std::unique_ptr<TelemVarGroup> const& lhs, std::unique_ptr<TelemVarGroup> const& rhs) const {
			if (!lhs) return bool(rhs);
			if (!rhs) return false;
			return lhs->name < rhs->name;
		}
	};

	private:

	std::set<std::unique_ptr<BaseTelemVar>, BaseTelemVar::unique_ptrCmp> config_vars_set;
	std::vector<BaseTelemVar*> config_vars_vec;
};