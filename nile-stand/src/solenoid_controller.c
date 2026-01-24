#include <esp_timer.h>
#include <esp_task_wdt.h>
#include <unistd.h>
#include <i2c.h>
#include <stdint.h>

#include "solenoid_controller.h"

static solenoid_controller_state_t controller_state = 0;
static i2c_master_dev_handle_t controller = NULL;

void solenoid_controller_setup(solenoid_controller_pins_t pins) {
    if (controller) {
        return;
    }
    
    i2c_device_config_t conf = {
        .device_address = 0x0F,
        .dev_addr_length = I2C_ADDR_BIT_7,
        .scl_speed_hz = 400000,
    };

    i2c_device_add(&controller, &conf);
}

void solenoid_controller_open(solenoid_controller_state_t state) {
    controller_state |= state;
}

void solenoid_controller_close(solenoid_controller_state_t state) {
    controller_state &= ~state;
}

void solenoid_controller_push(solenoid_controller_pins_t pins) {
    solenoid_controller_set(pins, controller_state);
}

void solenoid_controller_set(solenoid_controller_pins_t pins, solenoid_controller_state_t state) {
    controller_state = state;
    i2c_write_raw(controller, (uint8_t*)&state, sizeof state);
}

solenoid_controller_state_t solenoid_controller_get(void) {
    return controller_state;
}