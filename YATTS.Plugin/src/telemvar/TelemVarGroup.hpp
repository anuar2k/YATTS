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

    TelemVarGroup(std::string name, char frame_header) : name(name), frame_header(frame_header) { 
    
    }

    ~TelemVarGroup() { 
    
    }

    void insert(std::unique_ptr<BaseTelemVar> tv) {
        assert(vars_set.find(tv) == vars_set.end());
        BaseTelemVar* raw_ptr = tv.get();

        bool inserted = false;
        std::tie(std::ignore, inserted) = vars_set.insert(std::move(tv));

        if (inserted) {
            vars_vec.push_back(raw_ptr);
        }
    }

    void update_group(const scs_named_value_t* attributes) {
        while (attributes->name) {
            if (auto to_update_it = vars_set.find(attributes->name); to_update_it != vars_set.end()) {
                auto& to_update = *to_update_it;

                assert(attributes->value.type == to_update->type);
                to_update->store_value(attributes->value, attributes->index);
            }
            ++attributes;
        }
    }

    void clear() {
        vars_vec.clear();
        vars_set.clear();
    }

    size_t size() const {
        return vars_vec.size();
    }

    size_t frame_size() const {
        size_t result = 0;

        for (auto& var : vars_vec) {
            result += var->total_size();
        }

        return result;
    }

    BaseTelemVar* operator[](size_t pos) const {
        return vars_vec[pos];
    }

    const std::string name;
    const char frame_header;

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

    std::set<std::unique_ptr<BaseTelemVar>, BaseTelemVar::unique_ptrCmp> vars_set;
    std::vector<BaseTelemVar*> vars_vec;
};