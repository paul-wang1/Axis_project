//#include "ble_interface.h"
/* General Includes */
#include <string.h>
#include "esp_log.h"
#include "driver/i2c_slave.h"

/* I2C */
#include "i2c.h"

/* BLE */
#include "ble_interface.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "modlog/modlog.h"
#include "esp_central.h"

static const char *tag = "NimBLE_AXIS_BLE_CENT";
uint16_t attribute_handle[CONFIG_BT_NIMBLE_MAX_CONNECTIONS + 1];
static ble_addr_t connected_addr[CONFIG_BT_NIMBLE_MAX_CONNECTIONS + 1];


/* Function Declarations */

static int ble_axis_client_gap_event(struct ble_gap_event *event, void *arg);
static void ble_axis_client_scan(void);
static int ble_axis_client_gap_event(struct ble_gap_event *event, void *arg);
static void ble_axis_client_write_subscribe(const struct peer *peer);
static void ble_axis_client_set_handle(const struct peer *peer);
static void ble_axis_client_on_disc_complete(const struct peer *peer, int status, void *arg);
static int ble_axis_client_should_connect(const struct ble_gap_disc_desc *disc);
static void ble_axis_client_connect_if_interesting(const struct ble_gap_disc_desc *disc);


/* Function Definitions */


/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that is
 * established.  ble_axis_client uses the same callback for all connections.
 *
 * @param event                 The event being signalled.
 * @param arg                   Application-specified argument; unused by
 *                                  ble_axis_client.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */
static int ble_axis_client_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc;
    struct ble_hs_adv_fields fields;
    int rc;

    switch (event->type) {
    case BLE_GAP_EVENT_DISC:
        rc = ble_hs_adv_parse_fields(&fields, event->disc.data,
                                     event->disc.length_data);
        if (rc != 0) {
            return 0;
        }

        /* An advertisement report was received during GAP discovery. */
        print_adv_fields(&fields);

        /* Try to connect to the advertiser if it looks interesting. */
        ble_axis_client_connect_if_interesting(&event->disc);
        return 0;

    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed. */
        if (event->connect.status == 0) {
            /* Connection successfully established. */
            MODLOG_DFLT(INFO, "Connection established ");
            rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            assert(rc == 0);
            memcpy(&connected_addr[event->connect.conn_handle].val, desc.peer_id_addr.val,
                   PEER_ADDR_VAL_SIZE);
            print_conn_desc(&desc);
            MODLOG_DFLT(INFO, "\n");

            /* Remember peer. */
            rc = peer_add(event->connect.conn_handle);
            if (rc != 0) {
                MODLOG_DFLT(ERROR, "Failed to add peer; rc=%d\n", rc);
                return 0;
            }

#if MYNEWT_VAL(BLE_GATTC)
            /* Perform service discovery. */
            rc = peer_disc_all(event->connect.conn_handle,
                               ble_axis_client_on_disc_complete, NULL);
            if (rc != 0) {
                MODLOG_DFLT(ERROR, "Failed to discover services; rc=%d\n", rc);
                return 0;
            }
#endif
        } else {
            /* Connection attempt failed; resume scanning. */
            MODLOG_DFLT(ERROR, "Error: Connection failed; status=%d\n",
                        event->connect.status);
            ble_axis_client_scan();
        }

        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        /* Connection terminated. */
        MODLOG_DFLT(INFO, "disconnect; reason=%d ", event->disconnect.reason);
        print_conn_desc(&event->disconnect.conn);
        MODLOG_DFLT(INFO, "\n");

        /* Forget about peer. */
        memset(&connected_addr[event->disconnect.conn.conn_handle].val, 0, PEER_ADDR_VAL_SIZE);
        attribute_handle[event->disconnect.conn.conn_handle] = 0;
        peer_delete(event->disconnect.conn.conn_handle);

        /* Resume scanning. */
        ble_axis_client_scan();
        return 0;

    case BLE_GAP_EVENT_DISC_COMPLETE:
        MODLOG_DFLT(INFO, "discovery complete; reason=%d\n",
                    event->disc_complete.reason);
        return 0;

    case BLE_GAP_EVENT_NOTIFY_RX:
        /* Peer sent us a notification. In our case that means that 
		 * we can send this packet to the */
        i2c_slave_transmit(i2c_slave_handle, (OS_MBUF_DATA(event->notify_rx.om, uint8_t*)), 14, 100);
        return 0;

    case BLE_GAP_EVENT_MTU:
        MODLOG_DFLT(INFO, "mtu update event; conn_handle=%d cid=%d mtu=%d\n",
                    event->mtu.conn_handle,
                    event->mtu.channel_id,
                    event->mtu.value);
        return 0;

    default:
        return 0;
    }
}

/**
 * Subscribe to notifications for the AXIS characteristic.
 * A central enables notifications by writing two bytes (1, 0) to the
 * characteristic's client-characteristic-configuration-descriptor (CCCD).
 *
 * @param peer                  The peer information for the device that is being
 *                              subscribed to
 * @return                      None
 * @exception                   Disconnects from bluetooth if the characteristic
 *                              doesn't have a CCCD for unread alerts
 */
static void ble_axis_client_write_subscribe(const struct peer *peer)
{
  uint8_t value[2];
  int rc;
  const struct peer_dsc *dsc;

  dsc = peer_dsc_find_uuid(peer,
                    BLE_UUID16_DECLARE(GATT_AXIS_SVC_UUID),
                    BLE_UUID16_DECLARE(GATT_AXIS_CHR_UUID),
                    BLE_UUID16_DECLARE(BLE_GATT_DSC_CLT_CFG_UUID16));

  if (dsc == NULL) {
     MODLOG_DFLT(ERROR, "Error: Peer lacks a CCCD for the Unread Alert "
                       "Status characteristic\n");
    goto err;
  }

  value[0] = 1;
  value[1] = 0;
  rc = ble_gattc_write_flat(peer->conn_handle, dsc->dsc.handle,
                          value, sizeof(value), NULL, NULL);

  if (rc != 0) {
     MODLOG_DFLT(ERROR, "Error: Failed to subscribe to characteristic; "
                       "rc=%d\n", rc);
    goto err;
  }
  return;
  err:
    /* Terminate the connection if there is an error */
    ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);

}

/**
 * Sends out a packet with the handle info
 *
 * @param peer                  The peer information for the device that is being
 *                              subscribed to
 * @return                      None
 */
static void ble_axis_client_set_handle(const struct peer *peer)
{
    const struct peer_chr *chr;
    const struct peer_dsc *dsc;
    uint8_t value[2];
    chr = peer_chr_find_uuid(peer,
                             BLE_UUID16_DECLARE(GATT_AXIS_SVC_UUID),
                             BLE_UUID16_DECLARE(GATT_AXIS_CHR_UUID));
    attribute_handle[peer->conn_handle] = chr->chr.val_handle;
    MODLOG_DFLT(INFO, "attribute_handle %x\n", attribute_handle[peer->conn_handle]);

    dsc = peer_dsc_find_uuid(peer,
                             BLE_UUID16_DECLARE(GATT_AXIS_SVC_UUID),
                             BLE_UUID16_DECLARE(GATT_AXIS_CHR_UUID),
                             BLE_UUID16_DECLARE(BLE_GATT_DSC_CLT_CFG_UUID16));
    if (dsc == NULL) {
        MODLOG_DFLT(ERROR, "Error: Peer lacks a CCCD for the subscribable characteristic\n");
	    return;
    }

    value[0] = 1;
    value[1] = 0;
    ble_gattc_write_flat(peer->conn_handle, dsc->dsc.handle,
                            value, sizeof(value), NULL, NULL);
}

/**
 * Called when service discovery of the specified peer has completed. 
 * Writes back to set its handle and subscribe to the Axis service
 *
 * @param peer                  The peer information for the device that is being
 *                              subscribed to
 * @param status                Status of the discovery that was completeted
 * @return                      None
 */
static void ble_axis_client_on_disc_complete(const struct peer *peer, int status, void *arg)
{
    if (status != 0) {
        /* Service discovery failed.  Terminate the connection. */
        MODLOG_DFLT(ERROR, "Error: Service discovery failed; status=%d "
                    "conn_handle=%d\n", status, peer->conn_handle);
        ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        return;
    }

    /* Service discovery has completed successfully.  Now we have a complete
     * list of services, characteristics, and descriptors that the peer
     * supports.
     */
    MODLOG_DFLT(INFO, "Service discovery complete; status=%d "
                "conn_handle=%d\n", status, peer->conn_handle);

    ble_axis_client_set_handle(peer);
    ble_axis_client_write_subscribe(peer);

#if CONFIG_BT_NIMBLE_MAX_CONNECTIONS > 1
    ble_axis_client_scan();
#endif
}

/**
 * Initiates the GAP general discovery procedure.
 */
static void ble_axis_client_scan(void)
{
    uint8_t own_addr_type;
    struct ble_gap_disc_params disc_params = {0};
    int rc;

    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
        return;
    }

    /* Tell the controller to filter duplicates; we don't want to process
     * repeated advertisements from the same device.
     */
    disc_params.filter_duplicates = 1;

    /**
     * Perform a passive scan.  I.e., don't send follow-up scan requests to
     * each advertiser.
     */
    disc_params.passive = 1;

    /* Use defaults for the rest of the parameters. */
    disc_params.itvl = 0;
    disc_params.window = 0;
    disc_params.filter_policy = 0;
    disc_params.limited = 0;

    rc = ble_gap_disc(own_addr_type, BLE_HS_FOREVER, &disc_params,
                      ble_axis_client_gap_event, NULL);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "Error initiating GAP discovery procedure; rc=%d\n",
                    rc);
    }
}

/**
 * Indicates whether we should try to connect to the sender of the specified
 * advertisement.  The function returns a positive result if the device
 * advertises connectability and support for the Alert Notification service.
 */
static int ble_axis_client_should_connect(const struct ble_gap_disc_desc *disc)
{
    struct ble_hs_adv_fields fields;
    int rc;
    int i;

    /* Check if device is already connected or not */
    for ( i = 0; i <= CONFIG_BT_NIMBLE_MAX_CONNECTIONS; i++) {
        if (memcmp(&connected_addr[i].val, disc->addr.val, PEER_ADDR_VAL_SIZE) == 0) {
            MODLOG_DFLT(DEBUG, "Device already connected");
            return 0;
        }
    }

    /* The device has to be advertising connectability. */
    if (disc->event_type != BLE_HCI_ADV_RPT_EVTYPE_ADV_IND &&
            disc->event_type != BLE_HCI_ADV_RPT_EVTYPE_DIR_IND) {

        return 0;
    }

    rc = ble_hs_adv_parse_fields(&fields, disc->data, disc->length_data);
    if (rc != 0) {
        return 0;
    }

    /* The device has to advertise support for the AXIS
     * service (0xABF0).
     */
    for (i = 0; i < fields.num_uuids16; i++) {
        if (ble_uuid_u16(&fields.uuids16[i].u) == GATT_AXIS_SVC_UUID) {
            return 1;
        }
    }
    return 0;
}

/**
 * Connects to the sender of the specified advertisement of it looks
 * interesting.  A device is "interesting" if it advertises connectability and
 * support for the Alert Notification service.
 */
static void ble_axis_client_connect_if_interesting(const struct ble_gap_disc_desc *disc)
{
    uint8_t own_addr_type;
    int rc;

    /* Don't do anything if we don't care about this advertiser. */
    if (!ble_axis_client_should_connect(disc)) {
        return;
    }

#if !(MYNEWT_VAL(BLE_HOST_ALLOW_CONNECT_WITH_SCAN))
    /* Scanning must be stopped before a connection can be initiated. */
    rc = ble_gap_disc_cancel();
    if (rc != 0) {
        MODLOG_DFLT(DEBUG, "Failed to cancel scan; rc=%d\n", rc);
        return;
    }
#endif

    /* Figure out address to use for connect (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
        return;
    }

    /* Try to connect the the advertiser.  Allow 30 seconds (30000 ms) for
     * timeout.
     */

    rc = ble_gap_connect(own_addr_type, &disc->addr, 30000, NULL,
                         ble_axis_client_gap_event, NULL);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "Error: Failed to connect to device; addr_type=%d "
                    "addr=%s; rc=%d\n",
                    disc->addr.type, addr_str(disc->addr.val), rc);
        return;
    }
}

void ble_axis_client_on_reset(int reason)
{
    MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
}

void ble_axis_client_on_sync(void)
{
    int rc;

    /* Make sure we have proper identity address set (public preferred) */
    rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);

    /* Begin scanning for a peripheral to connect to. */
    ble_axis_client_scan();
}

/**
 * Task that begins the BLE stack and deinitializes when finished
 */
void ble_axis_client_host_task(void *param)
{
    ESP_LOGI(tag, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();

    nimble_port_freertos_deinit();
}
