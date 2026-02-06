#ifndef UART_H
#define UART_H

#include <stdlib.h>
#include <driver/uart.h>
#include <hal/uart_types.h>
#include <driver/gpio.h>


#define UART_BUFFER_SIZE (1024 * 8)  /* 8KB */
#define UART_QUEUE_SIZE 32
#define UART_BAUD_RATE 460800

/* Default RX/TX pins for UART 0 */

#define UART_NUM UART_NUM_2
#define UART_PIN_TX 4
#define UART_PIN_RX 5

typedef struct {
    QueueHandle_t queue;
    uart_port_t port;
    int pin_tx;
    int pin_rx;
} uart_t;

typedef void (*uart_print_function_t)(const char*);

/**
 * Initialize UART with a default full duplex configuration.
 */
void uart_init(uart_t* uart, uart_port_t port, gpio_num_t pin_tx, gpio_num_t pin_rx);

/**
 * Initialize UART with a default half duplex configuration intended for use as
 * RS485.
 */
void uart_rs485_init(uart_t* uart, uart_port_t port, gpio_num_t pin_tx, gpio_num_t pin_rx);

/**
 * Send the given buffer of length `n`.
 */
void uart_send(uart_t* uart, const char* msg, size_t n);

/**
 * Read not more that `n` bytes off of serial into the given buffer. Returns the
 * number of bytes read off of UART.
 */
size_t uart_recieve(uart_t* uart, char* msg, size_t n);

#endif  /* UART_H */