#pragma once
#include "../pch.h"
#include "BaseTelemVar.hpp"

#include <memory>

SCSAPI_VOID chan_callback(const scs_string_t name, 
						  const scs_u32_t index, 
						  const scs_value_t* const value, 
						  const scs_context_t context);

class ChannelUpdateHandler {
	public:

	ChannelUpdateHandler(std::unique_ptr<BaseTelemVar> telemvar) : telemvar(std::move(telemvar)) {

	}

	~ChannelUpdateHandler() {
		unreg_callbacks();
	}

	//this object is non copyable nor movable, because a pointer to it will be passed to the game
	//we don't want to move this object around
	ChannelUpdateHandler& operator=(ChannelUpdateHandler&) = delete;
	ChannelUpdateHandler(ChannelUpdateHandler&) = delete;

	//can be called multiple times to adjust the number of registered channels based on dynamic_count
	void reg_callbacks() {
		if (telemvar->dynamic_count) {
			while (reg_chan_cnt < *telemvar->dynamic_count && reg_chan_cnt < telemvar->max_count) {
				reg_chan(telemvar->name.c_str(), reg_chan_cnt, telemvar->type, SCS_TELEMETRY_CHANNEL_FLAG_none, chan_callback, this);
				++reg_chan_cnt;
			}
			while (reg_chan_cnt > *telemvar->dynamic_count && reg_chan_cnt > 0) {
				--reg_chan_cnt;
				unreg_chan(telemvar->name.c_str(), reg_chan_cnt, telemvar->type);
			}
		}
		else {
			if (reg_chan_cnt == 0) {
				if (telemvar->max_count == SCS_U32_NIL) {
					reg_chan(telemvar->name.c_str(), SCS_U32_NIL, telemvar->type, SCS_TELEMETRY_CHANNEL_FLAG_none, chan_callback, this);
				}
				else {
					for (scs_u32_t i = 0; i < telemvar->max_count; ++i) {
						reg_chan(telemvar->name.c_str(), i, telemvar->type, SCS_TELEMETRY_CHANNEL_FLAG_none, chan_callback, this);
					}
				}
				reg_chan_cnt = telemvar->max_count;
			}
		}
	}

	void unreg_callbacks() {
		if (reg_chan_cnt != 0) {
			if (telemvar->max_count == SCS_U32_NIL) {
				unreg_chan(telemvar->name.c_str(), SCS_U32_NIL, telemvar->type);
			}
			else {
				while (reg_chan_cnt > 0) {
					--reg_chan_cnt;
					unreg_chan(telemvar->name.c_str(), reg_chan_cnt, telemvar->type);
				}
			}
		}
	}

	static scs_telemetry_register_for_channel_t reg_chan;
	static scs_telemetry_unregister_from_channel_t unreg_chan;

	const std::unique_ptr<BaseTelemVar> telemvar;

	protected:
	scs_u32_t reg_chan_cnt = 0;
};

scs_telemetry_register_for_channel_t ChannelUpdateHandler::reg_chan = nullptr;
scs_telemetry_unregister_from_channel_t ChannelUpdateHandler::unreg_chan = nullptr;

SCSAPI_VOID chan_callback(const scs_string_t name, 
						  const scs_u32_t index, 
						  const scs_value_t* const value, 
						  const scs_context_t context) {
	ChannelUpdateHandler* const ch = static_cast<ChannelUpdateHandler*>(context);
	assert(value && ch && value->type == ch->telemvar->type);

	#pragma warning(suppress: 6011)
	ch->telemvar->store_value(*value, index);
}