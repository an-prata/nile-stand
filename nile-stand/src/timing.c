#include "timing.h"

#define MICROS_TO_SECONDS 0.000001

static uint64_t timing_prev_time = 0;
static uint64_t timing_delta_time;

void timing_mark_loop(void) {
    uint64_t current_time = esp_timer_get_time();
    timing_delta_time = current_time - timing_prev_time;
    timing_prev_time = current_time;
}

uint64_t timing_delta_time_us(void) {
    return timing_delta_time;
}

double timing_delta_time_s(void) {
    return (double)timing_delta_time + MICROS_TO_SECONDS;
}

double timing_d_dt(double x0, double x1) {
    return (x1 - x0) / timing_delta_time_s();
}