#pragma once
#include "../pch.h"
#include "BaseTelemVar.hpp"

#include <vector>
#include <functional>

enum RoundMode {
	FLOOR,
	ROUND,
	CEIL
};

class FloatConvTelemVar : public BaseTelemVar {
	public:

	FloatConvTelemVar(std::string name, scs_value_type_t type, std::string converter_name, RoundMode round_mode, scs_u32_t max_count = SCS_U32_NIL, scs_u32_t* dynamic_count = nullptr) :
		BaseTelemVar(name, type, max_count, dynamic_count), converter(get_converter(converter_name)), round_mode(round_mode) { 

	}

	virtual ~FloatConvTelemVar() {

	}

	const RoundMode round_mode;

	protected:
	std::vector<scs_value_t> storage;

	private:

	const std::function<double(double)>& get_converter(const std::string& name) const {
		static const std::map<std::string, std::function<double(double)>> float_converters = {
			{
				"ms_to_kmh", [](double val) -> double {
					return val * 3.6;
				}
			},
			{
				"ms_to_mph", [](double val) -> double {
					return val * 2.2369362920544;
				}
			},
			{
				"psi_to_mpa", [](double val) -> double {
					return val * 0.006894759086775369;
				}
			},
			{
				"psi_to_bar", [](double val) -> double {
					return val * 0.0689475729;
				}
			},
			{
				"c_to_f", [](double val) -> double {
					return (val * 9 / 5) + 32;
				}
			},
			{
				"c_to_k", [](double val) -> double {
					return val + 273.15;
				}
			},
			{
				"l_to_gal", [](double val) -> double {
					return val * 0.264172052;
				}
			},
			{
				"km_to_mi", [](double val) -> double {
					return val * 0.621371192;
				}
			},
			{
				"l100km_to_mpg", [](double val) -> double {
					return 235.214583 / val;
				}
			}
		};

		return float_converters.at(name);
	}

	const std::function<double(double)>& converter;
};