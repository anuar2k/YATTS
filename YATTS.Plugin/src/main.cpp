#include "pch.h"
#include "TelemVar.hpp"
#include "TelemVarSet.hpp"

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

#define CONFIG_FILENAME "YATTS.Config.json"

scs_log_t game_log = nullptr;
HANDLE timer_queue = NULL;

std::vector<std::shared_ptr<ChannelTelemVar>> channel_vars;
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
	std::vector<char> bytebuffer;

	std::shared_ptr<TelemVarSet> truck_set = *config_vars.find(SCS_TELEMETRY_CONFIG_truck);
	std::shared_ptr<TelemVarSet> trailer_set = *config_vars.find(SCS_TELEMETRY_CONFIG_trailer);
	std::shared_ptr<TelemVarSet> player_fined_set = *event_vars.find(SCS_TELEMETRY_GAMEPLAY_EVENT_player_fined);

	bool lblinker = *reinterpret_cast<const bool*>(channel_vars[0]->get_val(SCS_U32_NIL));
	bool rblinker = *reinterpret_cast<const bool*>(channel_vars[1]->get_val(SCS_U32_NIL));
	float speed = *reinterpret_cast<const float*>(channel_vars[2]->get_val(SCS_U32_NIL)) * 3.6f;
	log_line(SCS_LOG_TYPE_message, "channels - lblinker: %s, rblinker: %s, speed: %f",
			 (lblinker ? "on" : "off"),
			 (rblinker ? "on" : "off"),
			 speed);

	const char* truck_id = reinterpret_cast<const char*>((*truck_set)[0]->get_val(SCS_U32_NIL));
	const char* license_plate = reinterpret_cast<const char*>((*truck_set)[1]->get_val(SCS_U32_NIL));
	log_line(SCS_LOG_TYPE_message, "truck_cfg - %s, %s", truck_id, license_plate);

	scs_u32_t wheel_count = *reinterpret_cast<const scs_u32_t*>((*trailer_set)[0]->get_val(SCS_U32_NIL));
	const char* trailer_id = reinterpret_cast<const char*>((*trailer_set)[1]->get_val(SCS_U32_NIL));
	log_line(SCS_LOG_TYPE_message, "trailer_cfg - %d, %s", wheel_count, trailer_id);

	const char* fine_offence = reinterpret_cast<const char*>((*player_fined_set)[0]->get_val(SCS_U32_NIL));
	scs_s64_t fine_amount = *reinterpret_cast<const scs_s64_t*>((*player_fined_set)[1]->get_val(SCS_U32_NIL));
	log_line(SCS_LOG_TYPE_message, "fine - %s, %d", fine_offence, fine_amount);
}

SCSAPI_VOID telemetry_configuration(const scs_event_t event, const void* const event_info, const scs_context_t context) {
	assert(event == SCS_TELEMETRY_EVENT_configuration);
	const scs_telemetry_configuration_t* const config_info = static_cast<const scs_telemetry_configuration_t*>(event_info);
	assert(config_info);

	//update dynamic count
	#pragma warning(suppress: 6011)
	auto dyn_count_set_search = dynamic_count_vars.find(config_info->id);
	if (dyn_count_set_search != dynamic_count_vars.end()) {
		auto& map = dyn_count_set_search->second;
		const scs_named_value_t* attributes = config_info->attributes;
		while (attributes->name) {
			auto to_update_search = map.find(attributes->name);
			if (to_update_search != map.end()) {
				assert(attributes->value.type == SCS_VALUE_TYPE_u32);
				to_update_search->second = attributes->value.value_u32.value;
			}
			++attributes;
		}
	}

	#pragma warning(suppress: 6011)
	auto set_search = config_vars.find(config_info->id);

	if (set_search != config_vars.end()) {
		std::shared_ptr<TelemVarSet> set = *set_search;
		set->update_set(config_info->attributes);
	}
}

SCSAPI_VOID telemetry_gameplay_event(const scs_event_t event, const void* const event_info, const scs_context_t context) {
	assert(event == SCS_TELEMETRY_EVENT_gameplay);
	const scs_telemetry_gameplay_event_t* const gp_event_info = static_cast<const scs_telemetry_gameplay_event_t*>(event_info);
	assert(gp_event_info);

	#pragma warning(suppress: 6011)
	auto set_search = event_vars.find(gp_event_info->id);

	if (set_search != event_vars.end()) {
		std::shared_ptr<TelemVarSet> set = *set_search;
		set->update_set(gp_event_info->attributes);
	}
}

scs_u32_t* get_dynamic_count_ptr(std::string& dynamic_count_set_name, std::string& dynamic_count_var_name) {
	if (!dynamic_count_set_name.empty() && !dynamic_count_var_name.empty()) {
		auto dyn_count_set_search = dynamic_count_vars.find(dynamic_count_set_name);

		if (dyn_count_set_search != dynamic_count_vars.end()) {
			auto& map = dyn_count_set_search->second;

			auto dyn_count_search = map.find(dynamic_count_var_name);
			if (dyn_count_search != map.end()) {
				return &dyn_count_search->second;
			}
		}
	}

	return nullptr;
}

//parses the config file into channel_vars, config_vars, event_vars
bool load_config() {
	try {
		std::ifstream config_file(CONFIG_FILENAME);

		if (config_file.fail()) {
			throw std::exception("file " CONFIG_FILENAME " does not exist");
		}

		const json config = json::parse(config_file);
		size_t var_count = 0;

		//version check--------------------------------------------------------
		if (config.at("version").at("major").get<int>() != YATTS_VERSION_MAJOR) {
			throw std::exception("major version doesn't match");
		}

		//loading ChannelTelemVars---------------------------------------------
		const json& ctv_list = config.at("channel_vars");
		var_count += ctv_list.size();

		for (const json& ctv_desc : ctv_list) {
			std::string name = ctv_desc.at("name").get<std::string>();
			scs_value_type_t type = ctv_desc.at("type").get<scs_value_type_t>();
			scs_u32_t max_count = ctv_desc.value("max_count", SCS_U32_NIL);
			scs_u32_t* dynamic_count = nullptr;

			if (max_count != SCS_U32_NIL) {
				std::string dynamic_count_set_name = ctv_desc.value("dynamic_count_set_name", std::string());
				std::string dynamic_count_var_name = ctv_desc.value("dynamic_count_var_name", std::string());
				dynamic_count = get_dynamic_count_ptr(dynamic_count_set_name, dynamic_count_var_name);
			}

			channel_vars.emplace_back(std::make_shared<ChannelTelemVar>(name, type, max_count, dynamic_count));
		}

		//loading config and event TelemVars-----------------------------------
		auto parse_telemvar_sets = [&var_count](const json& source, std::set<std::shared_ptr<TelemVarSet>, TelemVarSet::shared_ptrCmp>& target) {
			for (const json& tv_set_desc : source) {
				std::string tv_set_name = tv_set_desc.at("name");
				std::shared_ptr<TelemVarSet> tvs = std::make_shared<TelemVarSet>(tv_set_name);

				const json& tv_list = tv_set_desc.at("vars");
				var_count += tv_list.size();

				for (const json& tv_desc : tv_list) {
					std::string name = tv_desc.at("name").get<std::string>();
					scs_value_type_t type = tv_desc.at("type").get<scs_value_type_t>();
					scs_u32_t max_count = tv_desc.value("max_count", SCS_U32_NIL);
					scs_u32_t* dynamic_count = nullptr;

					if (max_count != SCS_U32_NIL) {
						std::string dynamic_count_set_name = tv_desc.value("dynamic_count_set_name", std::string());
						std::string dynamic_count_var_name = tv_desc.value("dynamic_count_var_name", std::string());
						dynamic_count = get_dynamic_count_ptr(dynamic_count_set_name, dynamic_count_var_name);
					}

					if (type == SCS_VALUE_TYPE_string) {
						size_t truncate_nullpad = tv_desc.at("truncate_nullpad").get<size_t>();

						if (truncate_nullpad == 0) {
							throw new std::exception("truncate_nullpad parameter must not be 0");
						}

						tvs->insert(std::make_shared<StringTelemVar>(name, truncate_nullpad, max_count, dynamic_count));
					}
					else {
						tvs->insert(std::make_shared<ScalarTelemVar>(name, type, max_count, dynamic_count));
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
	log_line(SCS_LOG_TYPE_message, "YATTS: Game '%s', version %u.%u", version_params->common.game_id, SCS_GET_MAJOR_VERSION(version_params->common.game_version), SCS_GET_MINOR_VERSION(version_params->common.game_version));

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

	ChannelTelemVar::reg_chan = version_params->register_for_channel;
	ChannelTelemVar::unreg_chan = version_params->unregister_from_channel;

	//loading telemvars from config--------------------------------------------

	if (!load_config()) {
		scs_telemetry_shutdown();
		return SCS_RESULT_generic_error;
	}

	for (const std::shared_ptr<ChannelTelemVar>& ctv : channel_vars) {
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
	log_line(SCS_LOG_TYPE_message, "YATTS: Initialized");
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
	for (const std::shared_ptr<ChannelTelemVar>& ctv : channel_vars) {
		ctv->unreg_callbacks();
	}
	channel_vars.clear();
	config_vars.clear();
	event_vars.clear();

	ChannelTelemVar::reg_chan = nullptr;
	ChannelTelemVar::unreg_chan = nullptr;
	game_log = nullptr;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason_for_call, LPVOID reseved) {
	return TRUE;
}
