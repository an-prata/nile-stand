#include "pressure_transducer.h"

float pt_psi_from_volts(float volts) {
    float psi = 400.0 * (volts - 0.5);

    if (psi < 0.0) {
        return 0.0;
    }

    return psi;
}