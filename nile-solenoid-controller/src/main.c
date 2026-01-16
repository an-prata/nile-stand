#include <driver/gpio.h>
#include <esp_task_wdt.h>
#include <esp_timer.h>
#include <esp_event.h>
#include <driver/i2c_slave.h>
#include <unistd.h>

#define PULSE_DELIMETER 1000

#define PIN_SOLENOID_0     GPIO_NUM_32
#define PIN_SOLENOID_1     GPIO_NUM_33
#define PIN_SOLENOID_2     GPIO_NUM_25
#define PIN_SOLENOID_3     GPIO_NUM_26
#define PIN_SOLENOID_4     GPIO_NUM_19
#define PIN_SOLENOID_5     GPIO_NUM_17
#define PIN_SOLENOID_6     GPIO_NUM_16
#define PIN_SOLENOID_7     GPIO_NUM_23
#define PIN_E_MATCH        GPIO_NUM_12
#define PIN_SIGNAL_LIGHT_1 GPIO_NUM_13
#define PIN_SIGNAL_LIGHT_2 GPIO_NUM_14
#define PIN_SIGNAL_LIGHT_3 GPIO_NUM_15

#define SOLENOID_0     0x0001
#define SOLENOID_1     0x0002
#define SOLENOID_2     0x0004
#define SOLENOID_3     0x0008
#define SOLENOID_4     0x0010
#define SOLENOID_5     0x0020
#define SOLENOID_6     0x0040
#define SOLENOID_7     0x0080
#define SOLENOID_8     0x0100
#define E_MATCH        0x0200
#define SIGNAL_LIGHT_1 0x0400
#define SIGNAL_LIGHT_2 0x0800
#define SIGNAL_LIGHT_3 0x1000

typedef struct {
    QueueHandle_t event_queue;
    uint16_t command_data;
    i2c_slave_dev_handle_t handle;
} controller_context_t;

typedef enum {
    I2C_SLAVE_EVT_RX,
    I2C_SLAVE_EVT_TX
} i2c_slave_event_t;

static uint16_t solenoid_states;
static controller_context_t context;

static void pin_set(uint16_t mask, gpio_num_t pin) {
    if (solenoid_states & mask) {
        gpio_set_level(pin, 1);
        printf("Pin %i: HI\n", pin);
    } else {
        gpio_set_level(pin, 0);
        printf("Pin %i: LO\n", pin);
    }
}

static bool recv_callback(i2c_slave_dev_handle_t i2c_slave, const i2c_slave_rx_done_event_data_t *event_data, void *arg) {
    controller_context_t* context = arg; 
    i2c_slave_event_t evt = I2C_SLAVE_EVT_RX;
    BaseType_t xTaskWoken = 0;
    context->command_data = *event_data->buffer;
    xQueueSendFromISR(context->event_queue, &evt, &xTaskWoken);
    return xTaskWoken;
}

static void i2c_slave_task(void *arg)
{
    controller_context_t* context = (controller_context_t*)arg;
    i2c_slave_dev_handle_t handle = (i2c_slave_dev_handle_t)context->handle;

    while (true) {
        i2c_slave_event_t evt;

        if (xQueueReceive(context->event_queue, &evt, 10) == pdTRUE) {
            if (evt == I2C_SLAVE_EVT_RX) {
                solenoid_states = context->command_data;

                pin_set(SOLENOID_0, PIN_SOLENOID_0);
                pin_set(SOLENOID_1, PIN_SOLENOID_1);
                pin_set(SOLENOID_2, PIN_SOLENOID_2);
                pin_set(SOLENOID_3, PIN_SOLENOID_3);
                pin_set(SOLENOID_4, PIN_SOLENOID_4);
                pin_set(SOLENOID_5, PIN_SOLENOID_5);
                pin_set(SOLENOID_6, PIN_SOLENOID_6);
                pin_set(SOLENOID_7, PIN_SOLENOID_7);
                pin_set(E_MATCH, PIN_E_MATCH);
                pin_set(SIGNAL_LIGHT_1, PIN_SIGNAL_LIGHT_1);
                pin_set(SIGNAL_LIGHT_2, PIN_SIGNAL_LIGHT_2);
                pin_set(SIGNAL_LIGHT_3, PIN_SIGNAL_LIGHT_3);
                
                printf("Got: %04X\n", solenoid_states);
            }
        }
    }

    vTaskDelete(NULL);
}

void app_main() {
    sleep(1);
    
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    gpio_set_direction(PIN_SOLENOID_0, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_3, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_4, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_5, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_6, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SOLENOID_7, GPIO_MODE_OUTPUT);

    gpio_set_direction(PIN_E_MATCH, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SIGNAL_LIGHT_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SIGNAL_LIGHT_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SIGNAL_LIGHT_3, GPIO_MODE_OUTPUT);

    printf("Starting!\n");

    i2c_slave_config_t i2c_config = {
        .i2c_port = I2C_NUM_0,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .scl_io_num = GPIO_NUM_22,
        .sda_io_num = GPIO_NUM_21,
        .slave_addr = 0x0F,
        .send_buf_depth = 100,
        .receive_buf_depth = 100,
    };

    i2c_slave_event_callbacks_t callbacks = {
        .on_receive = recv_callback,
        .on_request = NULL
    };

    context.event_queue = xQueueCreate(16, sizeof(i2c_slave_event_t));

    ESP_ERROR_CHECK(i2c_new_slave_device(&i2c_config, &context.handle));
    ESP_ERROR_CHECK(i2c_slave_register_event_callbacks(context.handle, &callbacks, &context));
    xTaskCreate(i2c_slave_task, "i2c_slave_task", 1024 * 4, &context, 10, NULL);
}