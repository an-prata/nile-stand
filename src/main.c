#include <stdbool.h>
#include <unistd.h>
#include <driver/gpio.h>

#include "field.h"
#include "hx711.h"

#define LOAD_CELL_PIN_CLOCK 26
#define LOAD_CELL_PIN_DATA 27

static const hx711_pins_t load_cell_pins = {
    .clock_pin = LOAD_CELL_PIN_CLOCK,
    .out_pin = LOAD_CELL_PIN_DATA
};

static field_t load_cell_field = {
    .name = "Load Cell Value",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0.0
        }
    }
};

void app_main() {
    hx711_setup_pins(load_cell_pins);
    
    while (true) {
        hx711_read(load_cell_pins);
        update_field(load_cell_field);

        usleep(100 * 1000);
    }
}