#include <stdio.h>
#include <esp_task_wdt.h>

#include "uart.h"

#define BUFFER_SIZE (1024 * 8)

static char buffer_usb[BUFFER_SIZE] = { 0 };
static char buffer_uart[BUFFER_SIZE] = { 0 };

void app_main() {
    uart_init();

    while (1) {
        size_t bytes_in_usb = fread(buffer_usb, sizeof *buffer_usb, BUFFER_SIZE, stdin);
        size_t bytes_in_uart = uart_recieve(buffer_uart, BUFFER_SIZE);

        uart_send(buffer_usb, bytes_in_usb);
        fwrite(buffer_uart, sizeof *buffer_uart, bytes_in_uart, stdout);
        vTaskDelay(5);
    }
}