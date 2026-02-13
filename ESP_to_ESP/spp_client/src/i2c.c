#include "i2c.h"

static const char *TAG = "i2c_slave";

/* Dummy test data */
static int dummy_value_a = 12345;
static int dummy_value_b = 67890;
static int dummy_value_c = 42;
static const char *dummy_string = "Hello from ESP32!";

QueueHandle_t cmd_queue = NULL;
i2c_slave_dev_handle_t i2c_slave_handle = NULL;

/* Callback when master finishes sending data to slave */
bool i2c_slave_recv_done_cb(i2c_slave_dev_handle_t i2c_slave,
                                   const i2c_slave_rx_done_event_data_t *evt_data,
                                   void *arg)
{
    BaseType_t xTaskWoken = pdFALSE;

    if (evt_data->buffer != NULL)
    {
        uint8_t cmd = evt_data->buffer[0];
        xQueueSendFromISR(cmd_queue, &cmd, &xTaskWoken);
    }

    return xTaskWoken;
}

/* Load data into slave RAM based on command */
void load_response_data(uint8_t cmd)
{
    uint8_t buffer[I2C_SLAVE_RAM_SIZE] = {0};
    size_t len = 0;

    switch (cmd)
    {
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
        if (len > I2C_SLAVE_RAM_SIZE)
            len = I2C_SLAVE_RAM_SIZE;
        memcpy(buffer, dummy_string, len);
        break;
    default:
        ESP_LOGW(TAG, "Unknown command: 0x%02X", cmd);
        len = 4;
        break;
    }

    /* Write data to slave RAM - master will read this on next read operation */
    esp_err_t err = i2c_slave_write_ram(i2c_slave_handle, 0, buffer, len);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to write to slave RAM: %s", esp_err_to_name(err));
    }
}

/* Task to handle received commands */
void cmd_handler_task(void *arg)
{
    uint8_t cmd;

    ESP_LOGI(TAG, "Command handler task started");

    while (true)
    {
        if (xQueueReceive(cmd_queue, &cmd, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG, "Received command: 0x%02X", cmd);
            load_response_data(cmd);
        }
    }
}