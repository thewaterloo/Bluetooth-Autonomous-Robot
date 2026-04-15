// =============================================================================
// uart.cpp - ring-buffered RX interrupt, polled TX
// =============================================================================
#include "config.h"
#include "uart.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

namespace {
    constexpr uint8_t RX_BUF = 32;
    volatile uint8_t  rx_buf[RX_BUF];
    volatile uint8_t  rx_head = 0;
    volatile uint8_t  rx_tail = 0;
}

ISR(USART_RX_vect) {
    uint8_t b = UDR0;
    uint8_t next = (rx_head + 1) % RX_BUF;
    if (next != rx_tail) {
        rx_buf[rx_head] = b;
        rx_head = next;
    } // else drop on overflow
}

namespace uart {

void init(uint32_t baud) {
    uint16_t ubrr = (F_CPU / 16 / baud) - 1;
    UBRR0H = (ubrr >> 8) & 0xFF;
    UBRR0L =  ubrr       & 0xFF;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8N1
}

void tx(uint8_t b) {
    while (!(UCSR0A & (1 << UDRE0))) { }
    UDR0 = b;
}

void tx_str(const char* s) {
    while (*s) tx(static_cast<uint8_t>(*s++));
}

void tx_dec(int16_t v) {
    char buf[8];
    int i = 0;
    bool neg = v < 0;
    if (neg) v = -v;
    if (v == 0) { tx('0'); return; }
    while (v > 0 && i < 7) { buf[i++] = '0' + (v % 10); v /= 10; }
    if (neg) tx('-');
    while (i > 0) tx(static_cast<uint8_t>(buf[--i]));
}

bool rx_available() {
    uint8_t h, t;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { h = rx_head; t = rx_tail; }
    return h != t;
}

uint8_t rx() {
    while (!rx_available()) { }
    uint8_t b;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        b = rx_buf[rx_tail];
        rx_tail = (rx_tail + 1) % RX_BUF;
    }
    return b;
}

} // namespace uart
