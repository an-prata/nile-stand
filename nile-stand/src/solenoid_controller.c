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
    solenoid_controller->state = 0;

    solenoid_controller_try_recover_state(solenoid_controller);
}

void solenoid_controller_open(solenoid_controller_t* solenoid_controller, solenoid_controller_state_t state) {
    solenoid_controller->state |= state;
}

void solenoid_controller_close(solenoid_controller_t* solenoid_controller, solenoid_controller_state_t state) {
    solenoid_controller->state &= ~state;
}

void solenoid_controller_push(solenoid_controller_t* solenoid_controller) {
    char open = OPEN;
    char closed = CLOSED;

    for (int i = 0; i < SOLENOID_COUNT; i++) {
        if (solenoid_controller->state & (1 << i)) {
            uart_send(&solenoid_controller->uart, &open, 1);
        } else {
            uart_send(&solenoid_controller->uart, &closed, 1);
        }
    }

    char newline = '\n';
    uart_send(&solenoid_controller->uart, &newline, sizeof newline);
    
    char temp_buf[256];
    uart_recieve(&solenoid_controller->uart, temp_buf, 256);
}

solenoid_controller_state_t solenoid_controller_get(solenoid_controller_t* solenoid_controller) {
    return solenoid_controller->state;
}

int solenoid_controller_try_recover_state(solenoid_controller_t* solenoid_controller) {
    char buf[SOLENOID_COUNT];

    for (int i = 0; i < RECOVERY_RETRY_COUNT * SOLENOID_COUNT; i++) {
        char rx_char;
        size_t recieved = uart_recieve(&solenoid_controller->uart, &rx_char, 1);

        printf("Recovery recieved %d bytes\n", recieved);

        if (recieved == 1 && rx_char == '\n') {
            goto recover;
        }

        vTaskDelay(5);
    }

    printf("Recovery failed to start\n");
    return 1;

recover:
    solenoid_controller_state_t recovered_state = 0;
    size_t recieved = uart_recieve(&solenoid_controller->uart, buf, SOLENOID_COUNT);

    if (recieved != SOLENOID_COUNT) {
        printf("Recovery failed with malformed data\n");
        return 1;
    }

    if (buf[0] == OPEN) { recovered_state |= SOLENOID_0; }
    if (buf[1] == OPEN) { recovered_state |= SOLENOID_1; }
    if (buf[2] == OPEN) { recovered_state |= SOLENOID_2; }
    if (buf[3] == OPEN) { recovered_state |= SOLENOID_3; }
    if (buf[4] == OPEN) { recovered_state |= SOLENOID_4; }
    if (buf[5] == OPEN) { recovered_state |= SOLENOID_5; }
    if (buf[6] == OPEN) { recovered_state |= SOLENOID_6; }
    if (buf[7] == OPEN) { recovered_state |= SOLENOID_7; }
    if (buf[8] == OPEN) { recovered_state |= E_MATCH; }
    if (buf[9] == OPEN) { recovered_state |= SIGNAL_LIGHT_1; }
    if (buf[10] == OPEN) { recovered_state |= SIGNAL_LIGHT_2; }
    if (buf[11] == OPEN) { recovered_state |= SIGNAL_LIGHT_3; }

    solenoid_controller->state = recovered_state;
    printf("Recovery succeeded!\n");
    return 0;
}