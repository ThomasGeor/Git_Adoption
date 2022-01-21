/* BLE Driver for ESP32 burglar alarm master-server
 *
 *  author : Thomas Georgiadis
 *
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include "freertos/FreeRTOS.h"
 #include "freertos/task.h"
 #include "freertos/event_groups.h"
 #include "esp_system.h"
 #include "esp_log.h"
 #include "nvs_flash.h"
 #include "esp_bt.h"

 #include "esp_gap_ble_api.h"
 #include "esp_gatts_api.h"
 #include "esp_bt_defs.h"
 #include "esp_bt_main.h"
 #include "esp_gatt_common_api.h"
 #include "gpio_driver.h"

 #include "sdkconfig.h"

 /* Defintions */
 #define GATTS_TAG                 "GATT_SERVER"
 #define GATTS_ALARM_SERVICE_UUID   0x0012
 #define GATTS_ALARM_CHAR_UUID      0x0001
 #define GATTS_ALARM_DESCR_UUID     0x3333
 #define GATTS_ALARM_NUM_HANDLE     4

 #define MASTER_DEVICE_NAME         "DOOR_DEVICE"
 #define TEST_MANUFACTURER_DATA_LEN 17

 #define GATTS_ALARM_CHAR_VAL_LEN_MAX 0x40

 #define PREPARE_BUF_MAX_SIZE 1024

/* Struct declarations */
struct gattc_profile_inst
{
  esp_gatts_cb_t  gattc_cb;
  uint16_t        gattc_if;
  uint16_t        app_id;
  uint16_t        conn_id;
  uint16_t        service_start_handle;
  uint16_t        service_end_handle;
  uint16_t        char_handle;
  esp_bd_addr_t   remote_bda;
};

/* Function declarations */
void ble_client_init();
