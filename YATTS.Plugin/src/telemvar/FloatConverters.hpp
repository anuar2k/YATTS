#pragma once
#include "../pch.h"

#include <functional>
#include <unordered_map>

namespace FloatConverters {
    enum class Converter {
        NONE,
        MS_TO_KMH,
        MS_TO_MPH,
        PSI_TO_MPA,
        PSI_TO_BAR,
        C_TO_F,
        C_TO_K,
        L_TO_GAL,
        KM_TO_MI,
        L100KM_TO_MPG
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(Converter, {
        {Converter::NONE, "NONE"},
        {Converter::MS_TO_KMH, "MS_TO_KMH"},
        {Converter::MS_TO_MPH, "MS_TO_MPH"},
        {Converter::PSI_TO_MPA, "PSI_TO_MPA"},
        {Converter::PSI_TO_BAR, "PSI_TO_BAR"},
        {Converter::C_TO_F, "C_TO_F"},
        {Converter::C_TO_K, "C_TO_K"},
        {Converter::L_TO_GAL, "L_TO_GAL"},
        {Converter::KM_TO_MI, "KM_TO_MI"},
        {Converter::L100KM_TO_MPG, "L100KM_TO_MPG"}
    })

    const std::unordered_map<Converter, std::function<double(double)>> converters = {
        {Converter::NONE,          [](double val) { return val; }},
        {Converter::MS_TO_KMH,     [](double val) { return val * 3.6; }},
        {Converter::MS_TO_MPH,     [](double val) { return val * 2.2369362920544; }},
        {Converter::PSI_TO_MPA,    [](double val) { return val * 0.006894759086775369; }},
        {Converter::PSI_TO_BAR,    [](double val) { return val * 0.0689475729; }},
        {Converter::C_TO_F,        [](double val) { return (val * 9 / 5) + 32; }},
        {Converter::C_TO_K,        [](double val) { return val + 273.15; }},
        {Converter::L_TO_GAL,      [](double val) { return val * 0.264172052; }},
        {Converter::KM_TO_MI,      [](double val) { return val * 0.621371192; }},
        {Converter::L100KM_TO_MPG, [](double val) { return 235.214583 / val; }}
    };
};

