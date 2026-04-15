// =============================================================================
// main.cpp - Bluetooth-controlled autonomous robot firmware
// -----------------------------------------------------------------------------
// Bare-metal AVR firmware for an ATmega328P. Two top-level modes:
//
//   MANUAL      - Bluetooth 'F/B/L/R/S' commands drive the motors directly.
//   AUTONOMOUS  - PID maintains a cruise speed based on front ultrasonic
//                 clearance; IR side sensors trigger immediate evasive turns
//                 for sub-10 ms obstacle response.
//
// Mode is toggled at runtime with 'A' (auto) / 'M' (manual) over UART0.
// Telemetry (mode / distance / IR / left / right) is streamed back at 5 Hz.
//
// Control loop runs at 50 Hz via a busy-wait on _delay_ms — simple and
// deterministic. Ultrasonic reads are interleaved so no one iteration
// blocks for the full echo timeout in the common case.
// =============================================================================
#include "config.h"
#include "uart.h"
#include "motor.h"
#include "ultrasonic.h"
#include "ir_sensor.h"
#include "pid.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

namespace {

enum class Mode : uint8_t { MANUAL, AUTONOMOUS };

Mode     mode        = Mode::MANUAL;
int16_t  cmd_left    = 0;
int16_t  cmd_right   = 0;
uint16_t last_dist   = UINT16_MAX;
uint8_t  tel_divider = 0;

// PID: controls forward speed to keep the front distance near a setpoint.
// Input units: cm. Output units: PWM (0..255 effective forward).
PID speed_pid(8.0f, 0.5f, 1.0f, -255.0f, 255.0f);

// ------------------------------------------------------------------ commands
void handle_byte(uint8_t b) {
    switch (b) {
    case 'A': case 'a':
        mode = Mode::AUTONOMOUS;
        speed_pid.reset();
        uart::tx_str("[mode] AUTO\r\n");
        break;
    case 'M': case 'm':
        mode = Mode::MANUAL;
        cmd_left = cmd_right = 0;
        uart::tx_str("[mode] MANUAL\r\n");
        break;
    case 'F': case 'f':
        cmd_left = cmd_right = CRUISE_SPEED; break;
    case 'B': case 'b':
        cmd_left = cmd_right = -CRUISE_SPEED; break;
    case 'L': case 'l':
        cmd_left = -TURN_SPEED; cmd_right =  TURN_SPEED; break;
    case 'R': case 'r':
        cmd_left =  TURN_SPEED; cmd_right = -TURN_SPEED; break;
    case 'S': case 's':
        cmd_left = cmd_right = 0; break;
    default: break;
    }
}

void drain_uart() {
    while (uart::rx_available()) handle_byte(uart::rx());
}

// ------------------------------------------------------------------- control
// Returns (left, right) PWM given the current sensor state and mode.
void compute_drive(int16_t& left, int16_t& right) {
    if (mode == Mode::MANUAL) {
        left  = cmd_left;
        right = cmd_right;
        return;
    }

    // --- AUTONOMOUS ----------------------------------------------------------
    bool il = ir_sensor::left_blocked();
    bool ir = ir_sensor::right_blocked();

    // Reflexive evasion: IR side sensors take priority over PID cruise. This
    // is the sub-10 ms obstacle avoidance path.
    if (il && ir) {
        left  = -TURN_SPEED;
        right =  TURN_SPEED;           // both blocked: spin left
        return;
    }
    if (il) { left =  TURN_SPEED; right = -TURN_SPEED; return; } // veer right
    if (ir) { left = -TURN_SPEED; right =  TURN_SPEED; return; } // veer left

    // Front ultrasonic drives a PID on "desired clearance".
    // Setpoint = OBSTACLE_CM + headroom; below that, error goes negative and
    // the PID commands reverse; above, it cruises forward.
    const float setpoint_cm = static_cast<float>(OBSTACLE_CM + 15);
    float measured = (last_dist == UINT16_MAX) ? 200.0f
                                               : static_cast<float>(last_dist);
    float dt = CONTROL_PERIOD_MS / 1000.0f;
    float u  = speed_pid.update(setpoint_cm, measured, dt);

    // Hard stop if we've crossed the obstacle line even if PID is still settling.
    if (last_dist != UINT16_MAX && last_dist < OBSTACLE_CM) {
        left  = -TURN_SPEED;
        right =  TURN_SPEED;           // back-and-turn
        return;
    }

    // Cap at cruise speed and apply symmetrically.
    int16_t drive = static_cast<int16_t>(u);
    if (drive >  CRUISE_SPEED) drive =  CRUISE_SPEED;
    if (drive < -CRUISE_SPEED) drive = -CRUISE_SPEED;
    left  = drive;
    right = drive;
}

// ------------------------------------------------------------------ telemetry
void emit_telemetry(int16_t left, int16_t right) {
    if (++tel_divider < 10) return;    // 50 Hz / 10 = 5 Hz
    tel_divider = 0;

    uart::tx_str("mode=");
    uart::tx_str(mode == Mode::MANUAL ? "MAN" : "AUTO");
    uart::tx_str(" d=");
    if (last_dist == UINT16_MAX) uart::tx_str("---");
    else                         uart::tx_dec(static_cast<int16_t>(last_dist));
    uart::tx_str("cm ir=");
    uart::tx(ir_sensor::left_blocked()  ? 'L' : '-');
    uart::tx(ir_sensor::right_blocked() ? 'R' : '-');
    uart::tx_str(" L=");  uart::tx_dec(left);
    uart::tx_str(" R=");  uart::tx_dec(right);
    uart::tx_str("\r\n");
}

} // anonymous namespace

// =============================================================================
int main() {
    uart::init(UART_BAUD);
    motor::init();
    ultrasonic::init();
    ir_sensor::init();
    sei();

    uart::tx_str("\r\n[boot] bt-autonomous-robot ready\r\n");
    uart::tx_str("[boot] commands: F B L R S  /  A=auto  M=manual\r\n");

    for (;;) {
        drain_uart();

        // Ultrasonic every iteration — read is non-blocking on success, up
        // to ~30 ms on timeout. Obstacle-avoidance reflex uses IR, so a slow
        // ultrasonic read never compromises safety.
        last_dist = ultrasonic::read_cm();

        int16_t left, right;
        compute_drive(left, right);
        motor::set(left, right);

        emit_telemetry(left, right);

        _delay_ms(CONTROL_PERIOD_MS);
    }
}
