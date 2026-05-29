#pragma once

#include <cstdint>

namespace ProjectConfig
{
    static constexpr uint32_t COMMAND_CAN_ID = 0x101;
    static constexpr uint32_t STATUS_CAN_ID  = 0x201;

    static constexpr uint32_t LOOP_TIMEOUT_MS = 100U;
    static constexpr uint32_t LAMP_TIMEOUT_MS = 3000U;

    static constexpr const char* STATE_FILE =
        "runtime/smartdoor_state.txt";

    static constexpr const char* EVENT_FILE =
        "runtime/smartdoor_events.log";
}