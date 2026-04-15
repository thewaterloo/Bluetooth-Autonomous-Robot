// =============================================================================
// ir_sensor.cpp
// =============================================================================
#include "config.h"
#include "ir_sensor.h"
#include <avr/io.h>

namespace ir_sensor {

void init() {
    // Inputs with internal pull-ups (modules are open-collector active-low)
    IR_DDR  &= ~((1 << IR_LEFT_BIT) | (1 << IR_RIGHT_BIT));
    PORTC   |=  (1 << IR_LEFT_BIT) | (1 << IR_RIGHT_BIT);
}

bool left_blocked()  { return !(IR_PIN & (1 << IR_LEFT_BIT));  }
bool right_blocked() { return !(IR_PIN & (1 << IR_RIGHT_BIT)); }

} // namespace ir_sensor
