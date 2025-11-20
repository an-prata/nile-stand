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
double scales_get(size_t scale_num);

double scales_get_ox(void);
double scales_get_fuel(void);
double scales_get_thrust(void);

#endif  /* SCALES_H */