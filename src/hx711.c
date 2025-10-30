#include "hx711.h"

/**
 * Replace with a function which takes the type `PIN` from the header and
 * returns its value as some integer type with a value of 1 or 0.
 */
#define PIN_READ(pin) 1

/**
 * Replave with a function which writes the given 0/1 integer value to the given
 * `PIN` pin.
 */
#define PIN_WRITE(pin, i)

uint32_t hx711_read(hx711_pins_t pins) {
	uint32_t value = 0;

	PIN_WRITE(pin.clock, 0);

	// Wait for out pin to go low.
	while (PIN_READ(pins.out));

	for (uint8_t i = 0; i < HX711_READ_BITS; i++) {
		PIN_WRITE(pin.clock, 1);
		value << 1;
		PIN_WRITE(pin.clock, 0);
		value |= PIN_READ(pin.out);
	}

	// Pulse puts out pin back to high, effectively reseting the state.
	PIN_WRITE(pin.clock, 1);
	PIN_WRITE(pin.clock, 0);

	// Right now the gain for the next read would be 128.
	// 
	// If we wanted a gain of 32 we could pulse the clock once more, and if we
	// wanted a gain of 64 we would pulse the clock once more still.

	return value;
}
