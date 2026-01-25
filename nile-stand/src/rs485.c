#include "rs485.h"

void rs485_init(
    rs485_t* rs485,
    uart_port_t read_port,
    int read_tx,
    int read_rx,
    uart_port_t write_port,
    int write_tx,
    int write_rx
) {
    uart_rs485_init(&rs485->read, read_port, read_tx, read_rx);
    uart_rs485_init(&rs485->write, write_port, write_tx, write_rx);
}

void rs485_send(rs485_t* rs485, const char* msg, size_t n) {
    uart_send(&rs485->write, msg, n);
}

size_t rs485_recieve(rs485_t* rs485, char* msg, size_t n) {
    return uart_recieve(&rs485->read, msg, n);
}