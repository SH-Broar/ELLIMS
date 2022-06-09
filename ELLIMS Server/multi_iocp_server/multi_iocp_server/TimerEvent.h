#pragma once
#include "AMBIT.h"

struct TIMER_EVENT
{
public:
	int object_id;
	EVENT_TYPE ev;
	chrono::system_clock::time_point act_time;
	int target_id;

	constexpr bool operator < (const TIMER_EVENT& _Left) const
	{
		return (act_time > _Left.act_time);
	}

};