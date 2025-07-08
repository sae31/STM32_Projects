/*
 * EC200U.h
 *
 *  Created on: Jun 21, 2025
 *      Author: sai
 */

#ifndef INC_EC200U_H_
#define INC_EC200U_H_

#include "stdint.h"

extern char MQTT_PUB_Buff[512];

uint8_t modem_check_resp(const char *str,char *find_str);
void modem_send_msg(const char* msg);
void modem_initiate_cmd(uint8_t cmd);
void get_modem_info();
void modem_set_sim_configurations();
void modem_reset();
void modem_mqtt_init();
void modem_mqtt_publish();
void format_json_message();

#endif /* INC_EC200U_H_ */
