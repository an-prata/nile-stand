#ifndef SCALES_H
#define SCALES_H

/**
 * Initialize the backing load cells of all scales.
 */
void scales_init(void);

/**
 * Retrieve the OX scale in kilograms.
 */
float scales_get_ox(void);

/**
 * Retrieve the Fuel scale in kilograms.
 */
float scales_get_fuel(void);

/**
 * Retrieve the Fuel scale in kilograms.
 */
float scales_get_thrust(void);

#endif  /* SCALES_H */