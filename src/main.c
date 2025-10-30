#include <stdbool.h>
#include <unistd.h>
#include <driver/gpio.h>

#include "field.h"
#include "hx711.h"

#define LOAD_CELL_PIN_CLOCK GPIO_NUM_26
#define LOAD_CELL_PIN_DATA GPIO_NUM_27

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
        load_cell_field.value.field_value.floating = (float)hx711_read(load_cell_pins);
        update_field(load_cell_field);

        usleep(100 * 1000);
    }
}