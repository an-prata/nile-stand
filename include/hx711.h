#ifndef HX711_H
#define HX711_H

#include <stdint.h>

#define PIN uint8_t
#define HX711_READ_BITS 24

/**
 * Type holding pin pair information for an HX711 serial connection.
 */
typedef struct {
	PIN clock_pin;
	PIN out_pin;
} hx711_pins_t;

/**
 * Configure the given pin set for use as an HX711 instance.
 */
void hx711_setup_pins(hx711_pins_t pins);

/**
 * Read out a value from an HX711 connected with the given pin set.
 */
uint32_t hx711_read(hx711_pins_t pins);

#endif  /* HX711 */
