#pragma once
#include "../pch.h"
#include "telemvar/TelemVar.hpp"

#include <string>
#include <vector>
#include <memory>

class TelemVarParser {
	public:

	TelemVarParser() = delete;

	std::vector<std::unique_ptr<BaseTelemVar>> parse_vars_object(const json& vars_object) {
		std::vector<std::unique_ptr<BaseTelemVar>> result;

		for (const json& ctv_set_desc : vars_object) {
			std::string ctv_set_name = ctv_set_desc.at("name").get<std::string>();

			const json& ctv_list = ctv_set_desc.at("vars");
			var_count += ctv_list.size();

			for (const json& ctv_desc : ctv_list) {
				const std::string name = ctv_set_name + "." + ctv_desc.at("name").get<std::string>();
				const scs_value_type_t type = ctv_desc.at("type").get<scs_value_type_t>();
				const scs_u32_t max_count = ctv_desc.value("max_count", SCS_U32_NIL);
				scs_u32_t* dynamic_count = nullptr;

				if (max_count != SCS_U32_NIL && ctv_desc.contains("dynamic_count")) {
					const json& dynamic_count_desc = ctv_desc.at("dynamic_count");
					std::string dynamic_count_set_name = dynamic_count_desc.at("set_name").get<std::string>();
					std::string dynamic_count_var_name = dynamic_count_desc.at("var_name").get<std::string>();
					dynamic_count = get_dynamic_count_ptr(dynamic_count_set_name, dynamic_count_var_name);
				}

				std::unique_ptr<BaseTelemVar> telemvar;

				if (type == SCS_VALUE_TYPE_float || type == SCS_VALUE_TYPE_double) {
					telemvar = std::make_unique<ScalarTelemVar>(name, max_count, dynamic_count, type);
				}
				else if (type == SCS_VALUE_TYPE_string) {
					telemvar = std::make_unique<StringTelemVar>(name,
																max_count,
																dynamic_count,
																ctv_desc.at("truncate_nullpad").get<size_t>());
				}
				else {
					telemvar = std::make_unique<ScalarTelemVar>(name, max_count, dynamic_count, type);
				}

				result.emplace_back(std::make_unique<ChannelUpdateHandler>(std::move(telemvar)));
			}
		}

		return result;
	}
};