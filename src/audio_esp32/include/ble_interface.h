/* * * * BLE Interface * * *
* The interface to use bluetooth with ESP32. 
*  * * * * * * * * * * * * */

#include "i2c.h"

#ifndef BLE_AXIS_INTERFACE
#define BLE_AXIS_INTERFACE

struct ble_hs_adv_fields;
struct ble_gap_conn_desc;
struct ble_hs_cfg;
union ble_store_value;
union ble_store_key;

/* 16 Bit AXIS Service UUID */
#define GATT_AXIS_SVC_UUID                                  0xABF0

/* 16 Bit AXIS Service Characteristic UUID */
#define GATT_AXIS_CHR_UUID                                  0xABF1

extern QueueHandle_t axis_I2C_buffer_queue;

void ble_axis_client_host_task(void *param);
void ble_store_config_init(void);

void ble_axis_client_on_reset(int reason);
void ble_axis_client_on_sync(void);
#endif
