#include "pch.h"
#include "telemvar/TelemVar.hpp"
#include "telemvar/TelemVarGroup.hpp"

#include <vector>
#include <set>
#include <map>
#include <functional>
#include <memory>
#include <fstream>

scs_log_t game_log = nullptr;
HANDLE timer_queue = NULL;

std::vector<std::unique_ptr<ChannelUpdateHandler>> channel_vars;
std::set<std::unique_ptr<TelemVarGroup>, TelemVarGroup::unique_ptrCmp> config_vars;
std::set<std::unique_ptr<TelemVarGroup>, TelemVarGroup::unique_ptrCmp> event_vars;

//TODO: consider generating this structure dynamically, from config too
std::map<std::string, std::map<std::string, scs_u32_t, std::less<>>, std::less<>> dynamic_count_vars = {
	{SCS_TELEMETRY_CONFIG_hshifter, {{SCS_TELEMETRY_CONFIG_ATTRIBUTE_selector_count, 0}}},
	{SCS_TELEMETRY_CONFIG_truck, {
		{SCS_TELEMETRY_CONFIG_ATTRIBUTE_forward_gear_count, 0},
		{SCS_TELEMETRY_CONFIG_ATTRIBUTE_reverse_gear_count, 0},
		{SCS_TELEMETRY_CONFIG_ATTRIBUTE_wheel_count, 0}
	}},
	{SCS_TELEMETRY_CONFIG_trailer     , {{SCS_TELEMETRY_CONFIG_ATTRIBUTE_wheel_count, 0}}},
	{SCS_TELEMETRY_CONFIG_trailer ".0", {{SCS_TELEMETRY_CONFIG_ATTRIBUTE_wheel_count, 0}}},
	{SCS_TELEMETRY_CONFIG_trailer ".1", {{SCS_TELEMETRY_CONFIG_ATTRIBUTE_wheel_count, 0}}},
	{SCS_TELEMETRY_CONFIG_trailer ".2", {{SCS_TELEMETRY_CONFIG_ATTRIBUTE_wheel_count, 0}}},
	{SCS_TELEMETRY_CONFIG_trailer ".3", {{SCS_TELEMETRY_CONFIG_ATTRIBUTE_wheel_count, 0}}},
	{SCS_TELEMETRY_CONFIG_trailer ".4", {{SCS_TELEMETRY_CONFIG_ATTRIBUTE_wheel_count, 0}}},
	{SCS_TELEMETRY_CONFIG_trailer ".5", {{SCS_TELEMETRY_CONFIG_ATTRIBUTE_wheel_count, 0}}},
	{SCS_TELEMETRY_CONFIG_trailer ".6", {{SCS_TELEMETRY_CONFIG_ATTRIBUTE_wheel_count, 0}}},
	{SCS_TELEMETRY_CONFIG_trailer ".7", {{SCS_TELEMETRY_CONFIG_ATTRIBUTE_wheel_count, 0}}},
	{SCS_TELEMETRY_CONFIG_trailer ".8", {{SCS_TELEMETRY_CONFIG_ATTRIBUTE_wheel_count, 0}}},
	{SCS_TELEMETRY_CONFIG_trailer ".9", {{SCS_TELEMETRY_CONFIG_ATTRIBUTE_wheel_count, 0}}}
};

void log_line(const scs_log_type_t type, const char* const text, ...) {
	if (!game_log) {
		return;
	}
	char formatted[1000];

	va_list args;
	va_start(args, text);
	vsnprintf_s(formatted, sizeof(formatted), _TRUNCATE, text, args);
	formatted[sizeof(formatted) - 1] = '\0';
	va_end(args);

	game_log(type, formatted);
}

//debug function
VOID CALLBACK display_state(_In_ PVOID lpParam, _In_ BOOLEAN TimerOrWaitFired) {
	bool on_ground[6];

	for (scs_u32_t i = 0; i < 6; ++i) {
		on_ground[i] = *reinterpret_cast<const bool*>(channel_vars[0]->telemvar->get_val(i));
	}

	log_line(SCS_LOG_TYPE_message, "%s: %d %d %d %d %d %d",
			 channel_vars[0]->telemvar->name.c_str(),
			 on_ground[0],
			 on_ground[1],
			 on_ground[2],
			 on_ground[3],
			 on_ground[4],
			 on_ground[5]);
}

SCSAPI_VOID telemetry_configuration(const scs_event_t event, const void* const event_info, const scs_context_t context) {
	assert(event == SCS_TELEMETRY_EVENT_configuration);
	const scs_telemetry_configuration_t* const config_info = static_cast<const scs_telemetry_configuration_t*>(event_info);
	assert(config_info);

	//update dynamic count
	#pragma warning(suppress: 6011)
	if (auto group_it = dynamic_count_vars.find(config_info->id); group_it != dynamic_count_vars.end()) {
		auto& var_map = group_it->second;
		const scs_named_value_t* attributes = config_info->attributes;
		while (attributes->name) {
			if (auto var_it = var_map.find(attributes->name); var_it != var_map.end()) {
				assert(attributes->value.type == SCS_VALUE_TYPE_u32);
				var_it->second = attributes->value.value_u32.value;
			}
			++attributes;
		}
	}

	//refresh dynamic_count ChannelTelemVars
	for (std::unique_ptr<ChannelUpdateHandler>& cah : channel_vars) {
		cah->reg_callbacks();
	}

	if (auto group_it = config_vars.find(config_info->id); group_it != config_vars.end()) {
		auto& group = *group_it;
		group->update_group(config_info->attributes);
	}
}

SCSAPI_VOID telemetry_gameplay_event(const scs_event_t event, const void* const event_info, const scs_context_t context) {
	assert(event == SCS_TELEMETRY_EVENT_gameplay);
	const scs_telemetry_gameplay_event_t* const gp_event_info = static_cast<const scs_telemetry_gameplay_event_t*>(event_info);
	assert(gp_event_info);

	if (auto group_it = event_vars.find(gp_event_info->id); group_it != event_vars.end()) {
		auto& group = *group_it;
		group->update_group(gp_event_info->attributes);
	}
}

scs_u32_t* get_dynamic_count_ptr(const std::string& set_name, const std::string& var_name) {
	if (auto group_it = dynamic_count_vars.find(set_name); group_it != dynamic_count_vars.end()) {
		auto& var_map = group_it->second;

		if (auto var_it = var_map.find(var_name); var_it != var_map.end()) {
			return &var_it->second;
		}
	}

	return nullptr;
}

//parses the config file into channel_vars, config_vars, event_vars
bool load_config() {
	try {
		std::ifstream config_file(CONFIG_PATH);

		if (config_file.fail()) {
			throw std::exception("config file not found - it must be accessible under <game executable location>\\" CONFIG_PATH);
		}

		const json config = json::parse(config_file);
		size_t var_count = 0;

		//version check--------------------------------------------------------
		if (config.at("version").at("major").get<int>() != YATTS_VERSION_MAJOR) {
			throw std::exception("major version doesn't match");
		}

		auto parse_vars_object = [&var_count](const json& vars_object, const std::string& name_prefix) {
			std::vector<std::unique_ptr<BaseTelemVar>> result;

			for (const json& tv_desc : vars_object) {
				var_count += 1;

				const std::string name = name_prefix + tv_desc.at("name").get<std::string>();
				const scs_value_type_t type = tv_desc.at("type").get<scs_value_type_t>();
				const scs_u32_t max_count = tv_desc.value("max_count", SCS_U32_NIL);
				scs_u32_t* dynamic_count = nullptr;

				if (max_count != SCS_U32_NIL && tv_desc.contains("dynamic_count")) {
					const json& dynamic_count_desc = tv_desc.at("dynamic_count");
					const std::string dynamic_count_set_name = dynamic_count_desc.at("set_name").get<std::string>();
					const std::string dynamic_count_var_name = dynamic_count_desc.at("var_name").get<std::string>();
					dynamic_count = get_dynamic_count_ptr(dynamic_count_set_name, dynamic_count_var_name);
				}

				std::unique_ptr<BaseTelemVar> telemvar;

				if (tv_desc.contains("float_converter")) {
					const json& float_converter_desc = tv_desc.at("float_converter");

					const std::string converter_type = float_converter_desc.at("type").get<std::string>();

					std::function<double(double)> converter;
					if (converter_type == "multiplier") {
						const double multiplier = float_converter_desc.at("multiplier").get<double>();
						auto fun = [multiplier](double val) {
							return val * multiplier;
						};

						converter = fun;
					}
					else if (converter_type == "unit") {
						converter = FloatConverters::converters.at(float_converter_desc.at("converter").get<FloatConverters::Converter>());
					}
					else {
						throw std::exception("invalid converter type");
					}

					telemvar = std::make_unique<FloatConvTelemVar>(name, 
																   max_count, 
																   dynamic_count, 
																   type, 
																   converter, 
																   float_converter_desc.at("int_cast_mode").get<IntCastMode>());
				}
				else if (type == SCS_VALUE_TYPE_string) {
					telemvar = std::make_unique<StringTelemVar>(name,
																max_count,
																dynamic_count,
																tv_desc.at("truncate_nullpad").get<size_t>());
				}
				else {
					telemvar = std::make_unique<ScalarTelemVar>(name, max_count, dynamic_count, type);
				}

				result.push_back(std::move(telemvar));
			}

			return result;
		};

		//load channel TelemVars
		for (const json& ctv_group : config.at("channel_vars")) {
			const std::string name_prefix = ctv_group.at("name").get<std::string>() + '.';

			for (std::unique_ptr<BaseTelemVar>& telemvar : parse_vars_object(ctv_group.at("vars"), name_prefix)) {
				channel_vars.push_back(std::make_unique<ChannelUpdateHandler>(std::move(telemvar)));
			}
		}

		//load config TelemVars
		for (const json& ctv_group : config.at("config_vars")) {
			const std::string name = ctv_group.at("name").get<std::string>();
			std::unique_ptr<TelemVarGroup> group = std::make_unique<TelemVarGroup>(name);

			for (std::unique_ptr<BaseTelemVar>& telemvar : parse_vars_object(ctv_group.at("vars"), "")) {
				group->insert(std::move(telemvar));
			}

			config_vars.insert(std::move(group));
		}

		//load event TelemVars
		for (const json& ctv_group : config.at("channel_vars")) {
			const std::string name = ctv_group.at("name").get<std::string>();
			std::unique_ptr<TelemVarGroup> group = std::make_unique<TelemVarGroup>(name);

			for (std::unique_ptr<BaseTelemVar>& telemvar : parse_vars_object(ctv_group.at("vars"), "")) {
				group->insert(std::move(telemvar));
			}

			event_vars.insert(std::move(group));
		}

		log_line(SCS_LOG_TYPE_message, "YATTS: Successfully loaded the config - %llu variables total", var_count);
		return true;
	}
	catch (std::exception& e) {
		log_line(SCS_LOG_TYPE_error, "YATTS: Encountered an error during loading the config: %s", e.what());
	}
	return false;
}

//TODO: reorganize init and shutdown order/cleanup of data structures and telemvars
SCSAPI_RESULT scs_telemetry_init(const scs_u32_t version, const scs_telemetry_init_params_t* const params) {
	if (version != SCS_TELEMETRY_VERSION_CURRENT) {
		return SCS_RESULT_unsupported;
	}

	const scs_telemetry_init_params_v101_t* const version_params = static_cast<const scs_telemetry_init_params_v101_t*>(params);
	game_log = version_params->common.log;

	log_line(SCS_LOG_TYPE_message, "YATTS: Initializing YATTS v" YATTS_VERSION);
	log_line(SCS_LOG_TYPE_message, 
			 "YATTS: Game '%s', SDK version %u.%u", 
			 version_params->common.game_id, 
			 SCS_GET_MAJOR_VERSION(version_params->common.game_version), 
			 SCS_GET_MINOR_VERSION(version_params->common.game_version));

	//we support all versions since adding multiple trailers to the game
	//added/missing channel is not considered as a breaking change
	if (strcmp(version_params->common.game_id, SCS_GAME_ID_EUT2) == 0) {
		if (version_params->common.game_version < SCS_TELEMETRY_EUT2_GAME_VERSION_1_14) {
			log_line(SCS_LOG_TYPE_error, "YATTS: Too old version of the game");
			scs_telemetry_shutdown();
			return SCS_RESULT_unsupported;
		}
	}
	else if (strcmp(version_params->common.game_id, SCS_GAME_ID_ATS) == 0) {
		if (version_params->common.game_version < SCS_TELEMETRY_ATS_GAME_VERSION_1_01) {
			log_line(SCS_LOG_TYPE_error, "YATTS: Too old version of the game");
			scs_telemetry_shutdown();
			return SCS_RESULT_unsupported;
		}
	}
	else {
		log_line(SCS_LOG_TYPE_error, "YATTS: Unsupported game");
		scs_telemetry_shutdown();
		return SCS_RESULT_unsupported;
	}

	bool events_registered = version_params->register_for_event(SCS_TELEMETRY_EVENT_configuration,
																telemetry_configuration,
																nullptr) == SCS_RESULT_ok;
	events_registered &= version_params->register_for_event(SCS_TELEMETRY_EVENT_gameplay,
															 telemetry_gameplay_event,
															 nullptr) == SCS_RESULT_ok;
	if (!events_registered) {
		log_line(SCS_LOG_TYPE_error, "YATTS: Unable to register event callbacks");
		scs_telemetry_shutdown();
		return SCS_RESULT_generic_error;
	}

	ChannelUpdateHandler::reg_chan = version_params->register_for_channel;
	ChannelUpdateHandler::unreg_chan = version_params->unregister_from_channel;

	//loading telemvars from config--------------------------------------------

	if (!load_config()) {
		scs_telemetry_shutdown();
		return SCS_RESULT_generic_error;
	}

	for (std::unique_ptr<ChannelUpdateHandler>& cah : channel_vars) {
		cah->reg_callbacks();
	}

	//timer registration-------------------------------------------------------
	timer_queue = CreateTimerQueue();
	if (timer_queue == NULL) {
		log_line(SCS_LOG_TYPE_error, "YATTS: Failed to create timer_queue, error: %ul", GetLastError());
		scs_telemetry_shutdown();
		return SCS_RESULT_generic_error;
	}

	HANDLE display_state_timer; //ignored, we'll shut it down by deleting the whole queue
	BOOL result = CreateTimerQueueTimer(&display_state_timer, timer_queue, display_state, NULL, 5000, 5000, WT_EXECUTEDEFAULT);
	if (!result) {
		log_line(SCS_LOG_TYPE_error, "YATTS: Failed to create timer, error: %ul", GetLastError());
		scs_telemetry_shutdown();
		return SCS_RESULT_generic_error;
	}

	//we're finished here------------------------------------------------------
	log_line(SCS_LOG_TYPE_message, "YATTS: Initialization finished");
	return SCS_RESULT_ok;
}

SCSAPI_VOID scs_telemetry_shutdown(void) {
	if (timer_queue) {
		BOOL result = DeleteTimerQueueEx(timer_queue, NULL);
		timer_queue = NULL;
		assert(result);
	}

	channel_vars.clear();
	config_vars.clear();
	event_vars.clear();
	for (auto& [key, group] : dynamic_count_vars) {
		for (auto& [key, count] : group) {
			count = 0;
		}
	}

	ChannelUpdateHandler::reg_chan = nullptr;
	ChannelUpdateHandler::unreg_chan = nullptr;
	game_log = nullptr;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason_for_call, LPVOID reseved) {
	return TRUE;
}
