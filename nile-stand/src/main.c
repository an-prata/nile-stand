#include <stdbool.h>
#include <driver/gpio.h>
#include <esp_task_wdt.h>
#include <esp_timer.h>
#include <unistd.h>

#include "field.h"
#include "pressure_transducer.h"
#include "i2c.h"
#include "ads111x.h"
#include "solenoid_controller.h"
#include "uart.h"
#include "scales.h"
#include "timing.h"
#include "hx711.h"

#define IGNITE_TIME_S 10.0

#define ENABLE_PTS
#define ENABLE_LCS
#define ENABLE_SOLENOID_CONTROLLER
#define ENABLE_RATE_TRACKING

//#define SERIAL_UART
#define SERIAL_USB

#ifdef ENABLE_SOLENOID_CONTROLLER
static solenoid_controller_pins_t solenoid_pins = {
    .clock = GPIO_NUM_16,
    .data = GPIO_NUM_17,
};
#endif  /* ENABLE_SOLENOID_CONTROLLER */

/*
 * Ox Scale
 */

#ifdef ENABLE_LCS
static float scale_ox_value = 0.0;
static float scale_ox_value_rate = 0.0;

static field_t scale_ox_field = {
    .name = "Scale Ox",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0.0
        }
    }
};

static field_t scale_ox_rate_field = {
    .name = "Scale Ox Rate",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0.0
        }
    }
};


/*
 * Fuel Scale
 */

static float scale_fuel_value = 0.0;
static float scale_fuel_value_rate = 0.0;

static field_t scale_fuel_field = {
    .name = "Scale Fuel",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0.0
        }
    }
};

static field_t scale_fuel_rate_field = {
    .name = "Scale Fuel Rate",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0.0
        }
    }
};


/*
 * Ox/Fuel Ratio
 */

static field_t ox_fuel_ratio_field = {
    .name = "Ox/Fuel Ratio",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0.0
        }
    }
};


/*
 * Thrust Scale
 */

static float scale_thrust_value = 0.0;
static float scale_thrust_value_rate = 0.0;

static field_t scale_thrust_field = {
    .name = "Scale Thrust",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0.0
        }
    }
};

static field_t scale_thrust_rate_field = {
    .name = "Scale Thrust Rate",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0.0
        }
    }
};
#endif  /* ENABLE_LCS */


/*
 * Pressure Transducers
 */

#ifdef ENABLE_PTS
static field_t pressure_transducer_a0_field = {
    .name = "NPT1",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0
        }
    }
};

static field_t pressure_transducer_a1_field = {
    .name = "NPT3",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0
        }
    }
};

static field_t pressure_transducer_a2_field = {
    .name = "IPT1",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0
        }
    }
};

static field_t pressure_transducer_a3_field = {
    .name = "IPT3",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0
        }
    }
};
#endif  /* ENABLE_PTS */


/*
 * Valve fields
 */

#ifdef ENABLE_SOLENOID_CONTROLLER
static field_t valve_np1_field = {
    .name = "NP1",
    .value = {
        .field_type = FIELD_TYPE_BOOLEAN,
        .field_value = {
            .boolean = false
        }
    }
};

static field_t valve_np2_field = {
    .name = "NP2",
    .value = {
        .field_type = FIELD_TYPE_BOOLEAN,
        .field_value = {
            .boolean = false
        }
    }
};

static field_t valve_np3_field = {
    .name = "NP3",
    .value = {
        .field_type = FIELD_TYPE_BOOLEAN,
        .field_value = {
            .boolean = true
        }
    }
};

static field_t valve_np4_field = {
    .name = "NP4",
    .value = {
        .field_type = FIELD_TYPE_BOOLEAN,
        .field_value = {
            .boolean = false
        }
    }
};

static field_t valve_ip1_field = {
    .name = "IP1",
    .value = {
        .field_type = FIELD_TYPE_BOOLEAN,
        .field_value = {
            .boolean = false
        }
    }
};

static field_t valve_ip2_field = {
    .name = "IP2",
    .value = {
        .field_type = FIELD_TYPE_BOOLEAN,
        .field_value = {
            .boolean = false
        }
    }
};

static field_t valve_ip3_field = {
    .name = "IP3",
    .value = {
        .field_type = FIELD_TYPE_BOOLEAN,
        .field_value = {
            .boolean = true
        }
    }
};

static bool double_action_valve_triggered = false;
#endif  /* ENABLE_SOLENOID_CONTROLLER */

/*
 * Field for loop rate
 */

#ifdef ENABLE_RATE_TRACKING
static field_t update_rate_field = {
    .name = "Update Rate",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0.0
        }
    }
};

static field_t update_time_field = {
    .name = "Update Time",
    .value = {
        .field_type = FIELD_TYPE_FLOAT,
        .field_value = {
            .floating = 0.0,
        }
    }
};
#endif  /* ENABLE_RATE_TRACKING */

#define RX_BUF_LEN 256

char rx_buf[RX_BUF_LEN] = { '\0' };
size_t rx_idx = 0;

/*
 * Helpers for solenoid control.
 */

#ifdef ENABLE_SOLENOID_CONTROLLER
/**
 * Set a valve, open is `true`, closed is `false`.
 */
void set_valve(valve_e valve, bool state);

/**
 * Light the e-match on `true`, and for all sets of parameters check if the
 * e-match should have its power cut and do so if needed.
 */
void set_e_match(bool state);
#endif  /* ENABLE_SOLENOID_CONTROLLER */

void app_main() {
    sleep(2);
    uart_init();

#ifdef ENABLE_PTS
    i2c_init();
    ads111x_device_add();
#endif

#ifdef ENABLE_LCS
    scales_init();
#endif

#ifdef ENABLE_SOLENOID_CONTROLLER
    solenoid_controller_setup(solenoid_pins);
#endif

    command_reader_t command_reader = make_command_reader(NULL);
    command_t command;

    while (true) {
        timing_mark_loop();

        // PTs

#ifdef ENABLE_PTS
        pressure_transducer_a0_field.value.field_value.floating = pt_psi_from_volts(ads111x_read_voltage(ADS111X_CHANNEL_A0));
        pressure_transducer_a1_field.value.field_value.floating = pt_psi_from_volts(ads111x_read_voltage(ADS111X_CHANNEL_A1));
        pressure_transducer_a2_field.value.field_value.floating = pt_psi_from_volts(ads111x_read_voltage(ADS111X_CHANNEL_A2));
        pressure_transducer_a3_field.value.field_value.floating = pt_psi_from_volts(ads111x_read_voltage(ADS111X_CHANNEL_A3));
#endif

        // Scales - we also compute the rates of change of the scales for publishing, since thats
        // also fairly relevant information.

#ifdef ENABLE_LCS
        scales_update();

        float new_scale_ox_value = scales_get_ox();
        scale_ox_value_rate = timing_d_dt(scale_ox_value, new_scale_ox_value);
        scale_ox_value = new_scale_ox_value;
        scale_ox_field.value.field_value.floating = new_scale_ox_value;
        scale_ox_rate_field.value.field_value.floating = scale_ox_value_rate;

        float new_scale_fuel_value = scales_get_fuel();
        scale_fuel_value_rate = timing_d_dt(scale_fuel_value, new_scale_fuel_value);
        scale_fuel_value = new_scale_fuel_value;
        scale_fuel_field.value.field_value.floating = new_scale_fuel_value;
        scale_fuel_rate_field.value.field_value.floating = scale_fuel_value_rate;

        ox_fuel_ratio_field.value.field_value.floating = scale_ox_value_rate / scale_fuel_value_rate;

        float new_scale_thrust_value = scales_get_thrust();
        scale_thrust_value_rate = timing_d_dt(scale_thrust_value, new_scale_thrust_value);
        scale_thrust_field.value.field_value.floating = new_scale_thrust_value;
        scale_thrust_rate_field.value.field_value.floating = scale_thrust_value_rate;
#endif  /* ENABLE_LCS */

        // Update values of valves

#ifdef ENABLE_SOLENOID_CONTROLLER
        solenoid_controller_state_t solenoid_states = solenoid_controller_get();

        valve_np1_field.value.field_value.boolean = (solenoid_states & SOLENOID_0) > 0;
        valve_np2_field.value.field_value.boolean = (solenoid_states & SOLENOID_1) > 0;
        valve_np3_field.value.field_value.boolean = (solenoid_states & SOLENOID_2) == 0;
        valve_np4_field.value.field_value.boolean = (solenoid_states & SOLENOID_3) > 0;

        if (solenoid_states & SOLENOID_6) {
            valve_ip1_field.value.field_value.boolean = true;
        } else if (solenoid_states & SOLENOID_7) {
            valve_ip1_field.value.field_value.boolean = false;
        }
        
        valve_ip2_field.value.field_value.boolean = (solenoid_states & SOLENOID_4) > 0;
        valve_ip3_field.value.field_value.boolean = (solenoid_states & SOLENOID_5) == 0;
#endif

        // Field updates

#ifdef ENABLE_LCS
        update_field(scale_ox_field);
        update_field(scale_ox_rate_field);
        update_field(scale_fuel_field);
        update_field(scale_fuel_rate_field);
        update_field(ox_fuel_ratio_field);
        update_field(scale_thrust_field);
        update_field(scale_thrust_rate_field);
#endif  /* ENABLE_LCS */

#ifdef ENABLE_PTS
        update_field(pressure_transducer_a0_field);
        update_field(pressure_transducer_a1_field);
        update_field(pressure_transducer_a2_field);
        update_field(pressure_transducer_a3_field);
#endif

#ifdef ENABLE_SOLENOID_CONTROLLER
        update_field(valve_np1_field);
        update_field(valve_np2_field);
        update_field(valve_np3_field);
        update_field(valve_np4_field);

        if (double_action_valve_triggered) {
            update_field(valve_ip1_field);
        }

        update_field(valve_ip2_field);
        update_field(valve_ip3_field);
#endif

        // Handle commands

#ifdef SERIAL_USB
        rx_idx += fread(rx_buf, sizeof(char), RX_BUF_LEN, stdin);
#endif  /* SERIAL_USB */

#ifdef SERIAL_UART
        rx_idx += uart_recieve(rx_buf, RX_BUF_LEN);
#endif  /* SERIAL_UART */

        command_reader_buffer(&command_reader, rx_buf);

        while (command_reader_read(&command_reader, &command) == 0) {
            #ifdef ENABLE_SOLENOID_CONTROLLER
            switch (command.cmd_type) {
                case COMMAND_OPEN:
                    set_valve(command.cmd_valve, true);
                    break;
                case COMMAND_CLOSE:
                    set_valve(command.cmd_valve, false);
                    break;
                case COMMAND_IGNITE:
                    set_e_match(true);
                    break;
            }
            #endif  /* ENABLE_SOLENOID_CONTROLLER */
        }

        memset(rx_buf, '\0', RX_BUF_LEN);
        rx_idx = 0;

        // Update the solenoid controller, this should be done later in the loop since it blocks for
        // some time if enough time has not yet elapsed between calls, in which we ideally perform
        // some computation/gather data elsewhere.

#ifdef ENABLE_SOLENOID_CONTROLLER
        set_e_match(false);
        solenoid_controller_push(solenoid_pins);
#endif  /* ENABLE_SOLENOID_CONTROLLER */

#ifdef ENABLE_RATE_TRACKING
        update_time_field.value.field_value.floating = timing_delta_time_s();
        update_rate_field.value.field_value.floating = 1.0 / update_time_field.value.field_value.floating;

        update_field(update_time_field);
        update_field(update_rate_field);
#endif  /* ENABLE_RATE_TRACKING */

        // Delay so the watchdog doesn't bite
        //vTaskDelay(10);
    }
}

#ifdef ENABLE_SOLENOID_CONTROLLER
void set_valve(valve_e valve, bool state) {
    // Handle the weird double action valve.
    if (valve == IP1 && state) {
        solenoid_controller_open(SOLENOID_6);
        solenoid_controller_close(SOLENOID_7);
        double_action_valve_triggered = true;
        return;
    } else if (valve == IP1 && !state) {
        solenoid_controller_close(SOLENOID_6);
        solenoid_controller_open(SOLENOID_7);
        double_action_valve_triggered = true;
        return;
    }

    // Handle the two valves which fail open.
    if (valve == NP3 || valve == IP3) {
        state = !state;
    }

    solenoid_controller_state_t solenoid;

    switch (valve) {
        case NP1: solenoid = SOLENOID_0; break;
        case NP2: solenoid = SOLENOID_1; break;
        case NP3: solenoid = SOLENOID_2; break;
        case NP4: solenoid = SOLENOID_3; break;

        case IP2: solenoid = SOLENOID_4; break;
        case IP3: solenoid = SOLENOID_5; break;

        default: return;
    }

    if (state) {
        solenoid_controller_open(solenoid);
    } else {
        solenoid_controller_close(solenoid);
    }
}

/**
 * Sets the e-match high (ignites it) if `state` is true. Regardless of the
 * argument this function will set the e-match low after `IGNITE_TIME_S` has
 * elapsed since the last time the match was set high.
 */
void set_e_match(bool state) {
    static timing_marker_t ignite_marker;

    if (state) {
        ignite_marker = timing_mark();
        solenoid_controller_open(E_MATCH);
    }

    if (timing_time_since_s(ignite_marker) >= IGNITE_TIME_S) {
        solenoid_controller_close(E_MATCH);
    }
}
#endif  /* ENABLE_SOLENOID_CONTROLLER */