/*
 * Modem_RxProcess.c
 *
 *  Created on: Jun 22, 2025
 *      Author: sai
 */
#include "string.h"
#include "stdio.h"
#include "stdint.h"
#include "ATCommands.h"
#include "stm32g0xx_hal.h"
#include "cmsis_os.h"
#include "Modem_RxProcess.h"
#include "EC200U.h"
/****************************** MACROS *************************************************/

#define CHAR_IS_NUM(c)       ((c) >= '0' && (c) <= '9')
#define CHAR_TO_NUM(c)       ((c) - '0')

/****************************** Private Variables **************************************/
extern osThreadId ModemRx_TaskHandle;
struct modem_info modem_info_t;
uint8_t Modem_AT_check=0;
/****************************** External Variables **************************************/


/****************************** Function Prototypes **************************************/

void print_msg(const char *msg)
{
	HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg), 1000);
}

void Modem_Rx_Process_start()
{
	osThreadDef(ModemRxTask, ModemRx_Process, osPriorityNormal, 0, 256);
	ModemRx_TaskHandle = osThreadCreate(osThread(ModemRxTask), NULL);
}
void ModemRx_Process(void const * argument)
{
//	osDelay(2000);
	uint32_t ulNotifiedValue;
	for(;;)
	{
        // Wait for notification from ISR
		ulNotifiedValue=ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(ulNotifiedValue>0)
		{
			print_msg((const char*)EC200u_Rx_Buff);
			//print_msg("Rx Task Running\r\n");
			switch(cmd_val)
			{
				case MODEM_AT_CHECK:
				{
					osDelay(100);
					if(modem_check_resp((const char*)EC200u_Rx_Buff,"OK"))
					{
						Modem_AT_check=1;
						cmd_val=0;
					}
					break;
				}
				case MODEM_GET_INF0:
				{
					osDelay(100);
					const char* buffer_ptr = (const char*)EC200u_Rx_Buff;
					modem_parse_string(&buffer_ptr, modem_info_t.modem_prd_id_info, sizeof(modem_info_t.modem_prd_id_info));
					//strcpy(modem_info_t.modem_prd_id_info,(char*)EC200u_Rx_Buff);
					cmd_val=0;
					break;
				}
				case MODEM_GET_MANF_ID:
				{
					osDelay(100);
					const char* buffer_ptr = (const char*)EC200u_Rx_Buff;
					modem_parse_string(&buffer_ptr, modem_info_t.modem_manf_id, sizeof(modem_info_t.modem_manf_id));
					cmd_val=0;
					break;
				}
				case MODEM_GET_TA_MODEL_INFO:
				{
					osDelay(100);
					const char* buffer_ptr = (const char*)EC200u_Rx_Buff;
					modem_parse_string(&buffer_ptr, modem_info_t.modem_TA_model_info, sizeof(modem_info_t.modem_TA_model_info));
					cmd_val=0;
					memset(EC200u_Rx_Buff,0,sizeof(EC200u_Rx_Buff));
					break;
				}
				case MODEM_CHECK_SIM_READY:
				{
					if(modem_check_resp((const char*)EC200u_Rx_Buff,"READY"))
					{
						cmd_val=0;
						modem_info_t.simcard_info.sim_status=1;
						print_msg("Sim Card Detected\r\n");
					}
					else
					{
						modem_info_t.simcard_info.sim_status=255;
						print_msg("Sim Card Not Detected\r\n");
					}
					break;
				}
				case MODEM_CHECK_NETWORK_REG:
				{
					if(modem_check_resp((const char*)EC200u_Rx_Buff,"OK"))
					{
						cmd_val=0;
						modem_info_t.simcard_info.sim_reg_status=1;
						print_msg("Sim Card Registered\r\n");
					}
					else
					{
						modem_info_t.simcard_info.sim_reg_status=255;
						print_msg("Sim Card Registration Failed\r\n");
					}
					break;
				}
				case MODEM_ATTACH_GPRS:
				{
					if(modem_check_resp((const char*)EC200u_Rx_Buff,"OK"))
					{
						cmd_val=0;
						modem_info_t.simcard_info.gprs_attachment=1;
						print_msg("GPRS attachment sucessfull\r\n");
					}
					else
					{
						modem_info_t.simcard_info.gprs_attachment=255;
						print_msg("GPRS attachment Failed\r\n");
					}
					break;
				}
				case MODEM_SET_PDP:
				{
					if(modem_check_resp((const char*)EC200u_Rx_Buff,"OK"))
					{
						cmd_val=0;
						modem_info_t.simcard_info.pdp_status=1;
						print_msg("PDP is set\r\n");
					}
					else
					{
						modem_info_t.simcard_info.pdp_status=255;
						print_msg("PDP is Failed\r\n");
					}
					break;
				}
				case MODEM_ACTIVATE_PDP:
				{
					if(modem_check_resp((const char*)EC200u_Rx_Buff,"OK"))
					{
						cmd_val=0;
						modem_info_t.simcard_info.pdp_active_status=1;
						print_msg("PDP is active\r\n");
					}
					else
					{
						modem_info_t.simcard_info.pdp_active_status=255;
						print_msg("PDP is activation Failed\r\n");
					}
				}
				case MODEM_MQTT_VERSION_CFG:
				{
					if(modem_check_resp((const char*)EC200u_Rx_Buff,"OK"))
					{
						cmd_val=0;
						print_msg("MQTT Configutations Done\r\n");
					}
					break;
				}
				case MODEM_MQTT_OPEN:
				{
					if(modem_check_resp((const char*)EC200u_Rx_Buff, "+QMTOPEN"))
					{
						cmd_val=0;
						const char* p = (const char*)EC200u_Rx_Buff;
						modem_info_t.mqtt_info_t.mqtt_client_idx = modem_parse_number(&p);
						modem_info_t.mqtt_info_t.mqtt_open_stat  = modem_parse_number(&p);

					}
					else if(modem_check_resp((const char*)EC200u_Rx_Buff, "ERROR"))
					{
						print_msg("Failed to open MQTT network for a client\r\n");
						modem_info_t.mqtt_info_t.mqtt_client_idx=255;
						modem_info_t.mqtt_info_t.mqtt_open_stat=255;
					}
					break;
				}
				case MODEM_MQTT_CONN:
				{
					if(modem_check_resp((const char*)EC200u_Rx_Buff, "+QMTCONN"))
					{
						cmd_val=0;
						const char* p = (const char*)EC200u_Rx_Buff;
						modem_info_t.mqtt_info_t.mqtt_client_idx = modem_parse_number(&p);
						modem_info_t.mqtt_info_t.mqtt_conn_stat  = modem_parse_number(&p);
						modem_info_t.mqtt_info_t.mqtt_conn_ret_code  = modem_parse_number(&p);

					}
					else if(modem_check_resp((const char*)EC200u_Rx_Buff, "ERROR"))
					{
						print_msg("Failed to connect to a MQTT client\r\n");
						modem_info_t.mqtt_info_t.mqtt_client_idx=255;
						modem_info_t.mqtt_info_t.mqtt_conn_stat=255;
						modem_info_t.mqtt_info_t.mqtt_conn_ret_code=255;
					}
					break;
				}
				case MODEM_MQTT_SUBSCRIBE:
				{
					if(modem_check_resp((const char*)EC200u_Rx_Buff, "ERROR"))
					{
						modem_info_t.mqtt_info_t.mqtt_subs_stat=255;
						print_msg("Failed To subscribe to a topic\r\n");
					}
				}
				default:
				{
					break;
				}
			}

		}
		osDelay(10);
	}
}
#include <string.h>
#include <stdint.h>
#include <stddef.h>

/**
 * \brief           Parse ATI response and extract revision info
 * \param[in,out]   src: Pointer to pointer to ATI response string (multi-line)
 * \param[in]       dst: Destination buffer to copy revision into
 * \param[in]       dst_len: Size of destination buffer, including null terminator
 * \return          `1` on success, `0` otherwise
 */
uint8_t modem_parse_string(const char** src, char* dst, size_t dst_len)
{
    const char* p = *src;
    const char* rev_start = NULL;
    size_t i = 0;

    // Scan for "Revision: "
    while (*p != '\0') {
        if (strncmp(p, "Revision:", 9) == 0) {
            rev_start = p + 9;

            // Skip leading whitespace
            while (*rev_start == ' ') {
                ++rev_start;
            }

            // Copy up to newline or buffer limit
            if (dst != NULL && dst_len > 0) {
                --dst_len;
                while (*rev_start != '\0' && *rev_start != '\r' && *rev_start != '\n') {
                    if (i < dst_len) {
                        dst[i++] = *rev_start++;
                    } else {
                        break;
                    }
                }
                dst[i] = '\0';
            }

            *src = p;
            return 1;
        }

        // Move to next line
        while (*p != '\0' && *p != '\n') {
            ++p;
        }
        if (*p == '\n') {
            ++p;
        }
    }

    return 0; // Revision not found
}
int32_t modem_parse_number(const char** str)
{
    int32_t val = 0;
    uint8_t minus = 0;
    const char* p = *str;

    // Skip until we find a digit or minus sign
    while (*p && !(CHAR_IS_NUM(*p) || *p == '-')) {
        ++p;
    }

    // Handle negative sign if present
    if (*p == '-') {
        minus = 1;
        ++p;
    }

    // Parse the number
    while (CHAR_IS_NUM(*p)) {
        val = val * 10 + CHAR_TO_NUM(*p);
        ++p;
    }

    *str = p; // Save updated pointer
    return minus ? -val : val;
}
