// =============================================================================
// motor.cpp - Timer1 8-bit Fast PWM on OC1A (left) and OC1B (right)
// -----------------------------------------------------------------------------
// WGM13:0 = 0101 (Fast PWM, 8-bit, TOP=0xFF)
// COMnx1:0 = 10   (clear OCnx on compare match, set at BOTTOM)
// CS12:0   = 010  (prescaler /8 -> ~7.8 kHz PWM @ 16 MHz — inaudible)
// =============================================================================
#include "config.h"
#include "motor.h"
#include <avr/io.h>
#include <stdlib.h>

namespace {
    inline void set_dir_left(bool forward) {
        if (forward) {
            LEFT_IN1_PORT |=  (1 << LEFT_IN1_BIT);
            LEFT_IN2_PORT &= ~(1 << LEFT_IN2_BIT);
        } else {
            LEFT_IN1_PORT &= ~(1 << LEFT_IN1_BIT);
            LEFT_IN2_PORT |=  (1 << LEFT_IN2_BIT);
        }
    }
    inline void set_dir_right(bool forward) {
        if (forward) {
            RIGHT_IN1_PORT |=  (1 << RIGHT_IN1_BIT);
            RIGHT_IN2_PORT &= ~(1 << RIGHT_IN2_BIT);
        } else {
            RIGHT_IN1_PORT &= ~(1 << RIGHT_IN1_BIT);
            RIGHT_IN2_PORT |=  (1 << RIGHT_IN2_BIT);
        }
    }
}

namespace motor {

void init() {
    // Direction pins as outputs
    LEFT_IN1_DDR  |= (1 << LEFT_IN1_BIT);
    LEFT_IN2_DDR  |= (1 << LEFT_IN2_BIT);
    RIGHT_IN1_DDR |= (1 << RIGHT_IN1_BIT);
    RIGHT_IN2_DDR |= (1 << RIGHT_IN2_BIT);
    // PWM pins as outputs
    LEFT_PWM_DDR  |= (1 << LEFT_PWM_BIT);
    RIGHT_PWM_DDR |= (1 << RIGHT_PWM_BIT);

    // Timer1: Fast PWM 8-bit, non-inverting on OC1A+OC1B, /8 prescaler
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
    TCCR1B = (1 << WGM12)  | (1 << CS11);
    OCR1A  = 0;
    OCR1B  = 0;
    stop();
}

void set(int16_t left, int16_t right) {
    if (left  >  255) left  =  255;
    if (left  < -255) left  = -255;
    if (right >  255) right =  255;
    if (right < -255) right = -255;

    set_dir_left (left  >= 0);
    set_dir_right(right >= 0);
    OCR1A = static_cast<uint8_t>(abs(left));
    OCR1B = static_cast<uint8_t>(abs(right));
}

void stop() {
    OCR1A = 0;
    OCR1B = 0;
    LEFT_IN1_PORT  &= ~(1 << LEFT_IN1_BIT);
    LEFT_IN2_PORT  &= ~(1 << LEFT_IN2_BIT);
    RIGHT_IN1_PORT &= ~(1 << RIGHT_IN1_BIT);
    RIGHT_IN2_PORT &= ~(1 << RIGHT_IN2_BIT);
}

} // namespace motor
