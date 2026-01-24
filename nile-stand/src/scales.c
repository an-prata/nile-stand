#include <driver/gpio.h>

#include "hx711.h"
#include "scales.h"

#define SCALE_OX 1
#define SCALE_FUEL 0
#define SCALE_THRUST 2

/* Scale 0 */
#define SCALE_0_LCS 1
//#define SCALE_0_LC_0 GPIO_NUM_34
#define SCALE_0_LC_1 GPIO_NUM_35

/* Scale 1 */
#define SCALE_1_LCS 1
#define SCALE_1_LC_0 GPIO_NUM_36
#define SCALE_1_LC_1 GPIO_NUM_39

/* Scale 2 */
#define SCALE_2_LCS 0
#define SCALE_2_LC_0 GPIO_NUM_32
#define SCALE_2_LC_1 GPIO_NUM_33
#define SCALE_2_LC_2 GPIO_NUM_25

#define SCALE_COUNT 3
#define LC_CLOCK GPIO_NUM_26
#define LC_COUNT (SCALE_0_LCS + SCALE_1_LCS + SCALE_2_LCS)

typedef float (*scale_calibration_t)(uint32_t);

static const gpio_num_t lc_data_pins[LC_COUNT] = {
    //SCALE_0_LC_0,
    SCALE_0_LC_1,
    
    //SCALE_1_LC_0,
    SCALE_1_LC_1,
    
    //SCALE_2_LC_0,
    //SCALE_2_LC_1,
    //SCALE_2_LC_2,
};

static float apply_scale_0_calibration(uint32_t measurement) {
    return (float)measurement;
}

static float apply_scale_1_calibration(uint32_t measurement) {
    return (float)measurement;
}

static float apply_scale_2_calibration(uint32_t measurement) {
    return (float)measurement;
}

static uint32_t lc_measurements[LC_COUNT] = { 200 };
static uint32_t scale_measurements[SCALE_COUNT] = { 600 };

void scales_init(void) {
    hx711_setup_pins_many(LC_CLOCK, lc_data_pins, LC_COUNT);
}

void scales_update(void) {
    hx711_read_many(LC_CLOCK, lc_data_pins, lc_measurements, LC_COUNT);

    scale_measurements[SCALE_FUEL] = lc_measurements[0];
    scale_measurements[SCALE_OX] = lc_measurements[1] /*+ lc_measurements[2]*/;
    //scale_measurements[SCALE_THRUST] = lc_measurements[3] + lc_measurements[4] + lc_measurements[5];
}

float scales_get_ox(void) { return (float)scale_measurements[SCALE_OX] * 31968 + 1.189E6; }
float scales_get_fuel(void) { return (float)scale_measurements[SCALE_FUEL]; }
float scales_get_thrust(void) { return (float)scale_measurements[SCALE_THRUST]; }
