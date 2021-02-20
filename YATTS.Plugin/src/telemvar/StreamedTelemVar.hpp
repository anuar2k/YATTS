#pragma once
#include "../pch.h"
#include "ChannelUpdateHandler.hpp"
#include "ScalarTelemVar.hpp"
#include "StringTelemVar.hpp"

class StreamedScalarTelemVar : public ScalarTelemVar, public ChannelUpdateHandler {
	public:

	StreamedScalarTelemVar(std::string name, scs_u32_t max_count, scs_u32_t* dynamic_count, scs_value_type_t type) :
		ScalarTelemVar(name, max_count, dynamic_count, type), ChannelUpdateHandler(*this) {
	}
};