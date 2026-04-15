// =============================================================================
// config.h - pin map, clock, and tuning constants
// -----------------------------------------------------------------------------
// Target: ATmega328P @ 16 MHz (Arduino Uno-compatible clock).
// H-bridge: L298N-style, driven by 2 direction + 1 PWM line per side.
// Ultrasonic: HC-SR04 on PD2 (trig) / PD3 (echo, INT1 input capture).
// IR sensors: digital avoidance sensors on PC0 (left), PC1 (right).
// Bluetooth: HC-05 on UART0 @ 9600 8N1.
// =============================================================================
#pragma once

#define F_CPU 16000000UL

// --- Motor driver (L298N) ----------------------------------------------------
// Left side
#define LEFT_IN1_DDR   DDRD
#define LEFT_IN1_PORT  PORTD
#define LEFT_IN1_BIT   PD4
#define LEFT_IN2_DDR   DDRD
#define LEFT_IN2_PORT  PORTD
#define LEFT_IN2_BIT   PD5
// Left PWM = OC1A = PB1
#define LEFT_PWM_DDR   DDRB
#define LEFT_PWM_BIT   PB1

// Right side
#define RIGHT_IN1_DDR  DDRD
#define RIGHT_IN1_PORT PORTD
#define RIGHT_IN1_BIT  PD6
#define RIGHT_IN2_DDR  DDRD
#define RIGHT_IN2_PORT PORTD
#define RIGHT_IN2_BIT  PD7
// Right PWM = OC1B = PB2
#define RIGHT_PWM_DDR  DDRB
#define RIGHT_PWM_BIT  PB2

// --- HC-SR04 -----------------------------------------------------------------
#define US_TRIG_DDR    DDRD
#define US_TRIG_PORT   PORTD
#define US_TRIG_BIT    PD2
#define US_ECHO_DDR    DDRD
#define US_ECHO_PIN    PIND
#define US_ECHO_BIT    PD3

// --- IR avoidance sensors (digital, active-low on object) --------------------
#define IR_DDR         DDRC
#define IR_PIN         PINC
#define IR_LEFT_BIT    PC0
#define IR_RIGHT_BIT   PC1

// --- UART --------------------------------------------------------------------
#define UART_BAUD      9600UL

// --- Control tuning ----------------------------------------------------------
#define OBSTACLE_CM        20     // stop & turn distance
#define CRUISE_SPEED       180    // 0..255 PWM
#define TURN_SPEED         200
#define CONTROL_PERIOD_MS  20     // 50 Hz control loop
