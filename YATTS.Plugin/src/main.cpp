#include "pch.h"

scs_telemetry_register_for_channel_t register_for_channel = NULL;
scs_telemetry_unregister_from_channel_t unregister_from_channel = NULL;
scs_log_t game_log = NULL;

void log_line(const scs_log_type_t type, const char* const text, ...) {
	if (!game_log) {
		return;
	}
	char formated[1000];

	va_list args;
	va_start(args, text);
	vsnprintf_s(formated, sizeof(formated), _TRUNCATE, text, args);
	formated[sizeof(formated) - 1] = 0;
	va_end(args);

	game_log(type, formated);
}

SCSAPI_VOID telemetry_store_float(const scs_string_t name, const scs_u32_t index, const scs_value_t* const value, const scs_context_t context) {
	assert(context);
	scs_float_t* const storage = static_cast<scs_float_t*>(context);

	if (value) {
		assert(value->type == SCS_VALUE_TYPE_float);
		*storage = value->value_float.value;
	}
	else {
		*storage = 0.0f;
	}
}

const scs_named_value_t* find_attribute(const scs_telemetry_configuration_t& configuration, const char* const name, const scs_u32_t index, const scs_value_type_t expected_type) {
	for (const scs_named_value_t* current = configuration.attributes; current->name; ++current) {
		if ((current->index != index) || (strcmp(current->name, name) != 0)) {
			continue;
		}
		if (current->value.type == expected_type) {
			return current;
		}
		log_line(SCS_LOG_TYPE_error, "Attribute %s has unexpected type %u", name, static_cast<unsigned>(current->value.type));
		break;
	}
	return NULL;
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

	register_for_channel = version_params->register_for_channel;
	unregister_from_channel = version_params->unregister_from_channel;

	log_line(SCS_LOG_TYPE_message, "Memory telemetry example initialized");
	return SCS_RESULT_ok;
}

SCSAPI_VOID scs_telemetry_shutdown(void) {

	unregister_from_channel = NULL;
	register_for_channel = NULL;
	game_log = NULL;
}


BOOL APIENTRY DllMain(HMODULE module, DWORD  reason_for_call, LPVOID reseved) {
	return TRUE;
}

