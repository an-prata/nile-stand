#include "special_procedures.h"
#include "pressure_transducer.h"
#include "timing.h"
#include "ads111x.h"

#define TX_BUF_LEN 1024
#define RX_BUF_LEN 1024

#define DELAY_PROCEDURE_LEN_S 15.0

void special_procedure_delay_fuel(
    rs485_t* rs485,
    solenoid_controller_t* solenoid_controller,
    field_t* field_ipt1
) {
    timing_marker_t start_time = timing_mark();
    
    char tx_buf[TX_BUF_LEN] = { 0 };
    size_t tx_idx = 0;
    /* Not processing input, no rx needed */

    field_t sp_time_field = {
        .name = "SP Time",
        .value = {
            .field_type = FIELD_TYPE_FLOAT,
            .field_value = { .floating = 0.0 }
        }
    };

    field_t sp_rate_field = {
        .name = "SP Rate",
        .value = {
            .field_type = FIELD_TYPE_FLOAT,
            .field_value = { .floating = 0.0 }
        }
    };

    while (1) {
        timing_mark_loop();

        /* Dont process commands and send as a single large packet without handing off. */
        uart_send(&rs485->uart, tx_buf, tx_idx);
        tx_idx = 0;

        //if (timing_time_since_s(start_time) > DELAY_PROCEDURE_LEN_S) {
        //    /* Procedure finished. */
        //    break;
        //}

        /* Measure near-engine pressure on fuel side */
        field_ipt1->value.field_value.floating = ipt1_calibration(ads111x_read_voltage(ADS111X_CHANNEL_A2));
        sp_time_field.value.field_value.floating = timing_time_since_s(start_time);    
        sp_rate_field.value.field_value.floating = 1.0 / timing_delta_time_s();

        tx_idx += update_field(tx_buf, TX_BUF_LEN, tx_idx, *field_ipt1);
        tx_idx += update_field(tx_buf, TX_BUF_LEN, tx_idx, sp_time_field);
        tx_idx += update_field(tx_buf, TX_BUF_LEN, tx_idx, sp_rate_field);
    }

    /* Complete a very large RS485 transaction. */
    char rx_buf[RX_BUF_LEN] = { 0 };
    size_t recieved = rs485_transact(rs485, tx_buf, 0, rx_buf, RX_BUF_LEN);
}