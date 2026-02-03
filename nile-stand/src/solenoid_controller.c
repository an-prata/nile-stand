#include <esp_timer.h>
#include <esp_task_wdt.h>
#include <unistd.h>
#include <i2c.h>
#include <stdint.h>

#include "solenoid_controller.h"

#define RECOVERY_RETRY_COUNT 16

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
    char msg[sizeof solenoid_controller->state * 8 + 1];

    for (int i = 0; i < sizeof solenoid_controller->state * 8; i++) {
        msg[i] = (solenoid_controller->state & (1 << i)) 
            ? OPEN
            : CLOSED;
    }

    msg[sizeof solenoid_controller->state * 8] = '\n';

    uart_send(&solenoid_controller->uart, msg, sizeof solenoid_controller->state * 8 + 2);
}

void solenoid_controller_set(solenoid_controller_t* solenoid_controller, solenoid_controller_state_t state) {
    solenoid_controller->state = state;
}

solenoid_controller_state_t solenoid_controller_get(solenoid_controller_t* solenoid_controller) {
    return solenoid_controller->state;
}

int solenoid_controller_try_recover_state(solenoid_controller_t* solenoid_controller) {
    char buf[sizeof solenoid_controller->state * 8];
    size_t recieved = 0;

    for (int i = 0; i < RECOVERY_RETRY_COUNT; i++) {
        recieved = uart_recieve(&solenoid_controller->uart, buf, 1);
        
        if (recieved != 1) {
            continue;
        }

        if (recieved == 1 && buf[0] == '\n') {
            goto recover;
        }
    }

    return 1;

recover:
    solenoid_controller_state_t recovered_state = 0;
    recieved = uart_recieve(&solenoid_controller->uart, buf, sizeof solenoid_controller->state * 8);

    if (recieved != sizeof solenoid_controller->state * 8) {
        return 1;
    }

    for (int i = sizeof solenoid_controller->state * 8 - 1; i >= 0; i--) {
        recovered_state = recovered_state << 1;
        recovered_state |= buf[i] == OPEN;
    }

    solenoid_controller->state = recovered_state;
    return 0;
}