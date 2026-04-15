// =============================================================================
// ultrasonic.cpp - HC-SR04 driver
// -----------------------------------------------------------------------------
// Fires a 10 µs trigger pulse, then times the echo pulse width using Timer2
// in normal mode with a /8 prescaler (0.5 µs/tick at 16 MHz). Overflows are
// accumulated so the measurement window can exceed 256 ticks.
//
// Distance formula: speed of sound ≈ 343 m/s = 0.0343 cm/µs
//                   round-trip: cm = µs * 0.0343 / 2 = µs / 58.31
// =============================================================================
#include "config.h"
#include "ultrasonic.h"

#include <avr/io.h>
#include <util/delay.h>

namespace {
    // ~30 ms timeout corresponds to ~5 m round-trip — well past HC-SR04 range.
    constexpr uint16_t ECHO_TIMEOUT_US = 30000;
}

namespace ultrasonic {

void init() {
    US_TRIG_DDR  |=  (1 << US_TRIG_BIT);
    US_TRIG_PORT &= ~(1 << US_TRIG_BIT);
    US_ECHO_DDR  &= ~(1 << US_ECHO_BIT);   // echo is input

    // Timer2 normal mode, prescaler /8 -> 0.5 µs/tick
    TCCR2A = 0;
    TCCR2B = (1 << CS21);
    TCNT2  = 0;
}

uint16_t read_cm() {
    // 10 µs trigger pulse
    US_TRIG_PORT &= ~(1 << US_TRIG_BIT);
    _delay_us(2);
    US_TRIG_PORT |=  (1 << US_TRIG_BIT);
    _delay_us(10);
    US_TRIG_PORT &= ~(1 << US_TRIG_BIT);

    // Wait for echo to go high (with timeout)
    uint16_t wait_us = 0;
    while (!(US_ECHO_PIN & (1 << US_ECHO_BIT))) {
        _delay_us(1);
        if (++wait_us > ECHO_TIMEOUT_US) return UINT16_MAX;
    }

    // Echo high — start the timer
    TIFR2 |= (1 << TOV2);    // clear overflow flag
    TCNT2  = 0;
    uint32_t ticks = 0;

    while (US_ECHO_PIN & (1 << US_ECHO_BIT)) {
        if (TIFR2 & (1 << TOV2)) {
            TIFR2 |= (1 << TOV2);
            ticks += 256;
            if (ticks > (uint32_t)ECHO_TIMEOUT_US * 2) return UINT16_MAX;
        }
    }
    ticks += TCNT2;

    // ticks are 0.5 µs each → µs = ticks / 2
    uint32_t us = ticks >> 1;
    uint32_t cm = us / 58;
    if (cm > 400) return UINT16_MAX;   // HC-SR04 spec max
    return static_cast<uint16_t>(cm);
}

} // namespace ultrasonic
