/*
 * Modem_BLE.c
 *
 *  Created on: Jul 9, 2025
 *      Author: sai
 */
/*****************************************  Header Files ***********************************************/
#include "string.h"
#include "stdio.h"
#include "stdint.h"
#include "ATCommands.h"
#include "stm32g0xx_hal.h"
#include "cmsis_os.h"
#include "Modem_RxProcess.h"
#include "EC200U.h"
#include "Modem_BLE.h"

/***************************************** Private Variables ********************************************/
extern osThreadId ModemBLE_TaskHandle;
extern UART_HandleTypeDef huart1;
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
		osDelay(1000);
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
	osDelay(2000);

	modem_initiate_cmd(MODEM_BLE_SET_ADV_DATA);
	osDelay(2000);

	modem_initiate_cmd(MODEM_BLE_SET_SCAN_RESP_DATA);
	osDelay(2000);

	modem_initiate_cmd(MODEM_BLE_SET_PRIMARY_SVC);
	osDelay(2000);

	modem_initiate_cmd(MODEM_BLE_ADD_SVC_CHAR);
	osDelay(2000);

	modem_initiate_cmd(MODEM_BLE_CFG_CHAR_VALUE);
	osDelay(2000);

	modem_initiate_cmd(MODEM_BLE_FINSISH_ADDING_SVC);
	osDelay(2000);

	modem_initiate_cmd(MODEM_BLE_START_ADV);
	osDelay(2000);

	/*
	modem_initiate_cmd(MODEM_BLE_GET_NAME);
	vTaskDelay(2000 / portTICK_PERIOD_MS);
	*/




}
