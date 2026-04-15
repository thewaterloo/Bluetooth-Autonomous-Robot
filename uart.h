// =============================================================================
// uart.h - minimal interrupt-driven UART for HC-05 Bluetooth
// =============================================================================
#pragma once
#include <stdint.h>

namespace uart {
    void   init(uint32_t baud);
    void   tx(uint8_t b);
    void   tx_str(const char* s);
    void   tx_dec(int16_t v);
    bool   rx_available();
    uint8_t rx();               // blocking if nothing pending — check first
}
