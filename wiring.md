# Wiring

All pin references are the ATmega328P port names. Arduino Uno pin
numbers are listed in parentheses for reference when prototyping on a
Uno-style dev board.

## Motor driver (L298N)

| Signal        | MCU pin     | Uno pin | Notes                          |
|---------------|-------------|---------|--------------------------------|
| Left IN1      | `PD4`       | D4      | Direction A, left motor        |
| Left IN2      | `PD5`       | D5      | Direction B, left motor        |
| Left ENA/PWM  | `PB1 / OC1A`| D9      | 8-bit Fast PWM from Timer1     |
| Right IN1     | `PD6`       | D6      | Direction A, right motor       |
| Right IN2     | `PD7`       | D7      | Direction B, right motor       |
| Right ENB/PWM | `PB2 / OC1B`| D10     | 8-bit Fast PWM from Timer1     |
| VCC logic     | 5 V         |         | From MCU regulator             |
| VS motor      | 6–12 V      |         | Battery pack — **not** USB 5 V |
| GND           | Common      |         | Tie MCU GND and motor GND      |

Remove the L298N 5 V jumper if you're powering the MCU from USB; leave
it in only if the board is standalone on the battery pack.

## HC-SR04 ultrasonic

| HC-SR04 | MCU pin | Uno pin |
|---------|---------|---------|
| VCC     | 5 V     |         |
| TRIG    | `PD2`   | D2      |
| ECHO    | `PD3`   | D3      |
| GND     | GND     |         |

The ECHO pin is 5 V — use a 1 k / 2 k divider to bring it down if you're
running the MCU at 3.3 V.

## IR avoidance sensors

| Module | MCU pin | Uno pin | Notes                 |
|--------|---------|---------|-----------------------|
| Left   | `PC0`   | A0      | Digital OUT, active-low |
| Right  | `PC1`   | A1      | Digital OUT, active-low |

Internal pull-ups are enabled in `ir_sensor::init()` so you don't need
external resistors.

## HC-05 Bluetooth

| HC-05 | MCU pin     | Uno pin | Notes                        |
|-------|-------------|---------|------------------------------|
| VCC   | 5 V         |         |                              |
| GND   | GND         |         |                              |
| RXD   | `PD1 / TXD` | D1      | **Through a voltage divider** (HC-05 RX is 3.3 V) |
| TXD   | `PD0 / RXD` | D0      | Direct                        |

Unplug the HC-05 TX line while flashing the MCU over USB — the
bootloader uses the same UART and a connected module will block the
flash.

## Power

A 2S or 3S Li-ion pack into the L298N `VS` works well, with the MCU
taking 5 V from either USB (bench) or a small buck converter (robot).
Tie all grounds together at one point.
