#include <driver/gpio.h>
#include <esp_task_wdt.h>
#include <esp_timer.h>

#define PULSE_DELIMETER 500

#define PIN_CLOCK GPIO_NUM_16
#define PIN_DATA GPIO_NUM_17

#define PIN_SOLENOID_0     GPIO_NUM_32
#define PIN_SOLENOID_1     GPIO_NUM_33
#define PIN_SOLENOID_2     GPIO_NUM_25
#define PIN_SOLENOID_3     GPIO_NUM_26
#define PIN_SOLENOID_4     GPIO_NUM_19
#define PIN_SOLENOID_5     GPIO_NUM_21
#define PIN_SOLENOID_6     GPIO_NUM_22
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

static uint16_t solenoid_states;

static void pin_set(uint16_t mask, gpio_num_t pin) {
    if (solenoid_states & mask) {
        gpio_set_level(pin, 1);
    } else {
        gpio_set_level(pin, 0);
    }
}

void app_main() {
    gpio_set_direction(PIN_CLOCK, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_DATA, GPIO_MODE_INPUT);

    gpio_set_direction(PIN_SOLENOID_0, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_3, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_4, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_5, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_6, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_7, GPIO_MODE_OUTPUT);


    while (true) {
        solenoid_states = 0;

        uint64_t last_time = esp_timer_get_time();

        while (esp_timer_get_time() - last_time < PULSE_DELIMETER) {
            if (!gpio_get_level(PIN_CLOCK)) {
                last_time = esp_timer_get_time();
            }

            vTaskDelay(5);
        }

        for (int i = 0; i < 16; i++) {
            while (gpio_get_level(PIN_CLOCK)) {}

            solenoid_states = solenoid_states << 1;
            solenoid_states |= gpio_get_level(PIN_DATA);
        }

        pin_set(SOLENOID_0, PIN_SOLENOID_0);
        pin_set(SOLENOID_1, PIN_SOLENOID_1);
        pin_set(SOLENOID_2, PIN_SOLENOID_2);
        pin_set(SOLENOID_3, PIN_SOLENOID_3);
        pin_set(SOLENOID_4, PIN_SOLENOID_4);
        pin_set(SOLENOID_5, PIN_SOLENOID_5);
        pin_set(SOLENOID_6, PIN_SOLENOID_6);
        pin_set(SOLENOID_7, PIN_SOLENOID_7);
        pin_set(E_MATCH, PIN_E_MATCH);
        pin_set(SIGNAL_LIGHT_1, PIN_SIGNAL_LIGHT_1);
        pin_set(SIGNAL_LIGHT_2, PIN_SIGNAL_LIGHT_2);
        pin_set(SIGNAL_LIGHT_3, PIN_SIGNAL_LIGHT_3);

        vTaskDelay(10);
    }
}