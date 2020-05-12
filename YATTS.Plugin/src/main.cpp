#include "pch.h"
#include "TelemVar.hpp"
#include <vector>
#include <set>

scs_log_t game_log = nullptr;
HANDLE timer_queue = NULL;

std::vector<ChannelTelemVar*> channels;
std::set<TelemVar> telemvars;

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

	for (ChannelTelemVar* ctv : channels) {
		ctv->write_to_buf(bytebuffer);
	}

	assert(bytebuffer.size() == 6);

	float speed = *reinterpret_cast<float*>(&bytebuffer[2]) * 3.6f;

	log_line(SCS_LOG_TYPE_message, "lblinker: %s, rblinker: %s, speed: %f", 
			 (bytebuffer[0] ? "on" : "off"), 
			 (bytebuffer[1] ? "on" : "off"), 
			 speed);
}

SCSAPI_RESULT scs_telemetry_init(const scs_u32_t version, const scs_telemetry_init_params_t* const params) {
	if (version != SCS_TELEMETRY_VERSION_1_00) {
		return SCS_RESULT_unsupported;
	}
	const scs_telemetry_init_params_v100_t* const version_params = static_cast<const scs_telemetry_init_params_v100_t*>(params);
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

	ChannelTelemVar::reg_chan = version_params->register_for_channel;
	ChannelTelemVar::unreg_chan = version_params->unregister_from_channel;

	channels.push_back(new ChannelTelemVar(SCS_TELEMETRY_TRUCK_CHANNEL_lblinker, SCS_VALUE_TYPE_bool));
	channels.push_back(new ChannelTelemVar(SCS_TELEMETRY_TRUCK_CHANNEL_rblinker, SCS_VALUE_TYPE_bool));
	channels.push_back(new ChannelTelemVar(SCS_TELEMETRY_TRUCK_CHANNEL_speed, SCS_VALUE_TYPE_float));

	for (ChannelTelemVar* ctv : channels) {
		ctv->adjust_channels();
	}

	timer_queue = CreateTimerQueue();
	if (timer_queue == NULL) {
		log_line(SCS_LOG_TYPE_error, "Failed to create timer_queue, error: %ul", GetLastError());
		return SCS_RESULT_generic_error;
	}

	HANDLE display_state_timer; //ignored, we'll shut it down by deleting the whole queue
	BOOL result = CreateTimerQueueTimer(&display_state_timer, timer_queue, display_state, NULL, 0, 5000, WT_EXECUTEDEFAULT);
	if (!result) {
		log_line(SCS_LOG_TYPE_error, "failed to create timer, error %ul", GetLastError());
		return SCS_RESULT_generic_error;
	}

	log_line(SCS_LOG_TYPE_message, "YATTS initialized");
	return SCS_RESULT_ok;
}

SCSAPI_VOID scs_telemetry_shutdown(void) {
	BOOL result;

	result = DeleteTimerQueueEx(timer_queue, NULL);
	timer_queue = NULL;
	assert(result);

	for (ChannelTelemVar* ctv : channels) {
		delete ctv;
	}
	channels.clear();

	ChannelTelemVar::reg_chan = nullptr;
	ChannelTelemVar::unreg_chan = nullptr;
	game_log = nullptr;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason_for_call, LPVOID reseved) {
	return TRUE;
}
