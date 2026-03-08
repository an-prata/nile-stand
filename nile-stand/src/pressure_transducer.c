#include "pressure_transducer.h"

float npt1_calibration(float volts) {
    float offset = volts - 1.13711834628975;

    if (volts < 1.221321) {
        return offset * (30 - 0) / (1.221321 - 1.13711834628975);
    } else if (volts < 1.471524) {
        return offset * (60 - 30) / (1.471524 - 1.221321);
    } else {
        return offset * (100 - 60) / (1.69609592307692 - 1.471524);
    }
}

float npt3_calibration(float volts) {
    float offset = volts - 1.104024;

    if (volts < 1.189940) {
        return offset * (30 - 0) / (1.189940 - 1.104024);
    } else if (volts < 1.439086) {
        return offset * (60 - 30) / (1.439086 - 1.189940);
    } else {
        return offset * (100 - 60) / (1.66217707692308 - 1.439086);
    }
}

float ipt1_calibration(float volts) {
    float offset = volts - 1.123845;

    if (volts < 1.22441773793103) {
        return offset * (30 - 0) / (1.22441773793103 - 1.123845);
    } else if (volts < 1.419481) {
        return offset * (60 - 30) / (1.419481 - 1.22441773793103);
    } else {
        return offset * (100 - 60) / (1.690581 - 1.419481);
    }
}

float ipt3_calibration(float volts) {
    float offset = volts - 1.103688;

    if (volts < 1.199300) {
        return offset * (30 - 0) / (1.199300 - 1.103688);
    } else if (volts < 1.39695587955182) {
        return offset * (60 - 30) / (1.39695587955182 - 1.199300);
    } else {
        return offset * (100 - 60) / (1.670854 - 1.39695587955182);
    }
}
