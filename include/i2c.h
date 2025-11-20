#ifndef I2C_H
#define I2C_H

#include <sdkconfig.h>
#include <driver/i2c_master.h>

#define I2C_NUM I2C_NUM_0
#define I2C_CLOCK GPIO_NUM_22
#define I2C_DATA GPIO_NUM_21 

/**
 * Initialize the I2C bus.
 */
void i2c_init(void);

/**
 * Register and add a device by the specific configuration given.
 */
void i2c_device_add(i2c_master_dev_handle_t* dev, i2c_device_config_t* config);

/**
 * Remove the given I2C device.
 */
void i2c_device_remove(i2c_master_dev_handle_t dev);

/**
 * Reads at most `n` bytes over I2C from the given `i2c_master_dev_handle_t`.
 */
void i2c_read(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t* data, size_t n);

/**
 * Write data, over I2C, to the device pointed to by the given
 * `i2c_master_dev_handle_t`.
 */
void i2c_write(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t* data, size_t n);

#endif  /* I2C_H */