#include "pch.h"
#include "TelemVar.hpp"
#include "TelemVarSet.hpp"

// JSON for Modern C++
#include "nlohmann/json.hpp"

#include <vector>
#include <set>
#include <map>
#include <functional>
#include <memory>

scs_log_t game_log = nullptr;
HANDLE timer_queue = NULL;

std::vector<std::shared_ptr<ChannelTelemVar>> channel_vars;
std::set<std::shared_ptr<TelemVarSet>, TelemVarSet::shared_ptrCmp> config_vars;
std::set<std::shared_ptr<TelemVarSet>, TelemVarSet::shared_ptrCmp> event_vars;

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
		std::map<std::string, scs_u32_t, std::less<>>& set = dyn_count_set_search->second;
		const scs_named_value_t* attributes = config_info->attributes;
		while (attributes->name) {
			auto to_update_search = set.find(attributes->name);
			if (to_update_search != set.end()) {
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

//TODO: reorganize init and shutdown order/cleanup of data structures and telemvars
SCSAPI_RESULT scs_telemetry_init(const scs_u32_t version, const scs_telemetry_init_params_t* const params) {
	if (version != SCS_TELEMETRY_VERSION_1_01) {
		return SCS_RESULT_unsupported;
	}

	const scs_telemetry_init_params_v101_t* const version_params = static_cast<const scs_telemetry_init_params_v101_t*>(params);
	game_log = version_params->common.log;

	log_line(SCS_LOG_TYPE_message, "Game '%s' %u.%u", version_params->common.game_id, SCS_GET_MAJOR_VERSION(version_params->common.game_version), SCS_GET_MINOR_VERSION(version_params->common.game_version));

	if (strcmp(version_params->common.game_id, SCS_GAME_ID_EUT2) == 0) {
		if (version_params->common.game_version < SCS_TELEMETRY_EUT2_GAME_VERSION_1_15) {
			log_line(SCS_LOG_TYPE_error, "Too old version of the game");
			scs_telemetry_shutdown();
			return SCS_RESULT_unsupported;
		}
	}
	else if (strcmp(version_params->common.game_id, SCS_GAME_ID_ATS) == 0) {
		if (version_params->common.game_version < SCS_TELEMETRY_ATS_GAME_VERSION_1_02) {
			log_line(SCS_LOG_TYPE_error, "Too old version of the game");
			scs_telemetry_shutdown();
			return SCS_RESULT_unsupported;
		}
	}
	else {
		log_line(SCS_LOG_TYPE_warning, "Unsupported game, some features or values might behave incorrectly");
	}

	bool events_registered =
		(version_params->register_for_event(SCS_TELEMETRY_EVENT_configuration, telemetry_configuration, nullptr) == SCS_RESULT_ok);
		(version_params->register_for_event(SCS_TELEMETRY_EVENT_gameplay, telemetry_gameplay_event, nullptr) == SCS_RESULT_ok);

	if (!events_registered) {
		log_line(SCS_LOG_TYPE_error, "Unable to register event callbacks");
		scs_telemetry_shutdown();
		return SCS_RESULT_generic_error;
	}
	//////////////////////////////////////////////////////////////////////
	std::shared_ptr<TelemVarSet> truck_set = std::make_shared<TelemVarSet>(SCS_TELEMETRY_CONFIG_truck);
	truck_set->insert(std::make_shared<StringTelemVar>(SCS_TELEMETRY_CONFIG_ATTRIBUTE_id));
	truck_set->insert(std::make_shared<StringTelemVar>(SCS_TELEMETRY_CONFIG_ATTRIBUTE_license_plate));
	config_vars.insert(truck_set);

	std::shared_ptr<TelemVarSet> trailer_set = std::make_shared<TelemVarSet>(SCS_TELEMETRY_CONFIG_trailer);
	trailer_set->insert(std::make_shared<ScalarTelemVar>(SCS_TELEMETRY_CONFIG_ATTRIBUTE_wheel_count, SCS_VALUE_TYPE_u32));
	trailer_set->insert(std::make_shared<StringTelemVar>(SCS_TELEMETRY_CONFIG_ATTRIBUTE_id));
	config_vars.insert(trailer_set);

	std::shared_ptr<TelemVarSet> player_fined_set = std::make_shared<TelemVarSet>(SCS_TELEMETRY_GAMEPLAY_EVENT_player_fined);
	player_fined_set->insert(std::make_shared<StringTelemVar>(SCS_TELEMETRY_GAMEPLAY_EVENT_ATTRIBUTE_fine_offence));
	player_fined_set->insert(std::make_shared<ScalarTelemVar>(SCS_TELEMETRY_GAMEPLAY_EVENT_ATTRIBUTE_fine_amount, SCS_VALUE_TYPE_s64));
	event_vars.insert(player_fined_set);

	//////////////////////////////////////////////////////////////////////
	ChannelTelemVar::reg_chan = version_params->register_for_channel;
	ChannelTelemVar::unreg_chan = version_params->unregister_from_channel;

	channel_vars.emplace_back(std::make_unique<ChannelTelemVar>(SCS_TELEMETRY_TRUCK_CHANNEL_lblinker, SCS_VALUE_TYPE_bool));
	channel_vars.emplace_back(std::make_unique<ChannelTelemVar>(SCS_TELEMETRY_TRUCK_CHANNEL_rblinker, SCS_VALUE_TYPE_bool));
	channel_vars.emplace_back(std::make_unique<ChannelTelemVar>(SCS_TELEMETRY_TRUCK_CHANNEL_speed, SCS_VALUE_TYPE_float));

	for (const std::shared_ptr<ChannelTelemVar>& ctv : channel_vars) {
		ctv->reg_callbacks();
	}
	//////////////////////////////////////////////////////////////////////
	timer_queue = CreateTimerQueue();
	if (timer_queue == NULL) {
		log_line(SCS_LOG_TYPE_error, "Failed to create timer_queue, error: %ul", GetLastError());
		scs_telemetry_shutdown();
		return SCS_RESULT_generic_error;
	}

	HANDLE display_state_timer; //ignored, we'll shut it down by deleting the whole queue
	BOOL result = CreateTimerQueueTimer(&display_state_timer, timer_queue, display_state, NULL, 5000, 5000, WT_EXECUTEDEFAULT);
	if (!result) {
		log_line(SCS_LOG_TYPE_error, "failed to create timer, error: %ul", GetLastError());
		scs_telemetry_shutdown();
		return SCS_RESULT_generic_error;
	}
	//////////////////////////////////////////////////////////////////////
	log_line(SCS_LOG_TYPE_message, "YATTS initialized");
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
