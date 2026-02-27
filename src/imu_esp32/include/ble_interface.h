

#ifndef BLEINTERFACE_H
#define BLEINTERFACE_H

#include "esp_log.h"
#include "nvs_flash.h"
/* BLE */
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "console/console.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
// #include "ble_spp_server.h"
#include "driver/uart.h"
#include <stdbool.h>
#include "nimble/ble.h"
#include "modlog/modlog.h"
#include "esp_peripheral.h"



/* 16 Bit SPP Service UUID */
#define BLE_SVC_SPP_UUID16                                  0xABF0

/* 16 Bit SPP Service Characteristic UUID */
#define BLE_SVC_SPP_CHR_UUID16                              0xABF1

struct ble_hs_cfg;
struct ble_gatt_register_ctxt;
int ble_spp_server_gap_event(struct ble_gap_event *event, void *arg);

/* Counts the number of subscriber connections */
extern bool conn_handle_subscriber[CONFIG_BT_NIMBLE_MAX_CONNECTIONS + 1];
extern QueueHandle_t I2C_Queue;
int gatt_svr_register(void);

// Declare the SPP service definitions
extern const struct ble_gatt_svc_def new_ble_svc_gatt_defs[];

// Declare any functions or handles used in the definition
extern uint16_t ble_spp_svc_gatt_read_val_handle;

// typedef struct {
//     (void *)UART_NUM_0;
// } DataToSend;

void ble_store_config_init(void);
void ble_spp_server_advertise(void);
void ble_spp_server_print_conn_desc(struct ble_gap_conn_desc *desc);
void ble_spp_server_on_reset(int reason);
void ble_spp_server_on_sync(void);
void ble_spp_server_host_task(void *param);
int  ble_svc_gatt_handler(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);
int gatt_svr_init(void);
void ble_server_uart_task(void *pvParameters);
void ble_spp_uart_init(void);




// void app_main(void);
#endif