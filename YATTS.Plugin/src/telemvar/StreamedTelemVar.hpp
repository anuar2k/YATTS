#pragma once
#include "../pch.h"
#include "ChannelUpdateHandler.hpp"
#include "ScalarTelemVar.hpp"
#include "StringTelemVar.hpp"

class StreamedScalarTelemVar : public ScalarTelemVar, public ChannelUpdateHandler {
	public:

	StreamedScalarTelemVar(std::string name, scs_value_type_t type, scs_u32_t max_count = SCS_U32_NIL, scs_u32_t* dynamic_count = nullptr) :
		ScalarTelemVar(name, type, max_count, dynamic_count), ChannelUpdateHandler(*this) {
	}
};