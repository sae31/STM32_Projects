/*
 * Modem_BLE.h
 *
 *  Created on: Jul 9, 2025
 *      Author: SAI KUMAR
 */

#ifndef INC_MODEM_BLE_H_
#define INC_MODEM_BLE_H_
#include "config.h"
#include <stdbool.h>

typedef struct {
    int cid;
    int connID;
    char address[20];
    int conn_state;
    int att_handle;
    uint8_t power;
} BleState;

typedef struct {
    int cid;
    char address[13];
    int value_length;
    char value[50];
} BLEWriteData;

typedef struct
{
	uint8_t change_on_mqtt_username;
	uint8_t change_on_mqtt_password;
	uint8_t change_on_mqtt_clientid;
	uint8_t change_on_mqtt_port;
	uint8_t change_on_lat;
	uint8_t change_on_long;
}flag;
typedef struct
{
	uint16_t svc_uuid;
	uint8_t charaId; // 0, 1, 2.....
	uint16_t svc_property;
	uint8_t uuid_type;  // 16-bit UUID only need to use '1' always
	uint16_t uuid_16;
	/* Configure Characteristic Value */
	uint16_t char_permission;
	uint16_t val_len;
	char char_value[100];

}Gatt_param_t;
void Modem_BLE_Start();
void Modem_BLE_Task(void const * argument);
void modem_ble_init();
int modem_parse_ble_state(const char *response, BleState *Ble_info_t);
void modem_create_ble_gatt_svc_characteristics();
bool modem_parse_ble_write_data(char *input, BLEWriteData *outData);
bool hex_to_ascii(const char *hex_str, char *out);
void asciiToHexStr(const char *asciiStr, char *hexStr);
void modem_ble_update_client_lat_long();
#endif /* INC_MODEM_BLE_H_ */
