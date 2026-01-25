#include <string.h>
#include <driver/uart.h>
#include <esp_log.h>

#include "uart.h"

#define TAG "UART_C"
#define TIMEOUT (100 / portTICK_PERIOD_MS)

#define MIN(lhs, rhs) (lhs < rhs ? lhs : rhs)

/**
 * Initialize UART with a default configuation and given mode.
 */
static void _uart_init(uart_t* uart, uart_port_t port, int pin_tx, int pin_rx, uart_mode_t mode) {
    uart->port = port;
    uart->pin_tx = pin_tx;
    uart->pin_rx = pin_rx;

    ESP_ERROR_CHECK(
        uart_driver_install(
            uart->port, 
            UART_BUFFER_SIZE,
            UART_BUFFER_SIZE,
            UART_QUEUE_SIZE,
            &uart->queue,
            0
        )
    );

    uart_config_t config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_ODD,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    ESP_ERROR_CHECK(uart_param_config(uart->port, &config));
    ESP_ERROR_CHECK(uart_set_mode(uart->port, mode));
    ESP_ERROR_CHECK(
        uart_set_pin(
            uart->port,
            uart->pin_tx,
            uart->pin_rx,
            UART_PIN_NO_CHANGE,
            UART_PIN_NO_CHANGE
        )
    );

    ESP_ERROR_CHECK(uart_enable_rx_intr(UART_NUM));
    ESP_ERROR_CHECK(uart_enable_tx_intr(UART_NUM, 1, TIMEOUT));
}

void uart_init(uart_t* uart, uart_port_t port, int pin_tx, int pin_rx) {
    _uart_init(uart, port, pin_tx, pin_rx, UART_MODE_UART);
}

void uart_rs485_init(uart_t* uart, uart_port_t port, int pin_tx, int pin_rx) {
    _uart_init(uart, port, pin_tx, pin_rx, UART_MODE_RS485_HALF_DUPLEX);
}

void uart_send(uart_t* uart, const char* msg, size_t n) {
    if (uart_write_bytes(uart->port, msg, n) < 0) {
        ESP_LOGE(TAG, "Bad arguments to `uart_write_bytes`!");
        abort();
    }

    ESP_ERROR_CHECK(uart_wait_tx_done(uart->port, 100 / portTICK_PERIOD_MS));
}

size_t uart_recieve(uart_t* uart, char* msg, size_t n) {
    size_t len;
    
    ESP_ERROR_CHECK(uart_get_buffered_data_len(uart->port, &len));

    if (len == 0) {
        return 0;
    }

    int bytes_read = uart_read_bytes(uart->port, msg, MIN(n, len), TIMEOUT);

    if (bytes_read < 0) {
        ESP_LOGE(TAG, "Could not read off of UART port %d!", uart->port);
        abort();
    }

    return (size_t)bytes_read;
}