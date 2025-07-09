/*
 * EC200U.c
 *
 *  Created on: Jun 21, 2025
 *      Author: sai
 */
#include "EC200U.h"
#include "string.h"
#include "stdio.h"
#include "stdint.h"
#include "ATCommands.h"
#include "stm32g0xx_hal.h"
#include "cmsis_os.h"
#include "config.h"
#include "cJSON.h"
#include "Modem_RxProcess.h"
#include "stdlib.h"
/****************************** MACROS *************************************************/
#define MQTT_PUB_BUFF_LEN 512

/****************************** Private Variables **************************************/
uint8_t cmd_val=0;
char MQTT_PUB_Buff[MQTT_PUB_BUFF_LEN]={0};
/****************************** External Variables **************************************/
extern UART_HandleTypeDef huart1;
extern int Msg_cnt;
/****************************** Function Prototypes **************************************/
uint8_t modem_check_resp(const char *str,char *find_str)
{
    if (strstr(str, find_str) != NULL)
    {
        return 1;
    } else
    {
        return 0;
    }
}
void modem_send_msg(const char* msg)
{
	HAL_UART_Transmit(&huart1,(uint8_t*)msg,strlen(msg), 1000);
	HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", strlen("\r\n"), 1000);
}
void modem_initiate_cmd(uint8_t cmd)
{
	switch(cmd)
	{
		case MODEM_AT_CHECK:
		{
			modem_send_msg("AT");
			cmd_val=MODEM_AT_CHECK;
			break;
		}
		case MODEM_GET_INF0:
		{
			cmd_val=MODEM_GET_INF0;
			modem_send_msg("ATI");
			break;
		}
		case MODEM_GET_MANF_ID:
		{
			cmd_val=MODEM_GET_MANF_ID;
			modem_send_msg("AT+GMI");
			break;
		}
		case MODEM_DISABLE_ECHO:
		{
			modem_send_msg("ATE0");
			break;
		}
		case MODEM_ENABLE_ECHO:
		{
			modem_send_msg("ATE1");
			break;
		}
		case MODEM_GET_TA_MODEL_INFO:
		{
			cmd_val=MODEM_GET_TA_MODEL_INFO;
			modem_send_msg("AT+GMM");
			break;
		}
		case MODEM_CHECK_SIM_READY:
		{
			cmd_val=MODEM_CHECK_SIM_READY;
			modem_send_msg("AT+CPIN?");
			break;
		}
		case MODEM_SET_NETWORK_REG:
		{
			cmd_val=MODEM_SET_NETWORK_REG;
			modem_send_msg("AT+CREG=1");
			break;
		}
		case MODEM_CHECK_NETWORK_REG:
		{
			cmd_val=MODEM_CHECK_NETWORK_REG;
			modem_send_msg("AT+CREG?");
			break;
		}
		case MODEM_ATTACH_GPRS:
		{
			cmd_val=MODEM_ATTACH_GPRS;
			modem_send_msg("AT+CGATT=1");
			break;
		}
		case MODEM_DETACH_GPRS:
		{
			cmd_val=9;
			modem_send_msg("AT+CGATT=0");
			break;
		}
		case MODEM_CHECK_CGATT:
		{
			cmd_val=10;
			modem_send_msg("AT+CGATT=?");
			break;
		}
		case MODEM_SET_PDP:
		{
			cmd_val=MODEM_SET_PDP;
			char cmd[64];
			sprintf(cmd, "AT+CGDCONT=1,\"IP\",\"%s\"", AIRTEL_APN);
			modem_send_msg(cmd);
			break;
		}
		case MODEM_ACTIVATE_PDP:
		{
			cmd_val=MODEM_ACTIVATE_PDP;
			modem_send_msg("AT+QIACT=1");
			break;
		}
		case MODEM_RESET:
		{
			modem_send_msg("ATZ");
			break;
		}

		/********************************** MQTT AT Commands *****************************/
		case MODEM_MQTT_VERSION_CFG:
		{
			cmd_val=MODEM_MQTT_VERSION_CFG;
			char cmd[128];
			// --- Configure MQTT Version ---
			sprintf(cmd, "AT+QMTCFG=\"version\",%d,4", MQTT_CLIENT_IDX);  //// MQTT v3.1.1
			modem_send_msg(cmd);
			break;
		}
		case MODEM_MQTT_OPEN:
		{
			cmd_val=MODEM_MQTT_OPEN;
			char cmd[128];
			// --- Open MQTT Connection ---
			sprintf(cmd, "AT+QMTOPEN=%d,\"%s\",%d", MQTT_CLIENT_IDX, MQTT_HOSTNAME, MQTT_PORT);
			modem_send_msg(cmd);
			break;
		}
		case MODEM_MQTT_CONN:
		{
			cmd_val=MODEM_MQTT_CONN;
			char cmd[200];
			// --- Connect MQTT Client ---
			sprintf(cmd, "AT+QMTCONN=%d,\"%s\",\"%s\",\"%s\"", MQTT_CLIENT_IDX, MQTT_CLIENT_ID,MQTT_USERNAME,MQTT_PASSWORD);
			modem_send_msg(cmd);
			break;
		}
		case MODEM_MQTT_SUBSCRIBE:
		{
			cmd_val=MODEM_MQTT_SUBSCRIBE;
			char cmd[128];
			// --- Subscribe to Topic ---
			sprintf(cmd, "AT+QMTSUB=%d,1,\"%s\",%d", MQTT_CLIENT_IDX, MQTT_TOPIC_SUB, MQTT_QOS);
			modem_send_msg(cmd);
			break;
		}
		case MODEM_MQTT_PUBLISH:
		{
			cmd_val=MODEM_MQTT_PUBLISH;
			char cmd[128];
			//sprintf(cmd, "AT+QMTPUB=%d,0,%d,\"%s\"", MQTT_CLIENT_IDX, MQTT_QOS, MQTT_TOPIC_PUB);
			sprintf(cmd, "AT+QMTPUBEX=%d,%d,%d,%d,\"%s\",%d",
			        MQTT_CLIENT_IDX,
			        MQTT_MSG_ID,
			        MQTT_QOS,
			        MQTT_RETAIN_FLAG,
			        MQTT_TOPIC_PUB,
			        strlen(MQTT_PUB_Buff));
			modem_send_msg(cmd);
			osDelay(100);
			modem_send_msg(MQTT_PUB_Buff);
			break;
		}

		/********************************** BLE AT Commands *****************************/
		case MODEM_TURN_ON_BLE:
		{
			cmd_val=MODEM_TURN_ON_BLE;
			modem_send_msg("AT+QBTPWR=1");
			break;
		}
		case MODEM_TURN_OFF_BLE:
		{
			cmd_val=MODEM_TURN_OFF_BLE;
			modem_send_msg("AT+QBTPWR=0");
			break;
		}
		case MODEM_BLE_SET_ADV_PARAM:
		{
			cmd_val=MODEM_BLE_SET_ADV_PARAM;
			modem_send_msg("AT+QBTGATADV=1,60,120,0,0,7,0");
			break;
		}
		case MODEM_BLE_SET_SCAN_RESP_DATA:
		{
			cmd_val=MODEM_BLE_SET_SCAN_RESP_DATA;
			modem_send_msg("AT+QBTADVRSPDATA=13,\"0C094368617261454332303055\"");
			break;
		}
		case MODEM_BLE_SET_PRIMARY_SVC:
		{
			cmd_val=MODEM_BLE_SET_PRIMARY_SVC;
			//send_at_command("AT+QBTGATSS=0,1,6144,1\r\n");
			modem_send_msg("AT+QBTGATSS=0,1,44016,1");
			break;
		}
		case MODEM_BLE_ADD_SVC_CHAR:
		{
			cmd_val=MODEM_BLE_ADD_SVC_CHAR;
			modem_send_msg("AT+QBTGATSC=0,0,18,1,65268");  //18: Read and Notify
			break;
		}
		case MODEM_BLE_CFG_CHAR_VALUE:
		{
			cmd_val=MODEM_BLE_CFG_CHAR_VALUE;
			modem_send_msg("AT+QBTGATSCV=0,0,3,1,65268,42,\"48656C6C6F\"");
			//send_at_command("AT+QBTGATSCV=0,0,3,1,65268,42,\"BBFF\"\r\n");
			break;
		}
		case MODEM_BLE_FINSISH_ADDING_SVC:
		{
			cmd_val=MODEM_BLE_FINSISH_ADDING_SVC;
			modem_send_msg("AT+QBTGATSSC=1,1");
			break;
		}
		case MODEM_BLE_START_ADV:
		{
			cmd_val=MODEM_BLE_START_ADV;
			modem_send_msg("AT+QBTADV=1");
			break;
		}
		case MODEM_BLE_STOP_ADV:
		{
			cmd_val=MODEM_BLE_STOP_ADV;
			modem_send_msg("AT+QBTADV=0");
			break;
		}
		case MODEM_BLE_SET_NAME:
		{
			modem_send_msg("AT+QBTNAME=0,\"Chara_EC200U\"");
			break;
		}
		case MODEM_BLE_GET_NAME:
		{
			modem_send_msg("AT+QBTNAME?");
			break;
		}
		case MODEM_BLE_SEND_DATA:
		{
			//modem_ble_send_data_to_client(modem_ble_hex_data);
//			modem_ble_send_data_to_client(modem_ble_hex_data, sizeof(ble_data));
			break;
		}

		/********************************** GPS/GNSS AT Commands *****************************/

		case MODEM_GPS_TURN_ON:
		{
			modem_send_msg("AT+QGPS=1");
			break;
		}
		case MODEM_GPS_GET_CURR_LOCATION:
		{
			cmd_val=MODEM_GPS_GET_CURR_LOCATION;
			modem_send_msg("AT+QGPSLOC=0");
			break;
		}
		case MODEM_GPS_TURN_OFF:
		{

			modem_send_msg("AT+QGPSEND");
			break;
		}
		default:
		{
			break;
		}
	}
}
void get_modem_info()
{
	  modem_initiate_cmd(MODEM_GET_INF0);
	  osDelay(300);

	  modem_initiate_cmd(MODEM_GET_MANF_ID);
	  osDelay(300);

	  modem_initiate_cmd(MODEM_GET_TA_MODEL_INFO);
	  osDelay(300);
}
void modem_set_sim_configurations()
{
	  modem_initiate_cmd(MODEM_CHECK_SIM_READY);
	  osDelay(300);

	  modem_initiate_cmd(MODEM_SET_NETWORK_REG);
	  osDelay(300);

	  modem_initiate_cmd(MODEM_SET_PDP);
	  osDelay(300);

	  modem_initiate_cmd(MODEM_ATTACH_GPRS);
	  osDelay(300);

	  modem_initiate_cmd(MODEM_ACTIVATE_PDP);
	  osDelay(300);

	  /*
	  modem_initiate_cmd(MODEM_CHECK_SIM_READY);
	  osDelay(300);
	  */
}
void modem_reset()
{
	 modem_initiate_cmd(MODEM_RESET);
	 osDelay(300);
}
void modem_mqtt_init()
{
	modem_initiate_cmd(MODEM_MQTT_VERSION_CFG);
	osDelay(300);

	modem_initiate_cmd(MODEM_MQTT_OPEN);
	osDelay(2000);

	modem_initiate_cmd(MODEM_MQTT_CONN);
	osDelay(2000);

	modem_initiate_cmd(MODEM_MQTT_SUBSCRIBE);
	osDelay(2000);

	/*
	modem_initiate_cmd(MODEM_MQTT_VERSION_CFG);
	osDelay(300);
	*/
}
void modem_mqtt_publish()
{
	format_json_message();
	osDelay(100);
	modem_initiate_cmd(MODEM_MQTT_PUBLISH);
	osDelay(300);
}
void format_json_message(void)
{
    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
    {
        print_msg("JSON object creation failed\r\n");
        return;
    }

    cJSON_AddNumberToObject(root, "Msg_Count", Msg_cnt);

    char *json_str = cJSON_Print(root);
    if (json_str != NULL)
    {
        strncpy(MQTT_PUB_Buff, json_str, MQTT_PUB_BUFF_LEN - 1);
        MQTT_PUB_Buff[MQTT_PUB_BUFF_LEN - 1] = '\0';

        /*
        print_msg("Formatted JSON:\r\n");
        print_msg(MQTT_PUB_Buff);
		*/
        free(json_str);
    } else
    {
        print_msg("JSON formatting failed\r\n");
    }

    cJSON_Delete(root);
}

