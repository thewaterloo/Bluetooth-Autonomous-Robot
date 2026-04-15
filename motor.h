// =============================================================================
// motor.h - differential drive motor control via L298N + Timer1 PWM
// =============================================================================
#pragma once
#include <stdint.h>

namespace motor {
    void init();
    // Signed drive: [-255..+255]. Negative = reverse.
    void set(int16_t left, int16_t right);
    void stop();
}
