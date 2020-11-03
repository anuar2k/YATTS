#pragma once
#include "../pch.h"
#include "BaseTelemVar.hpp"

SCSAPI_VOID chan_callback(const scs_string_t name, const scs_u32_t index, const scs_value_t* const value, const scs_context_t context);

class ChannelUpdateHandler abstract {
	public:

	ChannelUpdateHandler(BaseTelemVar& tv) : tv(tv) {

	}

	virtual ~ChannelUpdateHandler() {
		unreg_callbacks();
	}

	//can be called multiple times to adjust the number of registered channels based on dynamic_count
	void reg_callbacks() {
		if (tv.dynamic_count) {
			while (reg_chan_cnt < *tv.dynamic_count && reg_chan_cnt < tv.max_count) {
				reg_chan(tv.name.c_str(), reg_chan_cnt, tv.type, SCS_TELEMETRY_CHANNEL_FLAG_none, chan_callback, this);
				++reg_chan_cnt;
			}
			while (reg_chan_cnt > *tv.dynamic_count && reg_chan_cnt > 0) {
				--reg_chan_cnt;
				unreg_chan(tv.name.c_str(), reg_chan_cnt, tv.type);
			}
		}
		else {
			if (reg_chan_cnt == 0) {
				if (tv.max_count == SCS_U32_NIL) {
					reg_chan(tv.name.c_str(), SCS_U32_NIL, tv.type, SCS_TELEMETRY_CHANNEL_FLAG_none, chan_callback, this);
				}
				else {
					for (scs_u32_t i = 0; i < tv.max_count; ++i) {
						reg_chan(tv.name.c_str(), i, tv.type, SCS_TELEMETRY_CHANNEL_FLAG_none, chan_callback, this);
					}
				}
				reg_chan_cnt = tv.max_count;
			}
		}
	}

	void unreg_callbacks() {
		if (reg_chan_cnt != 0) {
			if (tv.max_count == SCS_U32_NIL) {
				unreg_chan(tv.name.c_str(), SCS_U32_NIL, tv.type);
			}
			else {
				while (reg_chan_cnt > 0) {
					--reg_chan_cnt;
					unreg_chan(tv.name.c_str(), reg_chan_cnt, tv.type);
				}
			}
		}
	}

	static scs_telemetry_register_for_channel_t reg_chan;
	static scs_telemetry_unregister_from_channel_t unreg_chan;

	BaseTelemVar& tv;

	protected:
	scs_u32_t reg_chan_cnt = 0;
};

scs_telemetry_register_for_channel_t ChannelUpdateHandler::reg_chan = nullptr;
scs_telemetry_unregister_from_channel_t ChannelUpdateHandler::unreg_chan = nullptr;

SCSAPI_VOID chan_callback(const scs_string_t name, const scs_u32_t index, const scs_value_t* const value, const scs_context_t context) {
	ChannelUpdateHandler* const ch = static_cast<ChannelUpdateHandler*>(context);
	assert(value && ch && value->type == ch->tv.type);

	#pragma warning(suppress: 6011)
	ch->tv.store_value(*value, index);
}