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
#ifndef I2C_H
#define I2C_H


/* Global handle for I2C slave */
extern i2c_slave_dev_handle_t i2c_slave_handle;
extern QueueHandle_t cmd_queue;

/* I2C Configuration - Adjust these pins for your ESP32-C3 wiring */
#define I2C_SLAVE_SCL_IO 21 /* GPIO for I2C clock */
#define I2C_SLAVE_SDA_IO 20 /* GPIO for I2C data */
#define I2C_SLAVE_NUM 0
#define ESP_SLAVE_ADDR 0x28    /* 7-bit I2C address */
#define I2C_SLAVE_RAM_SIZE 128 /* Size of slave RAM buffer */

/* Command bytes the master (DaisySeed) will send */
#define CMD_GET_VALUE_A 0x10
#define CMD_GET_VALUE_B 0x20
#define CMD_GET_VALUE_C 0x30
#define CMD_GET_STRING 0x40

void load_response_data(uint8_t cmd);

bool i2c_slave_recv_done_cb(i2c_slave_dev_handle_t i2c_slave,
                                   const i2c_slave_rx_done_event_data_t *evt_data,
                                   void *arg);

void cmd_handler_task(void *arg);

#endif