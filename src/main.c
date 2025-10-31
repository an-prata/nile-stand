#include <stdbool.h>
#include <driver/gpio.h>
#include <esp_task_wdt.h>
#include <esp_timer.h>

#include "field.h"
#include "hx711.h"

#define MICROS_TO_SECONDS 0.000001

#define LOAD_CELL_CLOCK 11
#define LOAD_CELL_DATA_0 12
#define LOAD_CELL_DATA_1 13
#define LOAD_CELL_DATA_2 14
#define LOAD_CELL_DATA_3 15
#define LOAD_CELL_COUNT 4

static const uint32_t load_cell_measurements[LOAD_CELL_COUNT];
static const PIN load_cell_data_pins[LOAD_CELL_COUNT] = {
    LOAD_CELL_DATA_0,
    LOAD_CELL_DATA_1,
    LOAD_CELL_DATA_2,
    LOAD_CELL_DATA_3
};

static uint32_t scale_0_measurement = 0;
static double scale_0_value = 0.0;
static double scale_0_value_rate = 0.0;

static field_t scale_0_field = {
    .name = "Scale 0",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0.0
        }
    }
};

static field_t scale_0_rate_field = {
    .name = "Scale 0 Rate",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0.0
        }
    }
};

static uint32_t scale_1_measurement = 0;
static double scale_1_value = 0.0;
static double scale_1_value_rate = 0.0;

static field_t scale_1_field = {
    .name = "Scale 1",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0.0
        }
    }
};

static field_t scale_1_rate_field = {
    .name = "Scale 1 rate",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0.0
        }
    }
};

uint64_t time_us_prev = 0;
uint64_t time_us_delta = 0;

/**
 * Apply the calibration of scale 0 to the given scale measurement, returning a
 * value in grams.
 */
double apply_scale_0_calibration(uint32_t measurement);

/**
 * Apply the calibration of scale 1 to the given scale measurement, returning a
 * value in grams.
 */
double apply_scale_1_calibration(uint32_t measurement);

void app_main() {
    ESP_ERROR_CHECK(esp_timer_init());

    hx711_setup_pins_many(LOAD_CELL_CLOCK, load_cell_data_pins, LOAD_CELL_COUNT);
    
    while (true) {
        uint64_t current_time = esp_timer_get_time();
        time_us_delta = current_time - time_us_prev;
        time_us_prev = current_time;

        hx711_read_many(LOAD_CELL_CLOCK, load_cell_data_pins, load_cell_measurements, LOAD_CELL_COUNT);

        scale_0_measurement = load_cell_measurements[0] + load_cell_measurements[1];
        double new_scale_0_value = apply_scale_0_calibration(scale_0_measurement);
        scale_0_value_rate = (new_scale_0_value - scale_0_value) / ((double)time_us_delta * MICROS_TO_SECONDS);
        scale_0_value = new_scale_0_value;

        scale_1_measurement = load_cell_measurements[2] + load_cell_measurements[3];
        double new_scale_1_value = apply_scale_1_calibration(scale_1_measurement);
        scale_1_value_rate = (new_scale_1_value - scale_1_value) / ((double)time_us_delta * MICROS_TO_SECONDS);
        scale_1_value = new_scale_1_value;

        scale_0_field.value.field_value.floating = scale_0_value;
        scale_0_rate_field.value.field_value.floating = scale_0_value_rate;

        scale_1_field.value.field_value.floating = scale_1_value;
        scale_1_rate_field.value.field_value.floating = scale_1_value_rate;

        update_field(scale_0_field);
        update_field(scale_0_rate_field);
        
        update_field(scale_1_field);
        update_field(scale_1_rate_field);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

double apply_scale_0_calibration(uint32_t measurement) {
    return (double)measurement;
}

double apply_scale_1_calibration(uint32_t measurement) {
    return (double)measurement;
}