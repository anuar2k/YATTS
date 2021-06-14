#pragma once
#include "../pch.h"
#include "../utils/scssdk_value_sizes.h"

#include <string>

class BaseTelemVar abstract {
    public:

    BaseTelemVar(
        std::string name, 
        scs_u32_t max_count, 
        scs_u32_t* dynamic_count, 
        scs_value_type_t type
    ) :
        name(name), 
        max_count(max_count), 
        dynamic_count(dynamic_count), 
        type(type), 
        type_size(scssdk_value_sizes[type]) {

    }

    virtual ~BaseTelemVar() {

    }

    //writes max_count * type_size bytes of stored data to the buffer; string is an exception, obviously
    //TODO: consider adding an endianness switch (little endian by default)
    virtual void write_to_buf(std::vector<char>& buffer) const abstract;

    virtual void store_value(scs_value_t value, scs_u32_t index) abstract;

    virtual size_t total_size() const {
        return type_size * max_count;
    }

    //debug-only method, keep in mind these pointers might be invalidated after resize of storage
    virtual const void* debug_val_ptr(scs_u32_t index) const abstract;

    const std::string name;
    const scs_u32_t max_count;
    scs_u32_t* const dynamic_count;
    const scs_value_type_t type;
    const size_t type_size;

    //this should be probably generated using a template to work with a plethora of other ptr types
    struct unique_ptrCmp {
        using is_transparent = void;

        bool operator()(std::unique_ptr<BaseTelemVar> const& lhs, const char* const& rhs) const {
            if (!lhs) return bool(rhs);
            if (!rhs) return false;
            return lhs->name < rhs;
        }

        bool operator()(const char* const& lhs, std::unique_ptr<BaseTelemVar> const& rhs) const {
            if (!lhs) return bool(rhs);
            if (!rhs) return false;
            return lhs < rhs->name;
        }

        bool operator()(std::unique_ptr<BaseTelemVar> const& lhs, std::unique_ptr<BaseTelemVar> const& rhs) const {
            if (!lhs) return bool(rhs);
            if (!rhs) return false;
            return lhs->name < rhs->name;
        }
    };
};