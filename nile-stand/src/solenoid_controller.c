#include <esp_timer.h>
#include <esp_task_wdt.h>
#include <unistd.h>
#include <i2c.h>
#include <stdint.h>

#include "solenoid_controller.h"

#define OPEN '|'
#define CLOSED '-'

void solenoid_controller_init(
    solenoid_controller_t* solenoid_controller,
    uart_port_t port,
    gpio_num_t pin_tx,
    gpio_num_t pin_rx
) {
    uart_init(&solenoid_controller->uart, port, pin_tx, pin_rx);
}

void solenoid_controller_open(solenoid_controller_t* solenoid_controller, solenoid_controller_state_t state) {
    solenoid_controller->state |= state;
}

void solenoid_controller_close(solenoid_controller_t* solenoid_controller, solenoid_controller_state_t state) {
    solenoid_controller->state &= ~state;
}

void solenoid_controller_push(solenoid_controller_t* solenoid_controller) {
    char msg[sizeof solenoid_controller->state * 8 + 2];

    for (int i = 0; i < sizeof solenoid_controller->state * 8; i++) {
        msg[i + 1] = (solenoid_controller->state & (1 << i)) 
            ? OPEN
            : CLOSED;
    }

    msg[0] = '\n';
    msg[sizeof solenoid_controller->state * 8 + 1] = '\n';

    uart_send(&solenoid_controller->uart, msg, sizeof solenoid_controller->state * 8 + 2);
}

void solenoid_controller_set(solenoid_controller_t* solenoid_controller, solenoid_controller_state_t state) {
    solenoid_controller->state = state;
}

solenoid_controller_state_t solenoid_controller_get(solenoid_controller_t* solenoid_controller) {
    return solenoid_controller->state;
}