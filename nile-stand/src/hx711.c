#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_task_wdt.h>
#include <esp_timer.h>

#include "hx711.h"

void hx711_setup_pins(hx711_pins_t pins) {
	ESP_ERROR_CHECK(gpio_set_direction(pins.clock_pin, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(pins.out_pin, GPIO_MODE_INPUT));
}

void hx711_setup_pins_many(gpio_num_t clock, const gpio_num_t* data_pins, uint8_t n) {
	ESP_ERROR_CHECK(gpio_set_direction(clock, GPIO_MODE_OUTPUT));
	
	for (uint8_t i = 0; i < n; i++) {
		ESP_ERROR_CHECK(gpio_set_direction(data_pins[i], GPIO_MODE_INPUT));
	}
}

uint32_t hx711_read(hx711_pins_t pins) {
	uint32_t value = 0;

	gpio_set_level(pins.clock_pin, 0);

	// Wait for out pin to go low.
	while (gpio_get_level(pins.out_pin));

	for (uint8_t i = 0; i < HX711_READ_BITS; i++) {
		gpio_set_level(pins.clock_pin, 1);
		value = value << 1;
		gpio_set_level(pins.clock_pin, 0);
		value |= gpio_get_level(pins.out_pin);
	}

	// Pulse puts out pin back to high, effectively reseting the state.
	gpio_set_level(pins.clock_pin, 1);
	gpio_set_level(pins.clock_pin, 0);

	// Right now the gain for the next read would be 128.
	// 
	// If we wanted a gain of 32 we could pulse the clock once more, and if we
	// wanted a gain of 64 we would pulse the clock once more still.

	return value;
}

void hx711_read_many(gpio_num_t clock, const gpio_num_t* data_pins, uint32_t* values, uint8_t n) {
	gpio_set_level(clock, 0);

	for (int i = 0; i < n; i++) {
		values[i] = 0;
	}

wait:
	for (uint8_t i = 0; i < n; i++) {
		if (gpio_get_level(data_pins[i])) {
			goto wait;
		}
	}

	for (uint8_t i = 0; i < HX711_READ_BITS; i++) {
		gpio_set_level(clock, 1);
		gpio_set_level(clock, 0);
		
		for (uint8_t j = 0; j < n; j++) {
			values[j] = values[j] << 1;
			values[j] |= gpio_get_level(data_pins[j]);
		}
	}
	
	gpio_set_level(clock, 1);
	gpio_set_level(clock, 0);
}
