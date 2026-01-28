#ifndef RS485_H
#define RS485_H

#include "uart.h"

#define RS485_HANDOFF_CHAR '\r'
#define RS485_MAX_CONTROL_GAP_US (500 * 1000)  /* 500 Milliseconds */

typedef enum {
    RS485_ACTIVE,   /* Currently being waited on for a write. */
    RS485_WAITING,  /* Currently waiting to read.             */
    RS485_UNKNOWN,  /* Current state is unknown.              */
} rs485_control_state_t;

typedef enum {
    RS485_PRIMARY,    /* Initializes transaction, starting with a write.  */
    RS485_SECONDARY,  /* Waits for primary to transact, begins with read. */
} rs485_priority_t;

typedef struct {
    uart_t uart;
    rs485_control_state_t control_state;
    rs485_priority_t priority;
} rs485_t;

/**
 * Initialize a mediated RS485 connection. This RS485 interface wraps a UART
 * connection, and mediates the sending vs. recieving party.
 */
void rs485_init(rs485_t* rs485, rs485_priority_t priority, uart_port_t read_port, int read_tx, int read_rx);

/**
 * Perform a complete transaction over RS485, if the given `rs485_t` is setup as
 * `RS485_PRIMARY` then this transaction begins with a write, if the given
 * `rs485_t` is set up as `RS485_SECONDARY` then the transaction begins with a read.
 */
size_t rs485_transact(rs485_t* rs485, const char* tx_buf, size_t tx_len, char* rx_buf, size_t rx_len);

/**
 * Detect the current state (which party is presently sending) of the
 * connection.
 */
void rs485_detect_state(rs485_t* rs485);

#endif  /* RS485_H */