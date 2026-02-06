#include <driver/gpio.h>
#include <esp_task_wdt.h>
#include <esp_timer.h>
#include <esp_event.h>
#include <driver/i2c_slave.h>
#include <unistd.h>
#include <string.h>

#include "uart.h"

#define SOLENOID_COUNT 12

#define PIN_SOLENOID_0     GPIO_NUM_32
#define PIN_SOLENOID_1     GPIO_NUM_33
#define PIN_SOLENOID_2     GPIO_NUM_25
#define PIN_SOLENOID_3     GPIO_NUM_26
#define PIN_SOLENOID_4     GPIO_NUM_19
#define PIN_SOLENOID_5     GPIO_NUM_17
#define PIN_SOLENOID_6     GPIO_NUM_16
#define PIN_SOLENOID_7     GPIO_NUM_23
#define PIN_E_MATCH        GPIO_NUM_12
#define PIN_SIGNAL_LIGHT_1 GPIO_NUM_13
#define PIN_SIGNAL_LIGHT_2 GPIO_NUM_14
#define PIN_SIGNAL_LIGHT_3 GPIO_NUM_15

#define SOLENOID_0     0x0001
#define SOLENOID_1     0x0002
#define SOLENOID_2     0x0004
#define SOLENOID_3     0x0008
#define SOLENOID_4     0x0010
#define SOLENOID_5     0x0020
#define SOLENOID_6     0x0040
#define SOLENOID_7     0x0080
#define SOLENOID_8     0x0100
#define E_MATCH        0x0200
#define SIGNAL_LIGHT_1 0x0400
#define SIGNAL_LIGHT_2 0x0800
#define SIGNAL_LIGHT_3 0x1000

#define OPEN '|'
#define CLOSED '-'

static char solenoid_set_state[SOLENOID_COUNT] = { CLOSED };
static size_t solenoid_idx = 0;
static bool state_initialized = false;

static uart_t uart;

void app_main() {
    gpio_set_direction(PIN_SOLENOID_0, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_3, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_4, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_5, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_6, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_7, GPIO_MODE_OUTPUT);

    gpio_set_direction(PIN_E_MATCH, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SIGNAL_LIGHT_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SIGNAL_LIGHT_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SIGNAL_LIGHT_3, GPIO_MODE_OUTPUT);

    uart_init(&uart, UART_NUM_2, GPIO_NUM_22, GPIO_NUM_21);

    printf("Starting!\n");

    while (!state_initialized) {
        char rx_char;
        size_t recieved = uart_recieve(&uart, &rx_char, 1);

        if (recieved != 1) {
            continue;
        }

        if (rx_char == '\n') {
            break;
        }
    }

    while (1) {
        char rx_char;
        size_t recieved = 0;
        
        while ((recieved = uart_recieve(&uart, &rx_char, 1)) == 1) {
            switch (rx_char) {
                case '\n':
                    solenoid_idx = 0;
                    state_initialized = true;
                    break;
                
                case OPEN:
                    solenoid_set_state[solenoid_idx] = OPEN;
                    solenoid_idx++;
                    break;
                
                case CLOSED:
                    solenoid_set_state[solenoid_idx] = CLOSED;
                    solenoid_idx++;
                    break;
            }

        }

        if (state_initialized) {
            char newline = '\n';
            uart_send(&uart, &newline, 1);
            uart_send(&uart, solenoid_set_state, SOLENOID_COUNT);
            
            gpio_set_level(PIN_SOLENOID_0, solenoid_set_state[0] == OPEN);
            gpio_set_level(PIN_SOLENOID_1, solenoid_set_state[1] == OPEN);
            gpio_set_level(PIN_SOLENOID_2, solenoid_set_state[2] == OPEN);
            gpio_set_level(PIN_SOLENOID_3, solenoid_set_state[3] == OPEN);
            gpio_set_level(PIN_SOLENOID_4, solenoid_set_state[4] == OPEN);
            gpio_set_level(PIN_SOLENOID_5, solenoid_set_state[5] == OPEN);
            gpio_set_level(PIN_SOLENOID_6, solenoid_set_state[6] == OPEN);
            gpio_set_level(PIN_SOLENOID_7, solenoid_set_state[7] == OPEN);
        }

        vTaskDelay(10);
    }
}