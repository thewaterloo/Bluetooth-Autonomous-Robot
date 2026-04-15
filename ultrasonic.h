// =============================================================================
// ultrasonic.h - HC-SR04 distance sensor
// =============================================================================
#pragma once
#include <stdint.h>

namespace ultrasonic {
    void     init();
    // Returns distance in cm, or UINT16_MAX on timeout (no echo).
    uint16_t read_cm();
}
