/*
 * This header contains procedures which are meant to, for a time, override the
 * typical control from from `main` in favor of a routine which prioritizes a
 * subset of the stand's typical feature set in order to maximize performance on
 * that subset of features for a period of time.
 * 
 * These procedures should be self terminating, or enable some set of features
 * which would allow for them to be terminated.
 * 
 * While one of these are running it is feasible that the software failsafe will
 * be inoperable, use with care.
 */

#ifndef SPECIAL_PROCEDURES_H
#define SPECIAL_PROCEDURES_H

#include "solenoid_controller.h"
#include "field.h"
#include "rs485.h"

/**
 * Sacrifices all stand features except actuation of valves and measurement of
 * the NPT1 pressure transducer for measuring the delay between actuation of the
 * NP1 valve and fuel hitting the engine.
 * 
 * While this procedure is active the only typical field that will be reported
 * is the measurement from IPT1 and the stand time.
 */
void special_procedure_delay_fuel(
    rs485_t* rs485,
    solenoid_controller_t* solenoid_controller,
    field_t* field_ipt1
);

void special_procedure_delay_ox(void);

void special_procedure_engine_ignition(void);

#endif  /* SPECIAL_PROCEDURES_H */