#include <esp_timer.h>
#include "timing.h"

#define MICROS_TO_SECONDS 0.000001

static uint64_t timing_prev_time = 0;
static uint64_t timing_delta_time;

void timing_mark_loop(void) {
    uint64_t current_time = esp_timer_get_time();
    timing_delta_time = current_time - timing_prev_time;
    timing_prev_time = current_time;
}

timing_marker_t timing_mark(void) {
    return esp_timer_get_time();
}

uint64_t timing_time_since_us(timing_marker_t marker) {
    return esp_timer_get_time() - marker;
}

float timing_time_since_s(timing_marker_t marker) {
    return timing_time_since_us(marker) * MICROS_TO_SECONDS;
}

uint64_t timing_delta_time_us(void) {
    return timing_delta_time;
}

float timing_delta_time_s(void) {
    return (float)timing_delta_time * MICROS_TO_SECONDS;
}

float timing_d_dt(float x0, float x1) {
    return (x1 - x0) / timing_delta_time_s();
}