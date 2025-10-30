#include <driver/gpio.h>

#include "hx711.h"

void hx711_setup_pins(hx711_pins_t pins) {
	ESP_ERROR_CHECK(gpio_set_direction(pins.clock_pin, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(pins.out_pin, GPIO_MODE_INPUT));
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

float hx711_read_grams(hx711_pins_t pins) {
	uint32_t raw_value = hx711_read(pins);

	return 50000.0 * (float)raw_value / (float)HX711_MAX_RAW_VALUE;
}
