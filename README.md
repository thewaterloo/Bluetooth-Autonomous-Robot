# Bluetooth-Controlled Autonomous Robot

Bare-metal C++ firmware for an ATmega328P-based differential drive robot
with Bluetooth (HC-05) teleop *and* fully autonomous obstacle avoidance.
No Arduino core — direct register access throughout — and a small
Makefile that builds with `avr-gcc` and flashes with `avrdude`.

## What it does

- **Manual mode** — receive single-character commands (`F B L R S`) over
  UART from an HC-05 module and drive the H-bridge accordingly.
- **Autonomous mode** — a PID holds a target front clearance using the
  HC-SR04 ultrasonic sensor, while two side-facing digital IR sensors
  take priority over the PID when objects appear at the robot's sides.
  Sensor polling is synchronous, so response latency depends on echo timeout.
- **Telemetry** — every ten control iterations the firmware streams `mode / distance /
  IR flags / left PWM / right PWM` back over the same UART, so you can
  watch the robot's decisions live in any serial terminal app.
- Mode is toggled at runtime with `A` (auto) and `M` (manual).

## Architecture

```
         ┌───────────────┐
HC-05 ───│  uart.cpp     │── RX ring buffer (ISR)
         └──────┬────────┘
                │ commands / telemetry
         ┌──────▼────────┐
         │  main.cpp     │── sensor/control loop
         └──┬──────┬──┬──┘
            │      │  │
    ┌───────▼┐ ┌───▼─┐│┌──────────┐
    │ultra-  │ │ ir_ │││  pid.cpp │
    │sonic   │ │sens.│││          │
    └────────┘ └─────┘│└──────────┘
                      │
               ┌──────▼──────┐
               │  motor.cpp  │── Timer1 Fast PWM, L298N
               └─────────────┘
```

Every peripheral is a small, testable namespace with its own header.
Nothing global except the state inside `main.cpp`.

## Hardware

| Subsystem      | Part                  | Notes                               |
|----------------|-----------------------|-------------------------------------|
| MCU            | ATmega328P @ 16 MHz   | Uno-compatible clock                |
| Motor driver   | L298N dual H-bridge   | Any 2-IN + 1-EN per side works      |
| Drive motors   | 2× TT gearmotor       | 6 V nominal                         |
| Rangefinder    | HC-SR04               | 5 V, 2–400 cm                       |
| Side sensors   | 2× TCRT5000 IR module | Digital, active-low on obstacle     |
| Bluetooth      | HC-05                 | 9600 baud, 8N1                      |

See [`wiring.md`](wiring.md) for the full pin-by-pin hookup.

## Building

```bash
# Toolchain (once)
sudo apt install gcc-avr avr-libc avrdude      # Ubuntu/Debian
brew tap osx-cross/avr && brew install avr-gcc avrdude   # macOS

# Build
make
# → build/firmware.hex, and a size report

# Flash via the Arduino bootloader
make flash PORT=/dev/ttyUSB0    # or /dev/ttyACM0, COM5, etc.
```

## Controlling it

Pair the HC-05 (default pin `1234`), open any serial terminal at 9600
baud pointed at the Bluetooth serial port, and send single characters:

| Key | Action                  |
|-----|-------------------------|
| `F` | Forward                 |
| `B` | Reverse                 |
| `L` | Spin left               |
| `R` | Spin right              |
| `S` | Stop                    |
| `A` | Switch to autonomous    |
| `M` | Switch back to manual   |

Telemetry prints look like:

```
mode=AUTO d=42cm ir=-- L=180 R=180
mode=AUTO d=18cm ir=-R L=-200 R=200
mode=MAN  d=--- ir=-- L=0   R=0
```

## Tuning

Hardware constants and limits live in `config.h`:

- `OBSTACLE_CM` — trigger distance for the back-and-turn behaviour
- `CRUISE_SPEED`, `TURN_SPEED` — PWM caps
- `CONTROL_PERIOD_MS` — minimum loop delay; synchronous sensor reads and
  UART output extend the actual period

PID gains are set where the controller is instantiated in `main.cpp`
(`PID speed_pid(8.0f, 0.5f, 1.0f, ...)`). Start by zeroing Ki/Kd and
tuning Kp until the robot smoothly approaches the setpoint without
oscillation.

## License

MIT — see `LICENSE`.
