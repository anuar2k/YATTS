#pragma once
#include "../pch.h"
#include "BaseTelemVar.hpp"
#include "FloatConverters.hpp"

#include <vector>
#include <functional>

enum class IntConvMode {
	NONE,
	FLOOR,
	ROUND,
	CEIL
};

class FloatConvTelemVar : public BaseTelemVar {
	public:

	FloatConvTelemVar(std::string name, scs_u32_t max_count, scs_u32_t* dynamic_count, scs_value_type_t type, FloatConverters::Converter conv_name, IntConvMode int_conv_mode) :
		BaseTelemVar(name, max_count, dynamic_count, type), converter(FloatConverters::get_converter(conv_name)), int_conv_mode(int_conv_mode) {

	}

	virtual ~FloatConvTelemVar() {

	}

	const IntConvMode int_conv_mode;

	protected:
	std::vector<scs_value_t> storage;

	private:

	const std::function<double(double)>& converter;
};