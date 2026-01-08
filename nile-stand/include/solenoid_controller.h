#ifndef SOLENOID_CONTROLLER_H
#define SOLENOID_CONTROLLER_H

#include <stdint.h>
#include <driver/gpio.h>

#define PULSE_DELIMETER 500  /* Time between state sets, in this time the clock is high and data is low */

typedef struct {
    gpio_num_t clock;
    gpio_num_t data;
} solenoid_controller_pins_t;

#define SOLENOID_0     0x0001
#define SOLENOID_1     0x0002
#define SOLENOID_2     0x0004
#define SOLENOID_3     0x0008
#define SOLENOID_4     0x0010
#define SOLENOID_5     0x0020
#define SOLENOID_6     0x0040
#define SOLENOID_7     0x0080
#define SOLENOID_8     0x0100
#define E_MATCH        0x0200
#define SIGNAL_LIGHT_1 0x0400
#define SIGNAL_LIGHT_2 0x0800
#define SIGNAL_LIGHT_3 0x1000

typedef uint16_t solenoid_controller_state_t;

/**
 * Set up a solenoid controller at the given pins.
 */
void solenoid_controller_setup(solenoid_controller_pins_t pins);

/**
 * Update an internal record of the desired state of the solenoid controller by
 * setting it to also open the given solenoids.
 */
void solenoid_controller_open(solenoid_controller_state_t state);

/**
 * Update an internal record of the desired state of the solenoid controller by
 * setting it to also close the given solenoids.
 */
void solenoid_controller_close(solenoid_controller_state_t state);

/**
 * Push the internal record of desired state to the solenoid controller.
 * 
 * Careful when you call this function, it will block until `PULSE_DELIMETER`
 * microseconds have elapsed since the last time the solenoid controller state
 * was pushed/set.
 */
void solenoid_controller_push(solenoid_controller_pins_t pins);

/**
 * Set the state of the solenoid controller. The state is an integer who's bits
 * corrospond to solenoids at the same position. So `0b1100` would set solenoids
 * `0` and `1` closed and solenoids `2` and `3` open.
 * 
 * Careful when you call this function, it will block until `PULSE_DELIMETER`
 * microseconds have elapsed since the last time the solenoid controller state
 * was pushed/set.
 */
void solenoid_controller_set(solenoid_controller_pins_t pins, solenoid_controller_state_t state);

/**
 * Get the current record of the solenoid controller's state.
 */
solenoid_controller_state_t solenoid_controller_get(void);

#endif  /* SOLENOID_CONTROLLER_H */