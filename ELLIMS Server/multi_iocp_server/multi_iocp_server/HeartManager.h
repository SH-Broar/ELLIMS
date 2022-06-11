#pragma once
#include "AMBIT.h"

struct TIMER_EVENT {
    int object_id;
    TIMER_EVENT_TYPE ev;
    chrono::system_clock::time_point act_time;
    int target_id;

    constexpr bool operator < (const TIMER_EVENT& _Left) const
    {
        return (act_time > _Left.act_time);
    }

};

class HeartManager
{
    static priority_queue<TIMER_EVENT> timer_queue;
    static mutex timer_l;

    static array<chrono::system_clock::time_point,NUM_NPC> next_move_times;

public:
    static void initialize_npc();
    static void add_timer(int obj_id, int act_time, TIMER_EVENT_TYPE e_type, int target_id);
    static void move_npc(int npc_id);
    static void ai_thread();
};

