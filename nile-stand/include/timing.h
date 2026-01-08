#ifndef TIMING_H
#define TIMING_H

#include <stdint.h>

/**
 * Type for holding delay/duration starting times.
 */
typedef uint64_t timing_marker_t;

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
 * Get a marker for the current time.
 */
timing_marker_t timing_mark(void);

/**
 * Time since the given marking in microseconds.
 */
uint64_t timing_time_since_us(timing_marker_t marker);

/**
 * Time since the given marking in seconds.
 */
double timing_time_since_s(timing_marker_t marker);

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