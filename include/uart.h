#ifndef UART_H
#define UART_H

#include <stdlib.h>

#define UART_BUFFER_SIZE (1024 * 2)  /* 2KB */
#define UART_QUEUE_SIZE 12
#define UART_BAUD_RATE 115200

/* Default RX/TX pins for UART 0 */

#define UART_NUM UART_NUM_0
#define UART_PIN_TX 1
#define UART_PIN_RX 3

/**
 * Initialize UART.
 */
void uart_init(void);

/**
 * Send the given NULL terminated string down the UART serial connection.
 */
void uart_send(const char* msg);

/**
 * Read not more that `n` bytes off of serial into the given buffer. Returns the
 * number of bytes read off of UART.
 */
size_t uart_recieve(char* msg, size_t n);

#endif  /* UART_H */