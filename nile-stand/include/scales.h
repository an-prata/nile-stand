#ifndef SCALES_H
#define SCALES_H

#include <stddef.h>

/**
 * Initialize the backing load cells of all scales.
 */
void scales_init(void);

/**
 * Update the internal record of scale readings.
 */
void scales_update(void);

/**
 * Gets the given scale's measurement in grams.
 */
float scales_get(size_t scale_num);

float scales_get_ox(void);
float scales_get_fuel(void);
float scales_get_thrust(void);

#endif  /* SCALES_H */