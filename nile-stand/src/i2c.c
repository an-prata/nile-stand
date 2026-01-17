#include <string.h>
#include <i2c.h>
#include <esp_task_wdt.h>
#include <esp_timer.h>

// TODO: Maybe watchdog gets impatient with this timeout?
// also see `taskYIELD()` to maybe switch contexts?
#define I2C_MASTER_TIMEOUT_MS 100
#define I2C_RETRY_MAX 12

static i2c_master_bus_handle_t handle;

void i2c_init(void) {
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM,
        .sda_io_num = I2C_DATA,
        .scl_io_num = I2C_CLOCK,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
        .intr_priority = 3,
    };
    
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &handle));
    ESP_ERROR_CHECK(i2c_master_bus_wait_all_done(handle, I2C_MASTER_TIMEOUT_MS));
}

void i2c_device_add(i2c_master_dev_handle_t* dev, i2c_device_config_t* config) {
    ESP_ERROR_CHECK(i2c_master_bus_add_device(handle, config, dev));
    ESP_ERROR_CHECK(i2c_master_bus_wait_all_done(handle, I2C_MASTER_TIMEOUT_MS));
}

void i2c_device_remove(i2c_master_dev_handle_t dev) {
    ESP_ERROR_CHECK(i2c_master_bus_rm_device(dev));
    ESP_ERROR_CHECK(i2c_master_bus_wait_all_done(handle, I2C_MASTER_TIMEOUT_MS));
}

void i2c_read(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t* data, size_t n) {
    esp_err_t err;
    int retries = 0;

    do {
        retries++;
        err = i2c_master_transmit_receive(
            dev,
            &reg,
            1,
            data,
            n, 
            I2C_MASTER_TIMEOUT_MS
        );

        ESP_ERROR_CHECK(i2c_master_bus_wait_all_done(handle, I2C_MASTER_TIMEOUT_MS));
    } while (err != ESP_OK && retries <= I2C_RETRY_MAX);


    if (err != ESP_OK) {
        ESP_ERROR_CHECK(err);
    }
}

void i2c_write(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t* data, size_t n) {
    uint8_t* msg = malloc(1 + n);

    if (!msg) {
        abort();
    }

    msg[0] = reg;
    memcpy(&msg[1], data, n);

    esp_err_t err;
    int retries = 0;

    do {
        retries++;
        err = i2c_master_transmit(dev, msg, 1 + n, I2C_MASTER_TIMEOUT_MS);
        ESP_ERROR_CHECK(i2c_master_bus_wait_all_done(handle, I2C_MASTER_TIMEOUT_MS));
    } while (err != ESP_OK && retries <= I2C_RETRY_MAX);

    if (err != ESP_OK) {
        ESP_ERROR_CHECK(err);
    }
}

void i2c_write_raw(i2c_master_dev_handle_t dev, uint8_t* data, size_t n) {
    esp_err_t err;
    int retries = 0;

    do {
        retries++;
        err = i2c_master_transmit(dev, data, n, I2C_MASTER_TIMEOUT_MS);
        ESP_ERROR_CHECK(i2c_master_bus_wait_all_done(handle, I2C_MASTER_TIMEOUT_MS));
    } while (err != ESP_OK && retries <= I2C_RETRY_MAX);

    if (err != ESP_OK) {
        ESP_ERROR_CHECK(err);
    }
}