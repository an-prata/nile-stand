#ifndef TIMING_H
#define TIMING_H

#include <stdint.h>

/**
 * This function should be called once per main loop, and only once, in a
 * constistant location relative to other code in the loop. This function
 * records the time the loop has taken and that measurement is used in other
 * functions from this header.
 * 
 * This function must also have been called at least once before any other
 * functons in this header.
 */
void timing_mark_loop(void);

/**
 * Gets the main loop's delta time in microseconds.
 */
uint64_t timing_delta_time_us(void);

/**
 * Gets the main loop's delta time in seconds.
 */
double timing_delta_time_s(void);

/**
 * Returns the rate of change over time (in seconds) of the given initial and
 * final values.
 */
double timing_d_dt(double x0, double x1);

#endif