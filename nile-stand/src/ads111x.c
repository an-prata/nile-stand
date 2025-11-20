#include <math.h>
#include <unistd.h>

#include "ads111x.h"
#include "i2c.h"

#define ADS111X_MUX_POS 12
#define ADS111X_CONFIG_START_CONVERSION 0x8000
#define ADS111X_CONFIG_RESET 0x0583

typedef uint16_t ads111x_config_t;

typedef enum {
    ADS111X_REG_CONVERSION = 0b00,
    ADS111X_REG_CONFIG = 0b01
} ads111x_register_e;

static i2c_master_dev_handle_t handle;

static void ads111x_set_channel(ads111x_channel_e channel) {
    ads111x_config_t mux = 0;

    switch (channel) {
        case ADS111X_CHANNEL_A0: mux = 0x40; break;
        case ADS111X_CHANNEL_A1: mux = 0x50; break;
        case ADS111X_CHANNEL_A2: mux = 0x60; break;
        case ADS111X_CHANNEL_A3: mux = 0x70; break;
        default: mux = 0x40; break;
    }

    ads111x_config_t config
        = mux
        | ADS111X_CONFIG_START_CONVERSION
        | ADS111X_CONFIG_RESET;
    i2c_write(handle, ADS111X_REG_CONFIG, (uint8_t*)(&config), sizeof(ads111x_config_t));
}

void ads111x_device_add(void) {
    i2c_device_config_t conf = {
        .device_address = ADS111X_ADDR,
        .dev_addr_length = I2C_ADDR_BIT_7,
        .scl_speed_hz = 100000,
    };

    i2c_device_add(&handle, &conf);
}

uint16_t ads111x_read(ads111x_channel_e channel) {
    uint8_t data[2] = { 0 };
    ads111x_config_t state = 0;

    ads111x_set_channel(channel);
    usleep(200);

    do {
        i2c_read(handle, ADS111X_REG_CONFIG, data, 2);
        state = (data[0] << 8) | data[1];
    } while (!(state & ADS111X_CONFIG_START_CONVERSION));

    i2c_read(handle, ADS111X_REG_CONVERSION, data, 2);

    uint16_t value = (data[0] << 8) | data[1];
   
    return value;
}

float ads111x_read_voltage(ads111x_channel_e channel) {
    const float max = powf(2.0, 15.0);
    int16_t raw = (int16_t)ads111x_read(channel);
    //raw = (int16_t)ads111x_read(channel);

    float scaled = (float)raw / max;
    return scaled * 5.0;
}
