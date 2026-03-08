#ifndef ADS111X_H
#define ADS111X_H

#include <driver/i2c_master.h>

#define ADS111X_ADDR 0b1001000

typedef enum { 
    ADS111X_CHANNEL_A0 = 0b00,
    ADS111X_CHANNEL_A1 = 0b01,
    ADS111X_CHANNEL_A2 = 0b10,
    ADS111X_CHANNEL_A3 = 0b11
} ads111x_channel_e;

/**
 * Add an I2C device for the ADS111X.
 */
void ads111x_device_add(void);

/**
 * Set the current analog channel for reading, for use with unsafe functions.
 */
void ads111x_set_channel(ads111x_channel_e channel);

/**
 * Read out a 16 bit analog conversion from the given channel. Negative values
 * do in fact represent negative voltages?
 */
uint16_t ads111x_read(ads111x_channel_e channel);

/**
 * Read out 16 bit analog conversion from whatever the current channel is.
 */
uint16_t ads111x_read_unsafe();

/**
 * Read out a value from the given ADS111X channel and convert it to a humanly
 * readable voltage.
 */
float ads111x_read_voltage(ads111x_channel_e channel);

/**
 * Read voltage off of whatever the current channel is.
 */
float ads111x_read_voltage_unsafe();

#endif  /* ADS111X_H */