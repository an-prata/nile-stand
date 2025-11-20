#ifndef HX711_H
#define HX711_H

#include <stdint.h>
#include <driver/gpio.h>

#define HX711_READ_BITS 24
#define HX711_MAX_RAW_VALUE (uint32_t)0x00FFFFFF

/**
 * Type holding pin pair information for an HX711 serial connection.
 */
typedef struct {
	gpio_num_t clock_pin;
	gpio_num_t out_pin;
} hx711_pins_t;

/**
 * Configure the given pin set for use as an HX711 instance.
 */
void hx711_setup_pins(hx711_pins_t pins);

/**
 * Configure a clock pin and `n` data pins.
 */
void hx711_setup_pins_many(gpio_num_t clock, const gpio_num_t* data_pins, uint8_t n);

/**
 * Read out a value from an HX711 connected with the given pin set.
 */
uint32_t hx711_read(hx711_pins_t pins);

/**
 * Takes a clock pin and an array of `n` data pins and reads out the values from
 * all those data pins into the given `values` array.
 */
void hx711_read_many(gpio_num_t clock, const gpio_num_t* data_pins, uint32_t* values, uint8_t n);

#endif  /* HX711 */
