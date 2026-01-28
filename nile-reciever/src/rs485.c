#include "rs485.h"
#include "timing.h"

static const char handoff_buf = RS485_HANDOFF_CHAR;

static void rs485_send(rs485_t* rs485, const char* msg, size_t n) {
    uart_send(&rs485->uart, msg, n);
}

static size_t rs485_recieve(rs485_t* rs485, char* msg, size_t n) {
    return uart_recieve(&rs485->uart, msg, n);
}

void rs485_init(
    rs485_t* rs485,
    rs485_priority_t priority,
    uart_port_t read_port,
    int read_tx,
    int read_rx
) {
    uart_rs485_init(&rs485->uart, read_port, read_tx, read_rx);
    rs485->priority = priority;

    /* 
     * Secondaries can safely assume they are waiting to read because primaries
     * will detect the state of system as a whole on startup.
     */

    if (priority == RS485_SECONDARY) {
        rs485->control_state = RS485_WAITING;
    } else {
        rs485->control_state = RS485_UNKNOWN;
    }
}

size_t rs485_transact(rs485_t* rs485, const char* tx_buf, size_t tx_len, char* rx_buf, size_t rx_len) {
    size_t recieved = 0;

    if (rs485->control_state == RS485_UNKNOWN) {
        rs485_detect_state(rs485);
    }

recieve:
    if (rs485->control_state == RS485_WAITING && recieved == 0) {
        recieved = rs485_recieve(rs485, rx_buf, rx_len);

        if (recieved != 0 && rx_buf[recieved - 1] == RS485_HANDOFF_CHAR) {
            /* This device has been handed control over the RS485 connection. */
            rs485->control_state = RS485_ACTIVE;
        }
    }
    
    if (rs485->control_state == RS485_ACTIVE && tx_len > 0) {
        rs485_send(rs485, tx_buf, tx_len);
        rs485_send(rs485, &handoff_buf, 1);  /* Always handoff after a write. */
        rs485->control_state = RS485_WAITING;
        tx_len = 0;
        goto recieve;
    }
    
    return recieved;
}

void rs485_detect_state(rs485_t* rs485) {
    timing_marker_t start_marker = timing_mark();
    size_t buffered_data = 0;

    while (timing_time_since_us(start_marker) < RS485_MAX_CONTROL_GAP_US) {
        ESP_ERROR_CHECK(uart_get_buffered_data_len(rs485->uart.port, &buffered_data));

        if (buffered_data > 0) {
            /* Data is available, we need to recieve it. */
            rs485->control_state = RS485_WAITING;
            return;
        }
    }

    /* Timed out, indicating that this device is being waited on. */
    rs485->control_state = RS485_ACTIVE;
}