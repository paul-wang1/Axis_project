/*
 * ESP32 I2C Slave - Test Configuration
 * ESP-IDF 5.5 Compatible
 *
 * This code sets up the ESP32 as an I2C slave that responds to
 * commands from the DaisySeed (master) with dummy test data.
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/i2c_slave.h"

static const char *TAG = "i2c_slave";

/* I2C Configuration - Adjust these pins for your ESP32-C3 wiring */
#define I2C_SLAVE_SCL_IO    21                   /* GPIO for I2C clock */
#define I2C_SLAVE_SDA_IO    20                   /* GPIO for I2C data */
#define I2C_SLAVE_NUM       0
#define ESP_SLAVE_ADDR      0x28                /* 7-bit I2C address */
#define I2C_SLAVE_RAM_SIZE  128                 /* Size of slave RAM buffer */

/* Command bytes the master (DaisySeed) will send */
#define CMD_GET_VALUE_A     0x10
#define CMD_GET_VALUE_B     0x20
#define CMD_GET_VALUE_C     0x30
#define CMD_GET_STRING      0x40

/* Dummy test data */
static int dummy_value_a = 12345;
static int dummy_value_b = 67890;
static int dummy_value_c = 42;
static const char *dummy_string = "Hello from ESP32!";

/* Global handle for I2C slave */
static i2c_slave_dev_handle_t i2c_slave_handle = NULL;
static QueueHandle_t cmd_queue = NULL;

/* Callback when master finishes sending data to slave */
static bool i2c_slave_recv_done_cb(i2c_slave_dev_handle_t i2c_slave,
                                    const i2c_slave_rx_done_event_data_t *evt_data,
                                    void *arg)
{
    BaseType_t xTaskWoken = pdFALSE;

    if (evt_data->buffer != NULL) {
        uint8_t cmd = evt_data->buffer[0];
        xQueueSendFromISR(cmd_queue, &cmd, &xTaskWoken);
    }

    return xTaskWoken;
}

//error
// static bool i2c_slave_receive_cb(i2c_slave_dev_handle_t i2c_slave, const i2c_slave_rx_done_event_data_t *evt_data, void *arg)
// {
//     i2c_slave_event_t evt = I2C_SLAVE_EVT_RX;
//     BaseType_t xTaskWoken = 0;
//     // You can get data and length via i2c_slave_rx_done_event_data_t
//     xQueueSendFromISR(context->event_queue, &evt, &xTaskWoken);
//     return xTaskWoken;
// }

/* Load data into slave RAM based on command */
static void load_response_data(uint8_t cmd)
{
    uint8_t buffer[I2C_SLAVE_RAM_SIZE] = {0};
    size_t len = 0;

    switch (cmd) {
    case CMD_GET_VALUE_A:
        ESP_LOGI(TAG, "Loading Value A: %d", dummy_value_a);
        memcpy(buffer, &dummy_value_a, sizeof(int));
        len = sizeof(int);
        break;
    case CMD_GET_VALUE_B:
        ESP_LOGI(TAG, "Loading Value B: %d", dummy_value_b);
        memcpy(buffer, &dummy_value_b, sizeof(int));
        len = sizeof(int);
        break;
    case CMD_GET_VALUE_C:
        ESP_LOGI(TAG, "Loading Value C: %d", dummy_value_c);
        memcpy(buffer, &dummy_value_c, sizeof(int));
        len = sizeof(int);
        break;
    case CMD_GET_STRING:
        ESP_LOGI(TAG, "Loading String: %s", dummy_string);
        len = strlen(dummy_string) + 1;
        if (len > I2C_SLAVE_RAM_SIZE) len = I2C_SLAVE_RAM_SIZE;
        memcpy(buffer, dummy_string, len);
        break;
    default:
        ESP_LOGW(TAG, "Unknown command: 0x%02X", cmd);
        len = 4;
        break;
    }

    /* Write data to slave RAM - master will read this on next read operation */
    esp_err_t err = i2c_slave_write_ram(i2c_slave_handle, 0, buffer, len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write to slave RAM: %s", esp_err_to_name(err));
    }
}

/* Task to handle received commands */
static void cmd_handler_task(void *arg)
{
    uint8_t cmd;

    ESP_LOGI(TAG, "Command handler task started");

    while (true) {
        if (xQueueReceive(cmd_queue, &cmd, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "Received command: 0x%02X", cmd);
            load_response_data(cmd);
        }
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "ESP32 I2C Slave - Test Mode");
    ESP_LOGI(TAG, "ESP-IDF Version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "I2C Address: 0x%02X", ESP_SLAVE_ADDR);
    ESP_LOGI(TAG, "SCL Pin: GPIO%d", I2C_SLAVE_SCL_IO);
    ESP_LOGI(TAG, "SDA Pin: GPIO%d", I2C_SLAVE_SDA_IO);
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "Test data:");
    ESP_LOGI(TAG, "  CMD 0x%02X -> Value A: %d", CMD_GET_VALUE_A, dummy_value_a);
    ESP_LOGI(TAG, "  CMD 0x%02X -> Value B: %d", CMD_GET_VALUE_B, dummy_value_b);
    ESP_LOGI(TAG, "  CMD 0x%02X -> Value C: %d", CMD_GET_VALUE_C, dummy_value_c);
    ESP_LOGI(TAG, "  CMD 0x%02X -> String:  %s", CMD_GET_STRING, dummy_string);
    ESP_LOGI(TAG, "=================================");

    /* Create command queue */
    cmd_queue = xQueueCreate(16, sizeof(uint8_t));
    if (cmd_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create command queue");
        return;
    }

    /* Configure I2C slave */
    i2c_slave_config_t i2c_slv_config = {
        .i2c_port = I2C_SLAVE_NUM,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .scl_io_num = I2C_SLAVE_SCL_IO,
        .sda_io_num = I2C_SLAVE_SDA_IO,
        .slave_addr = ESP_SLAVE_ADDR,
        .send_buf_depth = I2C_SLAVE_RAM_SIZE,
    };

    esp_err_t err = i2c_new_slave_device(&i2c_slv_config, &i2c_slave_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2C slave device: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "I2C slave device created");

    /* Register callbacks */
    i2c_slave_event_callbacks_t cbs = {
        .on_recv_done = i2c_slave_recv_done_cb,
    };
    err = i2c_slave_register_event_callbacks(i2c_slave_handle, &cbs, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register callbacks: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "I2C callbacks registered");

    /* Pre-load default data (Value A) */
    load_response_data(CMD_GET_VALUE_A);

    /* Start command handler task */
    xTaskCreate(cmd_handler_task, "cmd_handler", 4096, NULL, 10, NULL);

    ESP_LOGI(TAG, "Ready! Waiting for commands from DaisySeed master...");
    ESP_LOGI(TAG, "Protocol: Master writes 1-byte command, then reads response");
}
