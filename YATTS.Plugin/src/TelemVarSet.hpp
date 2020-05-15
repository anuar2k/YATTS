#pragma once
#include "pch.h"
#include "TelemVar.hpp"
#include <string>
#include <vector>
#include <set>

struct TelemVarCmp {
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

class TelemVarSet {
	public:

	TelemVarSet(std::string name) : name(name) { }

	~TelemVarSet() {
		free_vars();
	}

	void insert(TelemVar* tv) {
		assert(config_vars_set.find(tv) == config_vars_set.end());
		config_vars_vec.push_back(tv);
		config_vars_set.insert(tv);
	}

	TelemVar* try_get(const char* name) {
		std::set<TelemVar*, TelemVarCmp>::iterator result = config_vars_set.find(name);
		return result != config_vars_set.end() ? *result : nullptr;
	}

	void update_set(const scs_named_value_t* attributes) {
		while (attributes->name) {
			TelemVar* to_update = try_get(attributes->name);
			if (to_update) {
				assert(attributes->value.type == to_update->type);
				to_update->store_value(attributes->value, attributes->index);
			}
			++attributes;
		}
	}

	void clear() {
		free_vars();
		config_vars_vec.clear();
		config_vars_set.clear();
	}

	size_t size() const {
		return config_vars_vec.size();
	}

	TelemVar* operator[](size_t pos) {
		return config_vars_vec[pos];
	}

	std::vector<TelemVar*>::const_iterator begin() const {
		return config_vars_vec.cbegin();
	}

	std::vector<TelemVar*>::const_iterator end() const {
		return config_vars_vec.cend();
	}

	const std::string name;

	private:

	void free_vars() {
		for (TelemVar* tv : config_vars_vec) {
			delete tv;
		}
	}

	std::vector<TelemVar*> config_vars_vec;
	std::set<TelemVar*, TelemVarCmp> config_vars_set;
};

struct TelemVarSetCmp {
	using is_transparent = void;

	bool operator()(TelemVarSet* const& lhs, const char* const& rhs) const {
		if (!lhs) return rhs;
		if (!rhs) return false;
		return lhs->name < rhs;
	}

	bool operator()(const char* const& lhs, TelemVarSet* const& rhs) const {
		if (!lhs) return rhs;
		if (!rhs) return false;
		return lhs < rhs->name;
	}

	bool operator()(TelemVarSet* const& lhs, TelemVarSet* const& rhs) const {
		if (!lhs) return rhs;
		if (!rhs) return false;
		return lhs->name < rhs->name;
	}
};