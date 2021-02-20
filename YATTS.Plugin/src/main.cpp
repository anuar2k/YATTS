#include "pch.h"
#include "telemvar/TelemVar.hpp"
#include "telemvar/TelemVarSet.hpp"

#include <vector>
#include <set>
#include <map>
#include <functional>
#include <memory>
#include <fstream>

#define YATTS_MAKE_VERSION_HELPER(major, minor, patch) #major "." #minor "." #patch
#define YATTS_MAKE_VERSION(major, minor, patch) YATTS_MAKE_VERSION_HELPER(major, minor, patch)

#define YATTS_VERSION_MAJOR 0
#define YATTS_VERSION_MINOR 1
#define YATTS_VERSION_PATCH 0

#define YATTS_VERSION YATTS_MAKE_VERSION(YATTS_VERSION_MAJOR, YATTS_VERSION_MINOR, YATTS_VERSION_PATCH)

//path is relative to the game's executable
#define CONFIG_PATH "plugins\\YATTS.Config.json"

scs_log_t game_log = nullptr;
HANDLE timer_queue = NULL;

std::vector<std::shared_ptr<StreamedScalarTelemVar>> channel_vars;
std::set<std::shared_ptr<TelemVarSet>, TelemVarSet::shared_ptrCmp> config_vars;
std::set<std::shared_ptr<TelemVarSet>, TelemVarSet::shared_ptrCmp> event_vars;

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
		on_ground[i] = *reinterpret_cast<const bool*>(channel_vars[0]->get_val(i));
	}

	log_line(SCS_LOG_TYPE_message, "%s: %d %d %d %d %d %d",
			 channel_vars[0]->name.c_str(),
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
	if (auto map_search = dynamic_count_vars.find(config_info->id); map_search != dynamic_count_vars.end()) {
		auto& map = map_search->second;
		const scs_named_value_t* attributes = config_info->attributes;
		while (attributes->name) {
			if (auto to_update_search = map.find(attributes->name); to_update_search != map.end()) {
				assert(attributes->value.type == SCS_VALUE_TYPE_u32);
				to_update_search->second = attributes->value.value_u32.value;
			}
			++attributes;
		}
	}

	//refresh dynamic_count ChannelTelemVars
	for (const std::shared_ptr<StreamedScalarTelemVar>& ctv : channel_vars) {
		ctv->reg_callbacks();
	}

	//#pragma warning(suppress: 6011)
	if (auto set_search = config_vars.find(config_info->id); set_search != config_vars.end()) {
		std::shared_ptr<TelemVarSet> set = *set_search;
		set->update_set(config_info->attributes);
	}
}

SCSAPI_VOID telemetry_gameplay_event(const scs_event_t event, const void* const event_info, const scs_context_t context) {
	assert(event == SCS_TELEMETRY_EVENT_gameplay);
	const scs_telemetry_gameplay_event_t* const gp_event_info = static_cast<const scs_telemetry_gameplay_event_t*>(event_info);
	assert(gp_event_info);

	//#pragma warning(suppress: 6011)
	if (auto set_search = event_vars.find(gp_event_info->id); set_search != event_vars.end()) {
		std::shared_ptr<TelemVarSet> set = *set_search;
		set->update_set(gp_event_info->attributes);
	}
}

scs_u32_t* get_dynamic_count_ptr(std::string& set_name, std::string& var_name) {
	if (auto map_search = dynamic_count_vars.find(set_name); map_search != dynamic_count_vars.end()) {
		auto& map = map_search->second;

		if (auto dyn_count_search = map.find(var_name); dyn_count_search != map.end()) {
			return &dyn_count_search->second;
		}
	}

	return nullptr;
}

//parses the config file into channel_vars, config_vars, event_vars
bool load_config() {
	try {
		std::ifstream config_file(CONFIG_PATH);

		if (config_file.fail()) {
			throw std::exception("config file not found - it must be accessible under " CONFIG_PATH);
		}

		const json config = json::parse(config_file);
		size_t var_count = 0;

		//version check--------------------------------------------------------
		if (config.at("version").at("major").get<int>() != YATTS_VERSION_MAJOR) {
			throw std::exception("major version doesn't match");
		}

		//loading ChannelTelemVars---------------------------------------------
		for (const json& ctv_set_desc : config.at("channel_vars")) {
			std::string ctv_set_name = ctv_set_desc.at("name").get<std::string>();

			const json& ctv_list = ctv_set_desc.at("vars");
			var_count += ctv_list.size();

			for (const json& ctv_desc : ctv_list) {
				std::string name = ctv_set_name + "." + ctv_desc.at("name").get<std::string>();
				scs_value_type_t type = ctv_desc.at("type").get<scs_value_type_t>();
				scs_u32_t max_count = ctv_desc.value("max_count", SCS_U32_NIL);
				scs_u32_t* dynamic_count = nullptr;

				if (max_count != SCS_U32_NIL && ctv_desc.contains("dynamic_count")) {
					const json& dynamic_count_desc = ctv_desc.at("dynamic_count");
					std::string dynamic_count_set_name = dynamic_count_desc.at("set_name").get<std::string>();
					std::string dynamic_count_var_name = dynamic_count_desc.at("var_name").get<std::string>();
					dynamic_count = get_dynamic_count_ptr(dynamic_count_set_name, dynamic_count_var_name);
				}

				//TODO: this will not be true anymore as ChannelUpdateHandlers will be introduced
				if (type == SCS_VALUE_TYPE_string) {
					throw std::exception("channel string variables are not supported");
				}

				channel_vars.emplace_back(std::make_shared<StreamedScalarTelemVar>(name, max_count, dynamic_count, type));
			}
		}

		//loading config and event TelemVars-----------------------------------
		auto parse_telemvar_sets = [&var_count](const json& source, std::set<std::shared_ptr<TelemVarSet>, TelemVarSet::shared_ptrCmp>& target) {
			for (const json& tv_set_desc : source) {
				std::string tv_set_name = tv_set_desc.at("name").get<std::string>();
				std::shared_ptr<TelemVarSet> tvs = std::make_shared<TelemVarSet>(tv_set_name);

				const json& tv_list = tv_set_desc.at("vars");
				var_count += tv_list.size();

				for (const json& tv_desc : tv_list) {
					std::string name = tv_desc.at("name").get<std::string>();
					scs_value_type_t type = tv_desc.at("type").get<scs_value_type_t>();
					scs_u32_t max_count = tv_desc.value("max_count", SCS_U32_NIL);
					scs_u32_t* dynamic_count = nullptr;

					if (max_count != SCS_U32_NIL && tv_desc.contains("dynamic_count")) {
						const json& dynamic_count_desc = tv_desc.at("dynamic_count");
						std::string dynamic_count_set_name = dynamic_count_desc.at("set_name").get<std::string>();
						std::string dynamic_count_var_name = dynamic_count_desc.at("var_name").get<std::string>();
						dynamic_count = get_dynamic_count_ptr(dynamic_count_set_name, dynamic_count_var_name);
					}

					if (type == SCS_VALUE_TYPE_string) {
						size_t truncate_nullpad = tv_desc.at("truncate_nullpad").get<size_t>();

						if (truncate_nullpad == 0) {
							throw std::exception("truncate_nullpad parameter must not be 0");
						}

						tvs->insert(std::make_shared<StringTelemVar>(name, max_count, dynamic_count, truncate_nullpad));
					}
					else {
						tvs->insert(std::make_shared<ScalarTelemVar>(name, max_count, dynamic_count, type));
					}
				}

				target.insert(tvs);
			}
		};

		parse_telemvar_sets(config.at("config_vars"), config_vars);
		parse_telemvar_sets(config.at("event_vars"), event_vars);

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
	log_line(SCS_LOG_TYPE_message, "YATTS: Game '%s', SDK version %u.%u", version_params->common.game_id, SCS_GET_MAJOR_VERSION(version_params->common.game_version), SCS_GET_MINOR_VERSION(version_params->common.game_version));

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

	bool events_registered =
		(version_params->register_for_event(SCS_TELEMETRY_EVENT_configuration, telemetry_configuration, nullptr) == SCS_RESULT_ok);
		(version_params->register_for_event(SCS_TELEMETRY_EVENT_gameplay, telemetry_gameplay_event, nullptr) == SCS_RESULT_ok);

	if (!events_registered) {
		log_line(SCS_LOG_TYPE_error, "YATTS: Unable to register event callbacks");
		scs_telemetry_shutdown();
		return SCS_RESULT_generic_error;
	}

	StreamedScalarTelemVar::reg_chan = version_params->register_for_channel;
	StreamedScalarTelemVar::unreg_chan = version_params->unregister_from_channel;

	//loading telemvars from config--------------------------------------------

	if (!load_config()) {
		scs_telemetry_shutdown();
		return SCS_RESULT_generic_error;
	}

	for (const std::shared_ptr<StreamedScalarTelemVar>& ctv : channel_vars) {
		ctv->reg_callbacks();
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
		BOOL result;

		result = DeleteTimerQueueEx(timer_queue, NULL);
		timer_queue = NULL;
		assert(result);
	}

	//make sure, that we're not leaving any instances of CTV with a callback registered...
	//it shouldn't happen though - at this point the only valid shared_ptr to them should be in vector below
	for (const std::shared_ptr<StreamedScalarTelemVar>& ctv : channel_vars) {
		ctv->unreg_callbacks();
	}
	channel_vars.clear();
	config_vars.clear();
	event_vars.clear();

	StreamedScalarTelemVar::reg_chan = nullptr;
	StreamedScalarTelemVar::unreg_chan = nullptr;
	game_log = nullptr;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason_for_call, LPVOID reseved) {
	return TRUE;
}
