// =============================================================================
// ir_sensor.h - digital IR avoidance sensors (active low on obstacle)
// =============================================================================
#pragma once
#include <stdint.h>

namespace ir_sensor {
    void init();
    bool left_blocked();
    bool right_blocked();
}
