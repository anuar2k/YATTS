#include "pch.h"
#include "TelemVar.hpp"
#include "TelemVarSet.hpp"
#include <vector>
#include <set>
#include <map>

scs_log_t game_log = nullptr;
HANDLE timer_queue = NULL;

std::vector<ChannelTelemVar*> channel_vars;
std::set<TelemVarSet*, TelemVarSetPtrCmp> config_vars;
std::set<TelemVarSet*, TelemVarSetPtrCmp> event_vars;

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

	TelemVarSet* truck_set = *config_vars.find(SCS_TELEMETRY_CONFIG_truck);
	TelemVarSet* trailer_set = *config_vars.find(SCS_TELEMETRY_CONFIG_trailer);
	TelemVarSet* player_fined_set = *event_vars.find(SCS_TELEMETRY_GAMEPLAY_EVENT_player_fined);

	for (ChannelTelemVar* ctv : channel_vars) {
		ctv->write_to_buf(bytebuffer);
	}
	for (TelemVar* tv : *truck_set) {
		tv->write_to_buf(bytebuffer);
	}
	for (TelemVar* tv : *trailer_set) {
		tv->write_to_buf(bytebuffer);
	}
	for (TelemVar* tv : *player_fined_set) {
		tv->write_to_buf(bytebuffer);
	}

	float speed = *reinterpret_cast<float*>(&bytebuffer[2]) * 3.6f;

	log_line(SCS_LOG_TYPE_message, "channels - lblinker: %s, rblinker: %s, speed: %f", 
			 (bytebuffer[0] ? "on" : "off"), 
			 (bytebuffer[1] ? "on" : "off"), 
			 speed);

	char* pos = &bytebuffer[6];
	char* pos1 = pos;
	char* pos2 = pos1 + strlen(pos1) + 1;
	char* pos3 = pos2 + strlen(pos2) + 1;
	char* pos4 = pos3 + sizeof(scs_value_u32_t);
	char* pos5 = pos4 + strlen(pos4) + 1;
	char* pos6 = pos5 + strlen(pos5) + 1;

	scs_u32_t wheel_count = *reinterpret_cast<scs_u32_t*>(pos3);
	scs_u64_t fine = *reinterpret_cast<scs_u64_t*>(pos6);

	log_line(SCS_LOG_TYPE_message, "truck_cfg - %s, %s", pos1, pos2);
	log_line(SCS_LOG_TYPE_message, "trailer_cfg - %d, %s", wheel_count, pos4);
	log_line(SCS_LOG_TYPE_message, "fine - %s, %d", pos5, fine);
}

SCSAPI_VOID telemetry_configuration(const scs_event_t event, const void* const event_info, const scs_context_t context) {
	assert(event == SCS_TELEMETRY_EVENT_configuration);
	const scs_telemetry_configuration_t* config_info = static_cast<const scs_telemetry_configuration_t*>(event_info);
	assert(config_info);

	#pragma warning(suppress: 6011)
	std::set<TelemVarSet*, TelemVarSetPtrCmp>::iterator set_search = config_vars.find(config_info->id);

	if (set_search != config_vars.end()) {
		TelemVarSet* set = *set_search;
		set->update_set(config_info->attributes);
	}
}

SCSAPI_VOID telemetry_gameplay_event(const scs_event_t event, const void* const event_info, const scs_context_t context) {
	assert(event == SCS_TELEMETRY_EVENT_gameplay);
	const scs_telemetry_gameplay_event_t* gp_event_info = static_cast<const scs_telemetry_gameplay_event_t*>(event_info);
	assert(gp_event_info);

	#pragma warning(suppress: 6011)
	std::set<TelemVarSet*, TelemVarSetPtrCmp>::iterator set_search = event_vars.find(gp_event_info->id);

	if (set_search != event_vars.end()) {
		TelemVarSet* set = *set_search;
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

		if (version_params->common.game_version < SCS_TELEMETRY_EUT2_GAME_VERSION_1_03) {
			log_line(SCS_LOG_TYPE_error, "Too old version of the game");
			game_log = NULL;
			return SCS_RESULT_unsupported;
		}

		if (version_params->common.game_version < SCS_TELEMETRY_EUT2_GAME_VERSION_1_07) {
			log_line(SCS_LOG_TYPE_warning, "This version of the game has less precise output of angular acceleration of the cabin");
		}

		const scs_u32_t IMPLEMENTED_VERSION = SCS_TELEMETRY_EUT2_GAME_VERSION_CURRENT;
		if (SCS_GET_MAJOR_VERSION(version_params->common.game_version) > SCS_GET_MAJOR_VERSION(IMPLEMENTED_VERSION)) {
			log_line(SCS_LOG_TYPE_warning, "Too new major version of the game, some features might behave incorrectly");
		}
	}
	else if (strcmp(version_params->common.game_id, SCS_GAME_ID_ATS) == 0) {

		const scs_u32_t MINIMAL_VERSION = SCS_TELEMETRY_ATS_GAME_VERSION_1_00;
		if (version_params->common.game_version < MINIMAL_VERSION) {
			log_line(SCS_LOG_TYPE_warning, "WARNING: Too old version of the game, some features might behave incorrectly");
		}

		const scs_u32_t IMPLEMENTED_VERSION = SCS_TELEMETRY_ATS_GAME_VERSION_CURRENT;
		if (SCS_GET_MAJOR_VERSION(version_params->common.game_version) > SCS_GET_MAJOR_VERSION(IMPLEMENTED_VERSION)) {
			log_line(SCS_LOG_TYPE_warning, "WARNING: Too new major version of the game, some features might behave incorrectly");
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
		game_log = nullptr;
		return SCS_RESULT_generic_error;
	}

	ChannelTelemVar::reg_chan = version_params->register_for_channel;
	ChannelTelemVar::unreg_chan = version_params->unregister_from_channel;

	channel_vars.push_back(new ChannelTelemVar(SCS_TELEMETRY_TRUCK_CHANNEL_lblinker, SCS_VALUE_TYPE_bool));
	channel_vars.push_back(new ChannelTelemVar(SCS_TELEMETRY_TRUCK_CHANNEL_rblinker, SCS_VALUE_TYPE_bool));
	channel_vars.push_back(new ChannelTelemVar(SCS_TELEMETRY_TRUCK_CHANNEL_speed, SCS_VALUE_TYPE_float));

	for (ChannelTelemVar* ctv : channel_vars) {
		ctv->adjust_channels();
	}

	timer_queue = CreateTimerQueue();
	if (timer_queue == NULL) {
		log_line(SCS_LOG_TYPE_error, "Failed to create timer_queue, error: %ul", GetLastError());
		return SCS_RESULT_generic_error;
	}

	HANDLE display_state_timer; //ignored, we'll shut it down by deleting the whole queue
	BOOL result = CreateTimerQueueTimer(&display_state_timer, timer_queue, display_state, NULL, 5000, 5000, WT_EXECUTEDEFAULT);
	if (!result) {
		log_line(SCS_LOG_TYPE_error, "failed to create timer, error: %ul", GetLastError());
		return SCS_RESULT_generic_error;
	}

	TelemVarSet* truck_set = new TelemVarSet(SCS_TELEMETRY_CONFIG_truck);
	truck_set->insert(new StringTelemVar(SCS_TELEMETRY_CONFIG_ATTRIBUTE_id));
	truck_set->insert(new StringTelemVar(SCS_TELEMETRY_CONFIG_ATTRIBUTE_license_plate));
	config_vars.insert(truck_set);

	TelemVarSet* trailer_set = new TelemVarSet(SCS_TELEMETRY_CONFIG_trailer);
	trailer_set->insert(new ScalarTelemVar(SCS_TELEMETRY_CONFIG_ATTRIBUTE_wheel_count, SCS_VALUE_TYPE_u32));
	trailer_set->insert(new StringTelemVar(SCS_TELEMETRY_CONFIG_ATTRIBUTE_id));
	config_vars.insert(trailer_set);

	TelemVarSet* player_fined_set = new TelemVarSet(SCS_TELEMETRY_GAMEPLAY_EVENT_player_fined);
	player_fined_set->insert(new StringTelemVar(SCS_TELEMETRY_GAMEPLAY_EVENT_ATTRIBUTE_fine_offence));
	player_fined_set->insert(new ScalarTelemVar(SCS_TELEMETRY_GAMEPLAY_EVENT_ATTRIBUTE_fine_amount, SCS_VALUE_TYPE_s64));
	event_vars.insert(player_fined_set);

	log_line(SCS_LOG_TYPE_message, "YATTS initialized");
	return SCS_RESULT_ok;
}

SCSAPI_VOID scs_telemetry_shutdown(void) {
	BOOL result;

	result = DeleteTimerQueueEx(timer_queue, NULL);
	timer_queue = NULL;
	assert(result);

	for (ChannelTelemVar* ctv : channel_vars) {
		delete ctv;
	}
	channel_vars.clear();

	for (TelemVarSet* tvs : config_vars) {
		delete tvs;
	}
	config_vars.clear();

	for (TelemVarSet* tvs : event_vars) {
		delete tvs;
	}
	event_vars.clear();

	ChannelTelemVar::reg_chan = nullptr;
	ChannelTelemVar::unreg_chan = nullptr;
	game_log = nullptr;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason_for_call, LPVOID reseved) {
	return TRUE;
}
