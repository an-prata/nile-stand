#ifndef RS485_H
#define RS485_H

#include "uart.h"

typedef struct {
    uart_t read;
    uart_t write;
} rs485_t;

/**
 * Initialize a dual RS485 connection. This function takes four pin numbers and
 * two UART ports, two pins and one port is for writing, and the other for
 * reading, in order to prevent collision.
 */
void rs485_init(
    rs485_t* rs485,
    uart_port_t read_port,
    int read_tx,
    int read_rx,
    uart_port_t write_port,
    int write_tx,
    int write_rx
);

/**
 * Send the given buffer of length `n`.
 */
void rs485_send(rs485_t* rs485, const char* msg, size_t n);

/**
 * Read not more that `n` bytes off of serial into the given buffer. Returns the
 * number of bytes read.
 */
size_t rs485_recieve(rs485_t* rs485, char* msg, size_t n);

#endif  /* RS485_H */