# =============================================================================
# Makefile - avr-gcc build for bt-autonomous-robot
# -----------------------------------------------------------------------------
# Targets:
#   make           build firmware.hex
#   make size      print section sizes
#   make flash     upload via avrdude (arduino bootloader over USB)
#   make clean     rm -rf build/
#
# Override the port on the command line, e.g.:
#   make flash PORT=/dev/ttyACM0
# =============================================================================

MCU      := atmega328p
F_CPU    := 16000000UL
PROG     := firmware
BUILD    := build
SRC_DIR  := src

PORT     ?= /dev/ttyUSB0
BAUD     ?= 115200
PROGRAMMER ?= arduino

CXX      := avr-g++
CC       := avr-gcc
OBJCOPY  := avr-objcopy
SIZE     := avr-size
AVRDUDE  := avrdude

CPPFLAGS := -mmcu=$(MCU) -DF_CPU=$(F_CPU) -I$(SRC_DIR)
CFLAGS   := -Os -Wall -Wextra -ffunction-sections -fdata-sections -std=gnu11
CXXFLAGS := -Os -Wall -Wextra -ffunction-sections -fdata-sections \
            -fno-exceptions -fno-rtti -std=gnu++17
LDFLAGS  := -mmcu=$(MCU) -Wl,--gc-sections

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD)/%.o,$(SRCS))

all: $(BUILD)/$(PROG).hex size

$(BUILD):
	@mkdir -p $@

$(BUILD)/%.o: $(SRC_DIR)/%.cpp | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD)/$(PROG).elf: $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

$(BUILD)/$(PROG).hex: $(BUILD)/$(PROG).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

size: $(BUILD)/$(PROG).elf
	@$(SIZE) --mcu=$(MCU) --format=avr $<

flash: $(BUILD)/$(PROG).hex
	$(AVRDUDE) -c $(PROGRAMMER) -p $(MCU) -P $(PORT) -b $(BAUD) -U flash:w:$<:i

clean:
	rm -rf $(BUILD)

.PHONY: all size flash clean
