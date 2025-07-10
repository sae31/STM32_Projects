/*
 * Modem_RxProcess.h
 *
 *  Created on: Jun 22, 2025
 *      Author: sai
 */

#ifndef INC_MODEM_RXPROCESS_H_
#define INC_MODEM_RXPROCESS_H_



#include "cmsis_os.h"
#include "stm32g0xx_hal.h"

struct sim_t
{
	uint8_t sim_status;
	uint8_t sim_reg_status;
	uint8_t pdp_status;
	uint8_t gprs_attachment;
	uint8_t pdp_active_status;
};
struct mqtt_t
{
	uint8_t mqtt_open_stat;
	uint8_t mqtt_client_idx;
	uint8_t mqtt_conn_stat;
	uint8_t mqtt_conn_ret_code;
	uint8_t mqtt_subs_stat;
	uint8_t mqtt_urc_error;

};
struct modem_info
{
	char modem_prd_id_info[20];
	char modem_manf_id[25];
	char modem_TA_model_info[25];
	struct sim_t simcard_info;
	struct mqtt_t mqtt_info_t;
};

//osThreadId ModemRx_TaskHandle;
extern UART_HandleTypeDef huart2;
extern uint8_t EC200u_Rx_Buff[100];
extern uint8_t cmd_val;


void Modem_Rx_Process_start();
void ModemRx_Process(void const * argument);
uint8_t modem_parse_string(const char** src, char* dst, size_t dst_len);
int32_t modem_parse_number(const char** str);
void print_msg(const char *msg);

#endif /* INC_MODEM_RXPROCESS_H_ */
