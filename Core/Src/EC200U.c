/*
 * EC200U.c
 *
 *  Created on: Jun 21, 2025
 *      Author: sai
 */
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "ATCommands.h"
#include "stm32g0xx_hal.h"
#include "cmsis_os.h"
#include "config.h"
#include "cJSON.h"
#include "Modem_RxProcess.h"
#include "Modem_BLE.h"
#include "EC200U.h"
/****************************** MACROS *************************************************/
#define MQTT_PUB_BUFF_LEN 512

/****************************** STRUCT *************************************************/


GpsData GpsInfo_t;
mqtt_conf_t modem_mqtt_conf_t;
extern flag mqtt_flag;
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
			//modem_send_msg("AT+QBTGATADV=1,60,120,0,0,7,0");
			char cmd[128];
			sprintf(cmd,"AT+QBTGATADV=1,%d,%d,0,0,7,0",MIN_INTERVAL,MAX_INTERVAL);
			modem_send_msg(cmd);
			break;
		}
		case MODEM_BLE_SET_ADV_NAME:
		{
			cmd_val=MODEM_BLE_SET_ADV_NAME;
			char cmd[128];
			sprintf(cmd,"AT+QBTADVSTR=1,2,\"%s\"",DEFAULT_BLE_DEVICENAME);
			modem_send_msg(cmd);
			break;
		}
		/*
		case MODEM_BLE_SET_SCAN_RESP_DATA:
		{
			cmd_val=MODEM_BLE_SET_SCAN_RESP_DATA;
			//modem_send_msg("AT+QBTADVRSPDATA=13,\"0C094368617261454332303055\""); //CharaEC200U
			modem_send_msg("AT+QBTADVRSPDATA=10,\"5155454354454C444556\""); //QUECTELBLE
			break;
		}
		*/
		case MODEM_BLE_SET_PRIMARY_SVC:
		{
			cmd_val=MODEM_BLE_SET_PRIMARY_SVC;
			//send_at_command("AT+QBTGATSS=0,1,6144,1\r\n");
			//modem_send_msg("AT+QBTGATSS=0,1,44016,1");
			char cmd[128];
			sprintf(cmd,"AT+QBTGATSS=0,1,%d,1",GATTS_SERVICE_UUID);
			modem_send_msg(cmd);
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
//	osDelay(100);
	modem_initiate_cmd(MODEM_MQTT_PUBLISH);
	osDelay(300);
}
void modem_mqtt_connect()
{
	char cmd[200];
	// --- Connect MQTT Client ---
	sprintf(cmd, "AT+QMTCONN=%d,\"%s\",\"%s\",\"%s\"", MQTT_CLIENT_IDX, modem_mqtt_conf_t.mqtt_client_id,
							modem_mqtt_conf_t.mqtt_username,modem_mqtt_conf_t.mqtt_password);
	modem_send_msg(cmd);
}
void modem_mqtt_disconnect()
{
	modem_send_msg("AT+QMTDISC=0");
	osDelay(1000);
}
void modem_handle_mqtt_urc_codes()
{
	switch(modem_info_t.mqtt_info_t.mqtt_urc_error)
	{
		case 1:
		{
			modem_info_t.mqtt_info_t.mqtt_urc_error=0;
			modem_reset();
			modem_mqtt_init();
			break;
		}
		case 2:
		{
			modem_info_t.mqtt_info_t.mqtt_urc_error=0;
			modem_reset();
			modem_mqtt_init();
			break;
		}
		case 3:
		{
			modem_info_t.mqtt_info_t.mqtt_urc_error=0;
			modem_reset();
			modem_mqtt_init();
			break;
		}
		case 4:
		{
			modem_info_t.mqtt_info_t.mqtt_urc_error=0;
			modem_reset();
			modem_mqtt_init();
			break;
		}
		case 5:
		{
			modem_info_t.mqtt_info_t.mqtt_urc_error=0;
			break;
		}
		default:
		{
			break;
		}

	}
}
void format_json_message(void)
{
	char Lat_string[10],Long_string[10];
    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
    {
        print_msg("JSON object creation failed\r\n");
        return;
    }

    cJSON_AddNumberToObject(root, "Msg_Count", Msg_cnt);
    if(GpsInfo_t.latitude)
    {
    	//cJSON_AddNumberToObject(root, "Lat",GpsInfo_t.latitude);
    	//cJSON_AddNumberToObject(root, "Long",GpsInfo_t.longitude);
    	sprintf(Lat_string,"%.4f",GpsInfo_t.latitude);
    	sprintf(Long_string,"%.4f",GpsInfo_t.longitude);
		cJSON_AddStringToObject(root,"Lat",Lat_string);
		cJSON_AddStringToObject(root,"Long",Long_string);
    }
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
// Convert NMEA DMM (Degrees and Decimal Minutes) to Decimal Degrees
double convertDMMtoDecimal(const char *dmmStr, char direction)
{
    double dmm = atof(dmmStr);
    int degrees = (int)(dmm / 100);
    double minutes = dmm - (degrees * 100);
    double decimal = degrees + (minutes / 60.0);
    return (direction == 'S' || direction == 'W') ? -decimal : decimal;
}
int modem_parse_gps_location(const char *response, GpsData *data)
{
    const char *start = strstr(response, "+QGPSLOC:");
    if (!start) return -1;
    start += strlen("+QGPSLOC: ");

    char buffer[128];
    strncpy(buffer, start, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char *token = strtok(buffer, ",");
    int field = 0;
    char latStr[16], lonStr[16];
    char latDir = 'N', lonDir = 'E';

    while (token != NULL)
    {
        switch (field)
        {
            case 0:
                strncpy(data->utc_time, token, sizeof(data->utc_time) - 1);
                data->utc_time[sizeof(data->utc_time) - 1] = '\0';
                break;
            case 1:
                strncpy(latStr, token, sizeof(latStr) - 1);
                latStr[sizeof(latStr) - 1] = '\0';
                latDir = latStr[strlen(latStr) - 1];
                latStr[strlen(latStr) - 1] = '\0';
                break;
            case 2:
                strncpy(lonStr, token, sizeof(lonStr) - 1);
                lonStr[sizeof(lonStr) - 1] = '\0';
                lonDir = lonStr[strlen(lonStr) - 1];
                lonStr[strlen(lonStr) - 1] = '\0';
                break;
            case 3:
                data->hdop = atof(token);
                break;
            case 4:
                data->altitude = atof(token);
                break;
            case 5:
                data->fix = atoi(token);
                break;
            case 6:
                data->cog = atof(token);
                break;
            case 7:
                data->spkm = atof(token);
                break;
            case 8:
                data->spkn = atof(token);
                break;
            case 9:
                strncpy(data->date, token, sizeof(data->date) - 1);
                data->date[sizeof(data->date) - 1] = '\0';
                break;
            case 10:
                data->nsat = atoi(token);
                break;
        }

        token = strtok(NULL, ",");
        field++;
    }

    if (field < 11) return -2;

    data->latitude = convertDMMtoDecimal(latStr, latDir);
    data->longitude = convertDMMtoDecimal(lonStr, lonDir);

    return 0;
}

