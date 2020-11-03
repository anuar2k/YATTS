#pragma once
#include "pch.h"
#include "TelemVar.hpp"
#include <string>
#include <vector>
#include <set>
#include <memory>

class TelemVarSet {
	public:

	TelemVarSet(std::string name) : name(name) { 
	
	}

	~TelemVarSet() { 
	
	}

	void insert(std::shared_ptr<BaseTelemVar> tv) {
		assert(config_vars_set.find(tv) == config_vars_set.end());
		config_vars_vec.push_back(tv);
		config_vars_set.insert(tv);
	}

	std::shared_ptr<BaseTelemVar> try_get(const char* name) {
		auto result = config_vars_set.find(name);
		return result != config_vars_set.end() ? *result : std::shared_ptr<BaseTelemVar>();
	}

	void update_set(const scs_named_value_t* attributes) {
		while (attributes->name) {
			std::shared_ptr<BaseTelemVar> to_update = try_get(attributes->name);
			if (to_update) {
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

	const std::shared_ptr<BaseTelemVar>& operator[](size_t pos) const {
		return config_vars_vec[pos];
	}

	std::vector<std::shared_ptr<BaseTelemVar>>::const_iterator begin() const {
		return config_vars_vec.cbegin();
	}

	std::vector<std::shared_ptr<BaseTelemVar>>::const_iterator end() const {
		return config_vars_vec.cend();
	}

	const std::string name;

	//this should be probably generated using a template to work with a plethora of other ptr types
	struct shared_ptrCmp {
		using is_transparent = void;

		bool operator()(std::shared_ptr<TelemVarSet> const& lhs, const char* const& rhs) const {
			if (!lhs) return bool(rhs);
			if (!rhs) return false;
			return lhs->name < rhs;
		}

		bool operator()(const char* const& lhs, std::shared_ptr<TelemVarSet> const& rhs) const {
			if (!lhs) return bool(rhs);
			if (!rhs) return false;
			return lhs < rhs->name;
		}

		bool operator()(std::shared_ptr<TelemVarSet> const& lhs, std::shared_ptr<TelemVarSet> const& rhs) const {
			if (!lhs) return bool(rhs);
			if (!rhs) return false;
			return lhs->name < rhs->name;
		}
	};

	private:

	std::vector<std::shared_ptr<BaseTelemVar>> config_vars_vec;
	std::set<std::shared_ptr<BaseTelemVar>, BaseTelemVar::shared_ptrCmp> config_vars_set;
};