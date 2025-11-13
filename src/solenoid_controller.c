#include <esp_timer.h>
#include <esp_task_wdt.h>

#include "solenoid_controller.h"

#define PULSE_DELIMETER 500

static solenoid_controller_state_t controller_state = 0;
static uint64_t controller_set_time = 0;

void solenoid_controller_setup(solenoid_controller_pins_t pins) {
    ESP_ERROR_CHECK(gpio_set_direction(pins.clock, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(pins.data, GPIO_MODE_OUTPUT));

    ESP_ERROR_CHECK(gpio_set_level(pins.clock, 1));
}

void solenoid_controller_open(solenoid_controller_state_t state) {
    controller_state |= state;
}

void solenoid_controller_close(solenoid_controller_state_t state) {
    controller_state &= ~state;
}

void solenoid_controller_push(solenoid_controller_pins_t pins) {
    solenoid_contrller_set(pins, controller_state);
}

void solenoid_contrller_set(solenoid_controller_pins_t pins, solenoid_controller_state_t state) {
    while (esp_timer_get_time() - controller_set_time < PULSE_DELIMETER) {
        vTaskDelay(5);
    }
    
    for (int i = 0; i < 16; i++) {
        ESP_ERROR_CHECK(gpio_set_level(pins.data, state % 2));
        ESP_ERROR_CHECK(gpio_set_level(pins.clock, 0));
        vTaskDelay(5);
        ESP_ERROR_CHECK(gpio_set_level(pins.clock, 1));
        state = state >> 1;
    }
    
    ESP_ERROR_CHECK(gpio_set_level(pins.data, 0));
    controller_state = state;
}

solenoid_controller_state_t solenoid_controller_get(void) {
    return controller_state;
}