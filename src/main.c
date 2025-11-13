#include <stdbool.h>
#include <driver/gpio.h>
#include <esp_task_wdt.h>
#include <esp_timer.h>
#include <unistd.h>

#include "field.h"
#include "pressure_transducer.h"
#include "hx711.h"
#include "i2c.h"
#include "ads111x.h"
#include "solenoid_controller.h"

#define MICROS_TO_SECONDS 0.000001

#define LOAD_CELL_CLOCK GPIO_NUM_17
#define LOAD_CELL_DATA_0 GPIO_NUM_16
#define LOAD_CELL_DATA_1 GPIO_NUM_26
#define LOAD_CELL_DATA_2 GPIO_NUM_27
#define LOAD_CELL_DATA_3 GPIO_NUM_14
#define LOAD_CELL_COUNT 4

static solenoid_controller_pins_t solenoid_pins = {
    .clock = GPIO_NUM_0,
    .data = GPIO_NUM_0,
};

static uint32_t load_cell_measurements[LOAD_CELL_COUNT] = { 0 };
static const gpio_num_t load_cell_data_pins[LOAD_CELL_COUNT] = {
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
    .name = "Scale 1 Rate",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0.0
        }
    }
};

static field_t scale_rates_ratio = {
    .name = "Scale Rates Ratio",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0.0
        }
    }
};

static field_t pressure_transducer_a0_field = {
    .name = "PT0",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0
        }
    }
};

static field_t pressure_transducer_a1_field = {
    .name = "PT1",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0
        }
    }
};

static field_t pressure_transducer_a2_field = {
    .name = "PT2",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0
        }
    }
};

static field_t pressure_transducer_a3_field = {
    .name = "PT3",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0
        }
    }
};

#define RX_BUF_LEN 256

char rx_buf[RX_BUF_LEN] = { '\0' };
size_t rx_idx = 0;

static uint64_t time_us_prev = 0;
static uint64_t time_us_delta = 0;

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

/**
 * Set a valve, open is `true`, closed is `false`.
 */
void set_valve(valve_e valve, bool state);

void app_main() {
    i2c_init();
    ads111x_device_add();
    hx711_setup_pins_many(LOAD_CELL_CLOCK, load_cell_data_pins, LOAD_CELL_COUNT);

    command_reader_t command_reader = make_command_reader(NULL);
    command_t command;
    
    while (true) {
        // Timing/clock

        uint64_t current_time = esp_timer_get_time();
        time_us_delta = current_time - time_us_prev;
        time_us_prev = current_time;

        // PTs

        pressure_transducer_a0_field.value.field_value.floating = pt_psi_from_volts(ads111x_read_voltage(ADS111X_CHANNEL_A0));
        pressure_transducer_a1_field.value.field_value.floating = pt_psi_from_volts(ads111x_read_voltage(ADS111X_CHANNEL_A1));
        pressure_transducer_a2_field.value.field_value.floating = pt_psi_from_volts(ads111x_read_voltage(ADS111X_CHANNEL_A2));
        pressure_transducer_a3_field.value.field_value.floating = pt_psi_from_volts(ads111x_read_voltage(ADS111X_CHANNEL_A3));

        // Load cells

        /*
        hx711_read_many(LOAD_CELL_CLOCK, load_cell_data_pins, load_cell_measurements, LOAD_CELL_COUNT);

        scale_0_measurement = load_cell_measurements[0] + load_cell_measurements[1];
        double new_scale_0_value = apply_scale_0_calibration(scale_0_measurement);
        scale_0_value_rate = (new_scale_0_value - scale_0_value) / ((double)time_us_delta * MICROS_TO_SECONDS);
        scale_0_value = new_scale_0_value;

        scale_1_measurement = load_cell_measurements[2] + load_cell_measurements[3];
        double new_scale_1_value = apply_scale_1_calibration(scale_1_measurement);
        scale_1_value_rate = (new_scale_1_value - scale_1_value) / ((double)time_us_delta * MICROS_TO_SECONDS);
        scale_1_value = new_scale_1_value;

        scale_0_field.value.field_value.floating = new_scale_0_value;
        scale_0_rate_field.value.field_value.floating = scale_0_value_rate;

        scale_1_field.value.field_value.floating = scale_1_value;
        scale_1_rate_field.value.field_value.floating = scale_1_value_rate;

        scale_rates_ratio.value.field_value.floating = scale_0_value_rate / scale_1_value_rate;
        */

        // Field updates

        update_field(scale_0_field);
        update_field(scale_0_rate_field);
        update_field(scale_1_field);
        update_field(scale_1_rate_field);
        update_field(scale_rates_ratio);

        update_field(pressure_transducer_a0_field);
        update_field(pressure_transducer_a1_field);
        update_field(pressure_transducer_a2_field);
        update_field(pressure_transducer_a3_field);

        // TODO: Give back info about valves!!

        // Handle commands

        rx_idx += fread(rx_buf, sizeof(char), RX_BUF_LEN, stdin);
        command_reader_buffer(&command_reader, rx_buf);

        while (command_reader_read(&command_reader, &command) == 0) {
            switch (command.cmd_type) {
                case COMMAND_OPEN:
                    set_valve(command.cmd_valve, true);
                    break;
                case COMMAND_CLOSE:
                    set_valve(command.cmd_valve, false);
                    break;
                case COMMAND_IGNITE:
                    // TODO: Handle ignition!!!!
                    break;
            }
        }

        memset(rx_buf, '\0', RX_BUF_LEN);
        rx_idx = 0;

        // Update the solenoid controller

        solenoid_controller_push(solenoid_pins);

        // Delay so the watchdog doesn't bite
        vTaskDelay(10);
    }
}

double apply_scale_0_calibration(uint32_t measurement) {
    return ((double)measurement - 1053966.648) / 90.53951915;
}

double apply_scale_1_calibration(uint32_t measurement) {
    return (double)measurement;
}

void set_valve(valve_e valve, bool state) {
    // Handle the weird double action valve.
    if (valve == NP4 && state) {
        solenoid_controller_open(SOLENOID_6);
        solenoid_controller_close(SOLENOID_7);
        return;
    } else if (valve == NP4 && !state) {
        solenoid_controller_close(SOLENOID_6);
        solenoid_controller_open(SOLENOID_7);
        return;
    }

    solenoid_controller_state_t solenoid;

    switch (valve) {
        case NP1: solenoid = SOLENOID_0; break;
        case NP2: solenoid = SOLENOID_1; break;
        case NP3: solenoid = SOLENOID_2; break;
        case IP1: solenoid = SOLENOID_3; break;
        case IP2: solenoid = SOLENOID_4; break;
        case IP3: solenoid = SOLENOID_5; break;
        default: return;
    }

    if (state) {
        solenoid_controller_open(solenoid);
    } else {
        solenoid_controller_close(solenoid);
    }
}