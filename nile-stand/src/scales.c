#include <driver/gpio.h>

#include "hx711.h"
#include "scales.h"

#define LC_0_DATA GPIO_NUM_23
#define LC_0_CLOCK GPIO_NUM_25
#define LC_1_DATA GPIO_NUM_26
#define LC_1_CLOCK GPIO_NUM_27
#define LC_2_DATA GPIO_NUM_32
#define LC_2_CLOCK GPIO_NUM_33

static hx711_pins_t lc_0_pins = {
    .clock_pin = LC_0_CLOCK,
    .out_pin = LC_0_DATA
};

static hx711_pins_t lc_1_pins = {
    .clock_pin = LC_1_CLOCK,
    .out_pin = LC_1_DATA
};

static hx711_pins_t lc_2_pins = {
    .clock_pin = LC_2_CLOCK,
    .out_pin = LC_2_DATA
};
void scales_init(void) {
    hx711_setup_pins(lc_0_pins);
    hx711_setup_pins(lc_1_pins);
    hx711_setup_pins(lc_2_pins);
}

float scales_get_ox(void) {
    float meaasurement = (float)hx711_read(lc_2_pins);
    float offset = meaasurement - 10440318.0625;

    if (meaasurement < 10452482.416185) {
        return offset / (10452482.416185 - 10440318.0625) * 0.45;
    } else if (meaasurement < 10468333.2669492) {
        return offset / (10468333.2669492 - 10452482.416185) * (0.91 - 0.45);
    } else {
        return offset / (10482717.0374532 - 10468333.2669492) * (1.36 - 0.91);
    }
}

float scales_get_fuel(void) {
    float meaasurement = (float)hx711_read(lc_1_pins);
    float offset = meaasurement - 8944650.78524946;

    if (meaasurement < 8956845.16356877) {
        return offset / (8956845.16356877 - 8944650.78524946) * 0.45;
    } else if (meaasurement < 8974977.11867704) {
        return offset / (8974977.11867704 - 8956845.16356877) * (0.91 - 0.45);
    } else {
        return offset / (8992145.71863118 - 8974977.11867704) * (1.36 - 0.91);
    }
}

float scales_get_thrust(void) {
    float meaasurement = (float)hx711_read(lc_0_pins);
    float offset = meaasurement - 8451232.04744526; // -20.3lbs
    
    if (meaasurement < 8476775.42) {
        float part = offset * (39.0 - 20.3) / (8476775.42 - 8451232.04744526);
        return part + 20.3;
    } else if (meaasurement < 8512086.28605201) {
        float part = offset * (69.2 - 39.0) / (8512086.28605201 - 8476775.42);
        return part + 20.3;
    } else {
        float part = offset * (107.9 - 69.2) / (8589435.9973545 - 8512086.28605201);
        return part + 20.3;
    }
}
