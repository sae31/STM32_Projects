/*
 * Modem_BLE.c
 *
 *  Created on: Jul 9, 2025
 *      Author: SAI KUMAR
 */
/*****************************************  Header Files ***********************************************/
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include "ATCommands.h"
#include "stm32g0xx_hal.h"
#include "cmsis_os.h"
#include "Modem_RxProcess.h"
#include "EC200U.h"
#include "Modem_BLE.h"

/***************************************** Macros ********************************************/
#define SVC_UUID_ABF0   0
#define UUID16_CAN_DATA 65268  // 0xFEF4

/* ================== BLE GATT Characteristics ================================== */
#define UU1D16_MQTT_USERNAME     45329  // 0xB111
#define UUID16_MQTT_PASSWORD     45330  // 0XB112
#define UUID16_MQTT_CLIENT_ID    45331  // 0XB113
#define UUID16_MQTT_PORT         45332  // 0XB114
#define UUID16_SET_PRESS         45333  // 0XB115
#define UUID16_GPS_LAT_LONG      45334  // 0XB116

/***************************************** Private Enumeration ********************************************/
typedef enum Ble_char_id
{
	__MQTT_USERNAME_CHAR_ID ,
	__MQTT_PASSWORD_CHAR_ID,
	__MQTT_CLIENT_CHAR_ID,
	__MQTT_PORT_CHAR_ID ,
	__SET_PRESS_CHAR_ID,
	__GPS_LAT_LONG_ID,
}BLE_chara_id;

/****************************** External Variables **************************************/
extern osThreadId ModemBLE_TaskHandle;
extern UART_HandleTypeDef huart1;
extern BLEWriteData BLE_Write_data;
extern mqtt_conf_t modem_mqtt_conf_t;
extern osSemaphoreId Modem_port_block_semaphore;
extern char Lat_string[10],Long_string[10];

/***************************************** Private Variables ********************************************/

BleState Ble_info_t={0};
flag mqtt_flag ={0};
Gatt_param_t Modem_GATT_Param[MAX_CHAR_IN_MQTT_SVC];
char Ble_write_data_ascii[50];
char hexOutput[256];
uint8_t client_write=0,clear_buff=0,mqtt_reinit=0,set_press=0;

/***************************************** Function Prototypes ******************************************/

void Modem_BLE_Start()
{
	osThreadDef(ModemBLETask, Modem_BLE_Task, osPriorityNormal, 0, 256);
	ModemBLE_TaskHandle = osThreadCreate(osThread(ModemBLETask), NULL);
}

void Modem_BLE_Task(void const * argument)
{

	HAL_UARTEx_ReceiveToIdle_IT(&huart1,(uint8_t*)EC200u_Rx_Buff, sizeof(EC200u_Rx_Buff));
	modem_ble_init();

	while(1)
	{
//		if(Ble_info_t.conn_state==1)
		{
			if(client_write)
			{

				switch(BLE_Write_data.cid)
				{
					case __MQTT_USERNAME_CHAR_ID:
					{
						memset(modem_mqtt_conf_t.mqtt_username,0,sizeof(modem_mqtt_conf_t.mqtt_username));
						strcpy(modem_mqtt_conf_t.mqtt_username,Ble_write_data_ascii);
						memset(Ble_write_data_ascii,0,sizeof(Ble_write_data_ascii));
						//memset(EC200u_Rx_Buff,0,sizeof(EC200u_Rx_Buff));
						mqtt_flag.change_on_mqtt_username=1;  //TODO :Need to compare previous data and set the flag
						break;
					}
					case __MQTT_PASSWORD_CHAR_ID:
					{
						memset(modem_mqtt_conf_t.mqtt_password,0,sizeof(modem_mqtt_conf_t.mqtt_password));
						strcpy(modem_mqtt_conf_t.mqtt_password,Ble_write_data_ascii);
						memset(Ble_write_data_ascii,0,sizeof(Ble_write_data_ascii));
						//memset(EC200u_Rx_Buff,0,sizeof(EC200u_Rx_Buff));
						mqtt_flag.change_on_mqtt_password=1;  //TODO :Need to compare previous data and set the flag
						break;
					}
					case __MQTT_CLIENT_CHAR_ID:
					{
						memset(modem_mqtt_conf_t.mqtt_client_id,0,sizeof(modem_mqtt_conf_t.mqtt_client_id));
						strcpy(modem_mqtt_conf_t.mqtt_client_id,Ble_write_data_ascii);
						memset(Ble_write_data_ascii,0,sizeof(Ble_write_data_ascii));
						//memset(EC200u_Rx_Buff,0,sizeof(EC200u_Rx_Buff));
						mqtt_flag.change_on_mqtt_clientid=1;   //TODO :Need to compare previous data and set the flag
						break;
					}
					case __MQTT_PORT_CHAR_ID:
					{

						modem_mqtt_conf_t.mqtt_port=atoi(Ble_write_data_ascii);
						memset(Ble_write_data_ascii,0,sizeof(Ble_write_data_ascii));
						//memset(EC200u_Rx_Buff,0,sizeof(EC200u_Rx_Buff));
						mqtt_flag.change_on_mqtt_port=1;   //TODO :Need to compare previous data and set the flag
						break;
					}
					case __SET_PRESS_CHAR_ID:
					{
						set_press=atoi(Ble_write_data_ascii);
						switch(set_press)
						{
							case 1:
							{
								mqtt_reinit=1;
								break;
							}
							case 2:
							{
								HAL_NVIC_SystemReset();
								break;
							}
							default:
							{
								break;
							}
						}

						memset(Ble_write_data_ascii,0,sizeof(Ble_write_data_ascii));
//						memset(EC200u_Rx_Buff,0,sizeof(EC200u_Rx_Buff));
						break;
					}
					default:
					{
						break;
					}
				}
				client_write=0;
			}
		}

		if(mqtt_flag.change_on_lat | mqtt_flag.change_on_long)
		{

			mqtt_flag.change_on_lat=0;
			mqtt_flag.change_on_long=0;
			if(xSemaphoreTake(Modem_port_block_semaphore,3000)!=pdFALSE)
			{
				modem_ble_update_client_lat_long();
				xSemaphoreGive(Modem_port_block_semaphore);
			}
		}
		if(clear_buff)
		{
			osDelay(100);
			memset(EC200u_Rx_Buff,0,sizeof(EC200u_Rx_Buff));
			clear_buff=0;
		}
		osDelay(10);
	}
}
void modem_ble_init()
{



	/*
	modem_initiate_cmd(MODEM_BLE_SET_NAME);
	vTaskDelay(2000 / portTICK_PERIOD_MS);
	*/

	modem_initiate_cmd(MODEM_TURN_ON_BLE);
	osDelay(2000);


	modem_initiate_cmd(MODEM_BLE_SET_ADV_PARAM);
	osDelay(1000);
	/*
	modem_initiate_cmd(MODEM_BLE_SET_ADV_DATA);
	osDelay(2000);

	modem_initiate_cmd(MODEM_BLE_SET_SCAN_RESP_DATA);
	osDelay(2000);
	*/
	modem_initiate_cmd(MODEM_BLE_SET_ADV_NAME);
	osDelay(1000);

	modem_initiate_cmd(MODEM_BLE_SET_PRIMARY_SVC);
	osDelay(2000);

	/*
	modem_initiate_cmd(MODEM_BLE_ADD_SVC_CHAR);
	osDelay(2000);

	modem_initiate_cmd(MODEM_BLE_CFG_CHAR_VALUE);
	osDelay(2000);
	*/
	modem_create_ble_gatt_svc_characteristics();

	modem_initiate_cmd(MODEM_BLE_FINSISH_ADDING_SVC);
	osDelay(2000);

	modem_initiate_cmd(MODEM_BLE_START_ADV);
	osDelay(2000);

	/*
	modem_initiate_cmd(MODEM_BLE_GET_NAME);
	vTaskDelay(2000 / portTICK_PERIOD_MS);
	*/




}
int modem_parse_ble_state(const char *response, BleState *Ble_info_t)
{
    if (response == NULL || Ble_info_t == NULL) {
        return -1; // Error: Null pointer
    }

    // Example: +QBTLESTATE: 0,0,"5d13f0ec567c",1,18
    const char *prefix = "+QBTLESTATE:";
    if (strncmp(response, prefix, strlen(prefix)) != 0) {
        return -2; // Error: Invalid prefix
    }

    // Parse the fields
    int ret = sscanf(response + strlen(prefix), " %d,%d,\"%19[^\"]\",%d,%d",
                     &Ble_info_t->cid,
                     &Ble_info_t->connID,
                     Ble_info_t->address,
                     &Ble_info_t->conn_state,
                     &Ble_info_t->att_handle);

    if (ret != 5) {
        return -3; // Error: Failed to parse all fields
    }

    return 0; // Success
}
void modem_create_ble_gatt_svc_characteristics()
{
	char cmd[200];

	#ifdef USE_BLE_TELEMETRY_ID

	/* GATT CHAR 0xFEF4 */
	Modem_GATT_Param[__BLE_TELEMETRY_CHAR_ID].svc_uuid=SVC_UUID_ABF0;
	Modem_GATT_Param[__BLE_TELEMETRY_CHAR_ID].charaId =__BLE_TELEMETRY_CHAR_ID;
	Modem_GATT_Param[__BLE_TELEMETRY_CHAR_ID].svc_property= 18;  //Read and Notify
	Modem_GATT_Param[__BLE_TELEMETRY_CHAR_ID].uuid_type=1;     //16-bit UUID
	Modem_GATT_Param[__BLE_TELEMETRY_CHAR_ID].uuid_16 =UUID16_CAN_DATA;
	Modem_GATT_Param[__BLE_TELEMETRY_CHAR_ID].val_len=42;
	strcpy(Modem_GATT_Param[__BLE_TELEMETRY_CHAR_ID].char_value,"48656C6C6F"); // Send Hello
	Modem_GATT_Param[__BLE_TELEMETRY_CHAR_ID].char_permission=1;  // Read Only

	sprintf(cmd,"AT+QBTGATSC=%d,%d,%d,%d,%d",Modem_GATT_Param[__BLE_TELEMETRY_CHAR_ID].svc_uuid,Modem_GATT_Param[__BLE_TELEMETRY_CHAR_ID].charaId,
			Modem_GATT_Param[__BLE_TELEMETRY_CHAR_ID].svc_property,Modem_GATT_Param[__BLE_TELEMETRY_CHAR_ID].uuid_type,Modem_GATT_Param[__BLE_TELEMETRY_CHAR_ID].uuid_16);
	modem_send_msg(cmd);
	osDelay(1000);

	sprintf(cmd,"AT+QBTGATSCV=%d,%d,%d,%d,%d,%d,\"%s\"",Modem_GATT_Param[__BLE_TELEMETRY_CHAR_ID].svc_uuid,Modem_GATT_Param[__BLE_TELEMETRY_CHAR_ID].charaId,Modem_GATT_Param[__BLE_TELEMETRY_CHAR_ID].char_permission,
			Modem_GATT_Param[__BLE_TELEMETRY_CHAR_ID].uuid_type,Modem_GATT_Param[__BLE_TELEMETRY_CHAR_ID].uuid_16,Modem_GATT_Param[__BLE_TELEMETRY_CHAR_ID].val_len,Modem_GATT_Param[__BLE_TELEMETRY_CHAR_ID].char_value);

	modem_send_msg(cmd);
	osDelay(1000);

	#endif
	/* GATT CHAR 0xB111 Configure MQTT Username */
	Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].svc_uuid=SVC_UUID_ABF0;
	Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].charaId =__MQTT_USERNAME_CHAR_ID;
	Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].svc_property= 4;  // 4 -> Write  // 26-> Read ,write, notify
	Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].uuid_type=1;     //16-bit UUID
	Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].uuid_16 =UU1D16_MQTT_USERNAME;
	Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].val_len=20;
//	asciiToHexStr(modem_mqtt_conf_t.mqtt_username, hexOutput);
	strcpy(Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].char_value,"48656C6C6F"); // Send NULL
//	strcpy(Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].char_value,modem_mqtt_conf_t.mqtt_username); // Send Mqtt Username
	Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].char_permission=2;  // 2-> write Only // 3-> Read,write

	sprintf(cmd,"AT+QBTGATSC=%d,%d,%d,%d,%d",Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].svc_uuid,Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].charaId,
			Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].svc_property,Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].uuid_type,Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].uuid_16);
	modem_send_msg(cmd);
	osDelay(1000);

	sprintf(cmd,"AT+QBTGATSCV=%d,%d,%d,%d,%d,%d,\"%s\"",Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].svc_uuid,Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].charaId,Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].char_permission,
			Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].uuid_type,Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].uuid_16,Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].val_len,Modem_GATT_Param[__MQTT_USERNAME_CHAR_ID].char_value);

	modem_send_msg(cmd);
	osDelay(1000);

	/* GATT CHAR 0xB112--> Configure MQTT Password */
	Modem_GATT_Param[__MQTT_PASSWORD_CHAR_ID].svc_uuid=SVC_UUID_ABF0;
	Modem_GATT_Param[__MQTT_PASSWORD_CHAR_ID].charaId =__MQTT_PASSWORD_CHAR_ID;
	Modem_GATT_Param[__MQTT_PASSWORD_CHAR_ID].svc_property= 4;  //Write
	Modem_GATT_Param[__MQTT_PASSWORD_CHAR_ID].uuid_type=1;     //16-bit UUID
	Modem_GATT_Param[__MQTT_PASSWORD_CHAR_ID].uuid_16 =UUID16_MQTT_PASSWORD;
	Modem_GATT_Param[__MQTT_PASSWORD_CHAR_ID].val_len=20;
	strcpy(Modem_GATT_Param[__MQTT_PASSWORD_CHAR_ID].char_value,"48656C6C6F"); // Send NULL
	Modem_GATT_Param[__MQTT_PASSWORD_CHAR_ID].char_permission=2;  // write Only

	sprintf(cmd,"AT+QBTGATSC=%d,%d,%d,%d,%d",Modem_GATT_Param[__MQTT_PASSWORD_CHAR_ID].svc_uuid,Modem_GATT_Param[__MQTT_PASSWORD_CHAR_ID].charaId,
			Modem_GATT_Param[__MQTT_PASSWORD_CHAR_ID].svc_property,Modem_GATT_Param[__MQTT_PASSWORD_CHAR_ID].uuid_type,Modem_GATT_Param[__MQTT_PASSWORD_CHAR_ID].uuid_16);
	modem_send_msg(cmd);
	osDelay(1000);

	sprintf(cmd,"AT+QBTGATSCV=%d,%d,%d,%d,%d,%d,\"%s\"",Modem_GATT_Param[__MQTT_PASSWORD_CHAR_ID].svc_uuid,Modem_GATT_Param[__MQTT_PASSWORD_CHAR_ID].charaId,Modem_GATT_Param[__MQTT_PASSWORD_CHAR_ID].char_permission,
			Modem_GATT_Param[__MQTT_PASSWORD_CHAR_ID].uuid_type,Modem_GATT_Param[__MQTT_PASSWORD_CHAR_ID].uuid_16,Modem_GATT_Param[__MQTT_PASSWORD_CHAR_ID].val_len,Modem_GATT_Param[__MQTT_PASSWORD_CHAR_ID].char_value);

	modem_send_msg(cmd);
	osDelay(1000);

	/* GATT CHAR 0xB113--> Configure MQTT ClientID */
	Modem_GATT_Param[__MQTT_CLIENT_CHAR_ID].svc_uuid=SVC_UUID_ABF0;
	Modem_GATT_Param[__MQTT_CLIENT_CHAR_ID].charaId =__MQTT_CLIENT_CHAR_ID;
	Modem_GATT_Param[__MQTT_CLIENT_CHAR_ID].svc_property= 4;  //Write
	Modem_GATT_Param[__MQTT_CLIENT_CHAR_ID].uuid_type=1;     //16-bit UUID
	Modem_GATT_Param[__MQTT_CLIENT_CHAR_ID].uuid_16 =UUID16_MQTT_CLIENT_ID;
	Modem_GATT_Param[__MQTT_CLIENT_CHAR_ID].val_len=20;
	strcpy(Modem_GATT_Param[__MQTT_CLIENT_CHAR_ID].char_value,"48656C6C6F"); // Send NULL
	Modem_GATT_Param[__MQTT_CLIENT_CHAR_ID].char_permission=2;  // write Only

	sprintf(cmd,"AT+QBTGATSC=%d,%d,%d,%d,%d",Modem_GATT_Param[__MQTT_CLIENT_CHAR_ID].svc_uuid,Modem_GATT_Param[__MQTT_CLIENT_CHAR_ID].charaId,
			Modem_GATT_Param[__MQTT_CLIENT_CHAR_ID].svc_property,Modem_GATT_Param[__MQTT_CLIENT_CHAR_ID].uuid_type,Modem_GATT_Param[__MQTT_CLIENT_CHAR_ID].uuid_16);
	modem_send_msg(cmd);
	osDelay(1000);

	sprintf(cmd,"AT+QBTGATSCV=%d,%d,%d,%d,%d,%d,\"%s\"",Modem_GATT_Param[__MQTT_CLIENT_CHAR_ID].svc_uuid,Modem_GATT_Param[__MQTT_CLIENT_CHAR_ID].charaId,Modem_GATT_Param[__MQTT_CLIENT_CHAR_ID].char_permission,
			Modem_GATT_Param[__MQTT_CLIENT_CHAR_ID].uuid_type,Modem_GATT_Param[__MQTT_CLIENT_CHAR_ID].uuid_16,Modem_GATT_Param[__MQTT_CLIENT_CHAR_ID].val_len,Modem_GATT_Param[__MQTT_CLIENT_CHAR_ID].char_value);

	modem_send_msg(cmd);
	osDelay(1000);

	/* GATT CHAR 0xB114--> Configure MQTT Port 1883 0r 8883 */
	Modem_GATT_Param[__MQTT_PORT_CHAR_ID].svc_uuid=SVC_UUID_ABF0;
	Modem_GATT_Param[__MQTT_PORT_CHAR_ID].charaId =__MQTT_PORT_CHAR_ID;
	Modem_GATT_Param[__MQTT_PORT_CHAR_ID].svc_property= 4;  //Write
	Modem_GATT_Param[__MQTT_PORT_CHAR_ID].uuid_type=1;     //16-bit UUID
	Modem_GATT_Param[__MQTT_PORT_CHAR_ID].uuid_16 =UUID16_MQTT_PORT;
	Modem_GATT_Param[__MQTT_PORT_CHAR_ID].val_len=10;
	strcpy(Modem_GATT_Param[__MQTT_PORT_CHAR_ID].char_value,"48656C6C6F"); // Send NULL
	Modem_GATT_Param[__MQTT_PORT_CHAR_ID].char_permission=2;  // write Only

	sprintf(cmd,"AT+QBTGATSC=%d,%d,%d,%d,%d",Modem_GATT_Param[__MQTT_PORT_CHAR_ID].svc_uuid,Modem_GATT_Param[__MQTT_PORT_CHAR_ID].charaId,
			Modem_GATT_Param[__MQTT_PORT_CHAR_ID].svc_property,Modem_GATT_Param[__MQTT_PORT_CHAR_ID].uuid_type,Modem_GATT_Param[__MQTT_PORT_CHAR_ID].uuid_16);
	modem_send_msg(cmd);
	osDelay(1000);

	sprintf(cmd,"AT+QBTGATSCV=%d,%d,%d,%d,%d,%d,\"%s\"",Modem_GATT_Param[__MQTT_PORT_CHAR_ID].svc_uuid,Modem_GATT_Param[__MQTT_PORT_CHAR_ID].charaId,Modem_GATT_Param[__MQTT_PORT_CHAR_ID].char_permission,
			Modem_GATT_Param[__MQTT_PORT_CHAR_ID].uuid_type,Modem_GATT_Param[__MQTT_PORT_CHAR_ID].uuid_16,Modem_GATT_Param[__MQTT_PORT_CHAR_ID].val_len,Modem_GATT_Param[__MQTT_PORT_CHAR_ID].char_value);

	modem_send_msg(cmd);
	osDelay(1000);

	/* GATT CHAR 0xB115--> Set Press operation  */
	Modem_GATT_Param[__SET_PRESS_CHAR_ID].svc_uuid=SVC_UUID_ABF0;
	Modem_GATT_Param[__SET_PRESS_CHAR_ID].charaId =__SET_PRESS_CHAR_ID;
	Modem_GATT_Param[__SET_PRESS_CHAR_ID].svc_property= 4;  //Write
	Modem_GATT_Param[__SET_PRESS_CHAR_ID].uuid_type=1;     //16-bit UUID
	Modem_GATT_Param[__SET_PRESS_CHAR_ID].uuid_16 =UUID16_SET_PRESS;
	Modem_GATT_Param[__SET_PRESS_CHAR_ID].val_len=5;
	strcpy(Modem_GATT_Param[__SET_PRESS_CHAR_ID].char_value,"48656C6F"); // Send NULL
	Modem_GATT_Param[__SET_PRESS_CHAR_ID].char_permission=2;  // write Only

	sprintf(cmd,"AT+QBTGATSC=%d,%d,%d,%d,%d",Modem_GATT_Param[__SET_PRESS_CHAR_ID].svc_uuid,Modem_GATT_Param[__SET_PRESS_CHAR_ID].charaId,
			Modem_GATT_Param[__SET_PRESS_CHAR_ID].svc_property,Modem_GATT_Param[__SET_PRESS_CHAR_ID].uuid_type,Modem_GATT_Param[__SET_PRESS_CHAR_ID].uuid_16);
	modem_send_msg(cmd);
	osDelay(1000);

	sprintf(cmd,"AT+QBTGATSCV=%d,%d,%d,%d,%d,%d,\"%s\"",Modem_GATT_Param[__SET_PRESS_CHAR_ID].svc_uuid,Modem_GATT_Param[__SET_PRESS_CHAR_ID].charaId,Modem_GATT_Param[__SET_PRESS_CHAR_ID].char_permission,
			Modem_GATT_Param[__SET_PRESS_CHAR_ID].uuid_type,Modem_GATT_Param[__SET_PRESS_CHAR_ID].uuid_16,Modem_GATT_Param[__SET_PRESS_CHAR_ID].val_len,Modem_GATT_Param[__SET_PRESS_CHAR_ID].char_value);

	modem_send_msg(cmd);
	osDelay(1000);


	/* GATT CHAR 0xB116--> GPS Latitude and Longitudes */
	Modem_GATT_Param[__GPS_LAT_LONG_ID].svc_uuid=SVC_UUID_ABF0;
	Modem_GATT_Param[__GPS_LAT_LONG_ID].charaId =__GPS_LAT_LONG_ID;
	Modem_GATT_Param[__GPS_LAT_LONG_ID].svc_property= 18;  // Read and Notify
	Modem_GATT_Param[__GPS_LAT_LONG_ID].uuid_type=1;     //16-bit UUID
	Modem_GATT_Param[__GPS_LAT_LONG_ID].uuid_16 =UUID16_GPS_LAT_LONG;
	Modem_GATT_Param[__GPS_LAT_LONG_ID].val_len=40;
//	strcpy(Modem_GATT_Param[__GPS_LAT_LONG_ID].char_value,"48656C6F"); // Send Latitude and Longitudes
	strcpy(Modem_GATT_Param[__GPS_LAT_LONG_ID].char_value,Lat_string); // Send Latitude and Longitudes
	Modem_GATT_Param[__GPS_LAT_LONG_ID].char_permission=1;  // Read Only

	sprintf(cmd,"AT+QBTGATSC=%d,%d,%d,%d,%d",Modem_GATT_Param[__GPS_LAT_LONG_ID].svc_uuid,Modem_GATT_Param[__GPS_LAT_LONG_ID].charaId,
			Modem_GATT_Param[__GPS_LAT_LONG_ID].svc_property,Modem_GATT_Param[__GPS_LAT_LONG_ID].uuid_type,Modem_GATT_Param[__GPS_LAT_LONG_ID].uuid_16);
	modem_send_msg(cmd);
	osDelay(1000);

	sprintf(cmd,"AT+QBTGATSCV=%d,%d,%d,%d,%d,%d,\"%s\"",Modem_GATT_Param[__GPS_LAT_LONG_ID].svc_uuid,Modem_GATT_Param[__GPS_LAT_LONG_ID].charaId,Modem_GATT_Param[__GPS_LAT_LONG_ID].char_permission,
			Modem_GATT_Param[__GPS_LAT_LONG_ID].uuid_type,Modem_GATT_Param[__GPS_LAT_LONG_ID].uuid_16,Modem_GATT_Param[__GPS_LAT_LONG_ID].val_len,Modem_GATT_Param[__GPS_LAT_LONG_ID].char_value);

	modem_send_msg(cmd);
	osDelay(1000);


}
bool modem_parse_ble_write_data(char *input, BLEWriteData *outData)
{

	client_write=1;
    const char *marker = "+QBTLEVALDATA:";
    char *start = strstr(input, marker);
    if (!start) return false;

    start += strlen(marker); // move past "+QBTLEVALDATA:"

    // Trim leading whitespace
    while (*start == ' ' || *start == '\r' || *start == '\n') start++;

    // Step-by-step manual parsing
    // Format: <cid>,"<address>",<value_length>,"<value>"

    // 1. Get <cid>
    char *token = strtok(start, ",");
    if (!token) return false;
    outData->cid = atoi(token);

    // 2. Get "<address>"
    token = strtok(NULL, ",");
    if (!token || token[0] != '\"') return false;
    token++;  // skip opening quote
    char *quote = strchr(token, '\"');
    if (!quote) return false;
    *quote = '\0';
    strncpy(outData->address, token, sizeof(outData->address));

    // 3. Get <value_length>
    token = strtok(NULL, ",");
    if (!token) return false;
    outData->value_length = atoi(token);

    // 4. Get "<value>"
    token = strtok(NULL, "\r\n"); // ends at CR or LF
    if (!token || token[0] != '\"') return false;
    token++; // skip opening quote
    quote = strchr(token, '\"');
    if (!quote) return false;
    *quote = '\0';
    strncpy(outData->value, token, sizeof(outData->value));
    hex_to_ascii(BLE_Write_data.value, Ble_write_data_ascii);
    memset(EC200u_Rx_Buff,0,sizeof(EC200u_Rx_Buff));
    return true;
}
bool hex_to_ascii(const char *hex_str, char *out)
{
    size_t len = strlen(hex_str);
    if (len % 2 != 0) return false;  // Must be even length

    for (size_t i = 0; i < len; i += 2)
    {
        if (!isxdigit((unsigned char)hex_str[i]) || !isxdigit((unsigned char)hex_str[i + 1]))
        {
            return false;  // Not valid hex digits
        }

        char byte_str[3] = { hex_str[i], hex_str[i + 1], '\0' };
        out[i / 2] = (char)strtol(byte_str, NULL, 16);
    }

    out[len / 2] = '\0';  // Null-terminate output
    return true;
}
void asciiToHexStr(const char *asciiStr, char *hexStr)
{
    while (*asciiStr)
    {
        sprintf(hexStr, "%02X", (unsigned char)*asciiStr);
        hexStr += 2;
        asciiStr++;
    }
    *hexStr = '\0'; // Null-terminate the hex string
}
void modem_ble_update_client_lat_long()
{

    #define LAT_LONG_BUF_LEN 50
    char lat_long_data[LAT_LONG_BUF_LEN];
    char lat_long_hex_str[2 * LAT_LONG_BUF_LEN + 1]={0};
    char cmd[180];

    sprintf(lat_long_data,"%s,%s",Lat_string,Long_string);
    asciiToHexStr(lat_long_data, lat_long_hex_str);
    /* Need to take Byte Length instead of hex string length */
    sprintf(cmd, "AT+QBTLESEND=5,0,%d,\"%s\"", strlen(lat_long_data), lat_long_hex_str);


    modem_send_msg(cmd);
}
