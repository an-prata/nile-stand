#include <driver/gpio.h>
#include <esp_task_wdt.h>
#include <esp_timer.h>
#include <esp_event.h>
#include <driver/i2c_slave.h>
#include <unistd.h>
#include <string.h>

#include "uart.h"

#define SOLENOID_COUNT 16

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

static size_t solenoid_idx = 1;
static char solenoid_set_state[SOLENOID_COUNT + 1] = { '-' };
static uint16_t solenoid_states;
static bool state_initialized = false;
static uart_t uart;

static void pin_set(uint16_t mask, gpio_num_t pin) {
    if (solenoid_states & mask) {
        gpio_set_level(pin, 1);
        printf("Pin %i: HI\n", pin);
    } else {
        gpio_set_level(pin, 0);
        printf("Pin %i: LO\n", pin);
    }
}

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

    uart_init(&uart, UART_NUM_2, GPIO_NUM_4, GPIO_NUM_5);

    printf("Starting!\n");

    while (1) {
        char rx_char;
        size_t recieved = uart_recieve(&uart, &rx_char, 1);
    
        if (recieved == 1) {
            char buf[SOLENOID_COUNT];
            recieved = uart_recieve(&uart, buf, SOLENOID_COUNT);

            if (recieved == SOLENOID_COUNT) {
                memcpy(solenoid_set_state, buf, SOLENOID_COUNT);
                state_initialized = true;
            }
        }

        if (state_initialized) {
            solenoid_set_state[SOLENOID_COUNT] = '\n';
            uart_send(&uart, solenoid_set_state, SOLENOID_COUNT + 1);
        }

        vTaskDelay(10);
    }
}