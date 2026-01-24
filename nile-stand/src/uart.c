#include <string.h>
#include <driver/uart.h>
#include <esp_log.h>

#include "uart.h"

#define TAG "UART_C"

#define MIN(lhs, rhs) (lhs < rhs ? lhs : rhs)

static QueueHandle_t uart_queue = NULL;

void uart_init(void) {
    ESP_ERROR_CHECK(
        uart_driver_install(
            UART_NUM, 
            UART_BUFFER_SIZE,
            UART_BUFFER_SIZE,
            UART_QUEUE_SIZE,
            &uart_queue,
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

    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &config));
    ESP_ERROR_CHECK(uart_set_mode(UART_NUM, UART_MODE_RS485_HALF_DUPLEX));
    ESP_ERROR_CHECK(
        uart_set_pin(
            UART_NUM,
            UART_PIN_TX,
            UART_PIN_RX,
            UART_PIN_NO_CHANGE,
            UART_PIN_NO_CHANGE
        )
    );

    ESP_ERROR_CHECK(uart_enable_rx_intr(UART_NUM));
    ESP_ERROR_CHECK(uart_enable_tx_intr(UART_NUM, 1, 100 / portTICK_PERIOD_MS));
}

void uart_send(const char* msg) {
    if (uart_write_bytes(UART_NUM, msg, strlen(msg)) < 0) {
        ESP_LOGE(TAG, "Bad arguments to `uart_write_bytes`!");
        abort();
    }

    ESP_ERROR_CHECK(uart_wait_tx_done(UART_NUM, 100 / portTICK_PERIOD_MS));
}

size_t uart_recieve(char* msg, size_t n) {
    size_t len;
    
    ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM, &len));

    if (len == 0) {
        return 0;
    }

    int bytes_read = uart_read_bytes(UART_NUM, msg, MIN(n, len), 100 / portTICK_PERIOD_MS);

    if (bytes_read < 0) {
        ESP_LOGE(TAG, "Could not read off of UART!");
        abort();
    }

    return (size_t)bytes_read;
}