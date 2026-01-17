#include <string.h>
#include <driver/uart.h>
#include <esp_log.h>

#include "uart.h"

#define TAG "UART_C"

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
        .parity = UART_PARITY_DISABLE,
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
}

void uart_send(const char* msg) {
    if (uart_write_bytes(UART_NUM, msg, strlen(msg)) < 0) {
        ESP_LOGE(TAG, "Bad arguments to `uart_write_bytes`!");
        abort();
    }
}

size_t uart_recieve(char* msg, size_t n) {
    size_t len = 0;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM, &len));
    printf("Len: %d\n", len);

    int bytes_read = uart_read_bytes(UART_NUM, msg, len, 100 / portTICK_PERIOD_MS);

    printf("Read: %d\n", bytes_read);

    if (bytes_read < 0) {
        ESP_LOGE(TAG, "Could not read off of UART!");
        abort();
    }

    return (size_t)bytes_read;
}