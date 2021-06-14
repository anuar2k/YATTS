#pragma once
#include "pch.h"
#include "scs_sdk_1_12/include/scssdk_value.h"

//reflects sizes of types from scssdk_value.h
const size_t scssdk_value_sizes[] = {
    0,
    sizeof(scs_value_bool_t),
    sizeof(scs_value_s32_t),
    sizeof(scs_value_u32_t),
    sizeof(scs_value_u64_t),
    sizeof(scs_value_float_t),
    sizeof(scs_value_double_t),
    sizeof(scs_value_fvector_t),
    sizeof(scs_value_dvector_t),
    sizeof(scs_value_euler_t),
    sizeof(scs_value_fplacement_t),
    sizeof(scs_value_dplacement_t),
    sizeof(scs_value_string_t),
    sizeof(scs_value_s64_t),
    sizeof(scs_value_s64_t)
};
