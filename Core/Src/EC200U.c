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
			cmd_val=1;
			break;
		}
		case MODEM_GET_INF0:
		{
			cmd_val=2;
			modem_send_msg("ATI");
			break;
		}
		case MODEM_GET_MANF_ID:
		{
			cmd_val=3;
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
			cmd_val=4;
			modem_send_msg("AT+GMM");
			break;
		}
		case MODEM_CHECK_SIM_READY:
		{
			cmd_val=5;
			modem_send_msg("AT+CPIN?");
			break;
		}
		case MODEM_SET_NETWORK_REG:
		{
			cmd_val=6;
			modem_send_msg("AT+CREG=1");
			break;
		}
		case MODEM_CHECK_NETWORK_REG:
		{
			cmd_val=7;
			modem_send_msg("AT+CREG?");
			break;
		}
		case MODEM_ATTACH_GPRS:
		{
			cmd_val=8;
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
			cmd_val=11;
			char cmd[64];
			sprintf(cmd, "AT+CGDCONT=1,\"IP\",\"%s\"", AIRTEL_APN);
			modem_send_msg(cmd);
			break;
		}
		case MODEM_ACTIVATE_PDP:
		{
			cmd_val=12;
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
			cmd_val=13;
			char cmd[128];
			// --- Configure MQTT Version ---
			sprintf(cmd, "AT+QMTCFG=\"version\",%d,4", MQTT_CLIENT_IDX);  //// MQTT v3.1.1
			modem_send_msg(cmd);
			break;
		}
		case MODEM_MQTT_OPEN:
		{
			cmd_val=14;
			char cmd[128];
			// --- Open MQTT Connection ---
			sprintf(cmd, "AT+QMTOPEN=%d,\"%s\",%d", MQTT_CLIENT_IDX, MQTT_HOSTNAME, MQTT_PORT);
			modem_send_msg(cmd);
			break;
		}
		case MODEM_MQTT_CONN:
		{
			cmd_val=15;
			char cmd[128];
			// --- Connect MQTT Client ---
			sprintf(cmd, "AT+QMTCONN=%d,\"%s\"", MQTT_CLIENT_IDX, MQTT_CLIENT_ID);
			modem_send_msg(cmd);
			break;
		}
		case MODEM_MQTT_SUBSCRIBE:
		{
			cmd_val=16;
			char cmd[128];
			// --- Subscribe to Topic ---
			sprintf(cmd, "AT+QMTSUB=%d,1,\"%s\",%d", MQTT_CLIENT_IDX, MQTT_TOPIC_SUB, MQTT_QOS);
			modem_send_msg(cmd);
			break;
		}
		case MODEM_MQTT_PUBLISH:
		{
			cmd_val=17;
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

