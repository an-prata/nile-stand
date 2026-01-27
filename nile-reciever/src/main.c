#include <stdio.h>
#include <esp_task_wdt.h>

#include "uart.h"
#include "rs485.h"

#define BUFFER_SIZE (1024 * 8)

static rs485_t rs485;

static char buffer_usb[BUFFER_SIZE] = { 0 };
static char buffer_uart[BUFFER_SIZE] = { 0 };

void app_main() {
    rs485_init(&rs485, UART_NUM_2, GPIO_NUM_4, GPIO_NUM_5, -1, -1 ,-1);

    while (1) {
        size_t bytes_in_usb = fread(buffer_usb, sizeof *buffer_usb, BUFFER_SIZE, stdin);
        size_t bytes_in_uart = rs485_recieve(&rs485, buffer_uart, BUFFER_SIZE);

        rs485_send(&rs485, buffer_usb, bytes_in_usb);
        fwrite(buffer_uart, sizeof *buffer_uart, bytes_in_uart, stdout);
        
        vTaskDelay(5);
    }
}