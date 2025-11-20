#include <driver/gpio.h>

#include "hx711.h"
#include "scales.h"

#define SCALE_OX 0
#define SCALE_FUEL 1
#define SCALE_THRUST 2

/* Scale 0 */
#define SCALE_0_LCS 2
#define SCALE_0_LC_0 GPIO_NUM_34
#define SCALE_0_LC_1 GPIO_NUM_35

/* Scale 1 */
#define SCALE_1_LCS 2
#define SCALE_1_LC_0 GPIO_NUM_36
#define SCALE_1_LC_1 GPIO_NUM_37

/* Scale 2 */
#define SCALE_2_LCS 3
#define SCALE_2_LC_0 GPIO_NUM_32
#define SCALE_2_LC_1 GPIO_NUM_33
#define SCALE_2_LC_2 GPIO_NUM_25

#define SCALE_COUNT 3
#define LC_CLOCK GPIO_NUM_26
#define LC_COUNT (SCALE_0_LCS + SCALE_1_LCS + SCALE_2_LCS)

typedef double (*scale_calibration_t)(uint32_t);

static const gpio_num_t lc_data_pins[LC_COUNT] = {
    SCALE_0_LC_0,
    SCALE_0_LC_1,
    
    SCALE_1_LC_0,
    SCALE_1_LC_1,
    
    SCALE_2_LC_0,
    SCALE_2_LC_1,
    SCALE_2_LC_2,
};

static double apply_scale_0_calibration(uint32_t measurement) {
    return ((double)measurement - 1053966.648) / 90.53951915;
}

static double apply_scale_1_calibration(uint32_t measurement) {
    return ((double)measurement - 103290.83242068) / 31.9587971598634;
}

static double apply_scale_2_calibration(uint32_t measurement) {
    return (double)measurement;
}

static const size_t scale_map[SCALE_COUNT] = { SCALE_0_LCS, SCALE_1_LCS, SCALE_2_LCS };
static const scale_calibration_t scale_calibrations[SCALE_COUNT] = {
    apply_scale_0_calibration,
    apply_scale_1_calibration,
    apply_scale_2_calibration
};

static uint32_t lc_measurements[LC_COUNT] = { 0 };
static uint32_t scale_measurements[SCALE_COUNT] = { 0 };

void scales_init(void) {
    hx711_setup_pins_many(LC_CLOCK, lc_data_pins, LC_COUNT);
}

void scales_update(void) {
    hx711_read_many(LC_CLOCK, lc_data_pins, lc_measurements, LC_COUNT);

    size_t offset = 0;
    size_t num_lcs = 0;

    for (int scale = 0; scale < SCALE_COUNT; scale++) {
        uint32_t measurement = 0;
        scale_measurements[scale] = 0;
        num_lcs = scale_map[scale];

        for (int lc = 0; lc < num_lcs; lc++) {
            scale_measurements[scale] += lc_measurements[offset + lc]; 
        }

        offset += scale_map[scale];
    }
}

double scales_get(size_t scale_num) {
    return scale_calibrations[scale_num](scale_measurements[scale_num]);
}

double scales_get_ox(void) { return scales_get(SCALE_OX); }
double scales_get_fuel(void) { return scales_get(SCALE_FUEL); }
double scales_get_thrust(void) { return scales_get(SCALE_THRUST); }
