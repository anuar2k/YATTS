#pragma once
#include "../pch.h"
#include "BaseTelemVar.hpp"
#include "FloatConverters.hpp"

#include <vector>
#include <functional>

enum class IntCastMode {
    NONE,
    FLOOR,
    ROUND,
    CEIL
};

NLOHMANN_JSON_SERIALIZE_ENUM(IntCastMode, {
    {IntCastMode::NONE, "NONE"},
    {IntCastMode::FLOOR, "FLOOR"},
    {IntCastMode::ROUND, "ROUND"},
    {IntCastMode::CEIL, "CEIL"}
})

/**
* This class has the ability to convert single/double precision floating point values
* and cast it to int32_t
*/
class FloatConvTelemVar : public BaseTelemVar {
    public:

    FloatConvTelemVar(
        std::string name, 
        scs_u32_t max_count, 
        scs_u32_t* dynamic_count, 
        scs_value_type_t type, 
        std::function<double(double)> converter, 
        IntCastMode int_conv_mode
    ) :
        BaseTelemVar(
            name, 
            max_count, 
            dynamic_count, 
            type == SCS_VALUE_TYPE_float || type == SCS_VALUE_TYPE_double 
            ? type 
            : throw std::exception("FloatConvTelemVar is only applicable to float and double types")
        ),
        storage(max_count == SCS_U32_NIL ? 1 : max_count), 
        element_size(int_conv_mode != IntCastMode::NONE ? sizeof(int32_t) : type_size),
        converter(converter), 
        int_cast_mode(int_conv_mode) {

    }

    virtual ~FloatConvTelemVar() {

    }

    virtual void write_to_buf(std::vector<char>& buffer) const override {
        for (scs_u32_t i = 0; i < storage.size(); ++i) {
            if (dynamic_count && i >= *dynamic_count) {
                //write zeroes as stored data might be irrelevant
                for (size_t pos = 0; pos < element_size; ++pos) {
                    buffer.push_back(0);
                }
            }
            else {
                const char* data = reinterpret_cast<const char*>(&storage[i].value_bool.value);
                for (size_t pos = 0; pos < element_size; ++pos) {
                    buffer.push_back(data[pos]);
                }
            }
        }
    }

    virtual void store_value(scs_value_t value, scs_u32_t index) override {
        assert(value.type == type);

        if (index == SCS_U32_NIL) {
            index = 0;
        }

        if (index < storage.size()) {
            storage[index] = convert(value);
        }
    }

    virtual size_t total_size() const override {
        return element_size * (max_count == SCS_U32_NIL ? 1 : max_count);
    }

    virtual const void* debug_val_ptr(scs_u32_t index) const override {
        if (index == SCS_U32_NIL) {
            index = 0;
        }

        return &storage[index].value_bool.value;
    }

    private:

    scs_value_t convert(scs_value_t value) {
        scs_value_t result;

        if (type == SCS_VALUE_TYPE_float) {
            #pragma warning(disable: 4244) //we're fine with the lossy double->float conversion
            result.value_float.value = converter(value.value_float.value);

            if (int_cast_mode == IntCastMode::FLOOR) {
                result.value_s32.value = (int32_t)std::floor(result.value_float.value);
            }
            else if (int_cast_mode == IntCastMode::ROUND) {
                result.value_s32.value = (int32_t)std::round(result.value_float.value);
            }
            else if (int_cast_mode == IntCastMode::CEIL) {
                result.value_s32.value = (int32_t)std::ceil(result.value_float.value);
            }
        }
        else {
            result.value_double.value = converter(value.value_double.value);

            if (int_cast_mode == IntCastMode::FLOOR) {
                result.value_s32.value = (int32_t)std::floor(result.value_double.value);
            }
            else if (int_cast_mode == IntCastMode::ROUND) {
                result.value_s32.value = (int32_t)std::round(result.value_double.value);
            }
            else if (int_cast_mode == IntCastMode::CEIL) {
                result.value_s32.value = (int32_t)std::ceil(result.value_double.value);
            }
        }

        return result;
    }

    std::vector<scs_value_t> storage;
    const size_t element_size;
    const std::function<double(double)> converter;
    const IntCastMode int_cast_mode;
};