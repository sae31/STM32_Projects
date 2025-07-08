/*
 * ATCommands.h
 *
 *  Created on: Jun 21, 2025
 *      Author: sai
 */

#ifndef INC_ATCOMMANDS_H_
#define INC_ATCOMMANDS_H_

typedef enum commands
{

	MODEM_AT_CHECK=0, /* AT */
	MODEM_GET_INF0,  /* ATI */
	MODEM_ENABLE_ECHO, /* ATE */
	MODEM_DISABLE_ECHO,	/* ATE0 */
	MODEM_GET_MANF_ID,  /*AT+GMI */
	MODEM_GET_TA_MODEL_INFO, /* AT+GMM */
	MODEM_CHECK_SIM_READY, /* AT+CPIN? */
	MODEM_CHECK_SIGNAL_STRENGTH, /* AT+CSQ */
	MODEM_CHECK_NETWORK_REG,  /* AT+CREG? */
	MODEM_SET_NETWORK_REG, /* AT+CREG=1 */
	MODEM_ATTACH_GPRS,   /* AT+CATT=1 */
	MODEM_DETACH_GPRS,	/* AT+CATT=0 */
	MODEM_CHECK_CGATT,	/* AT+CATT=? */
	MODEM_SET_PDP,  /* AT+CGDCONT=1,"IP","<APN>"    AT+CGDCONT=1,"IP","airtelgprs.com" */
	MODEM_ACTIVATE_PDP, /*AT+QIACT=1 */
	MODEM_RESET,

	/*MQTT Related AT Commands */
	MODEM_MQTT_VERSION_CFG, /* AT+QMTCFG="version",<client_idx>[,<vsn>]  */
	MODEM_MQTT_OPEN, 		/* AT+QMTOPEN=<client_idx>,<host_name>,<port>  */
	MODEM_MQTT_GET_OPEN_STAT, /* AT+QMTOPEN? */
	MODEM_MQTT_CONN,		/* AT+QMTCONN=<client_idx>,<clientID>[,<username>,<password>]*/
	MODEM_MQTT_GET_CONN_STAT, /* AT+QMTCONN? */
	MODEM_MQTT_SUBSCRIBE,	/* AT+QMTSUB=<client_idx>,<msgid>,<topic1>,<qos1>[,<topic2>,<qos2>…] */
	MODEM_MQTT_PUBLISH,		/* AT+QMTPUBEX=<client_idx>,<msgid>,<qos>,<retain>,<topic>,<length>  */
	MODEM_MQTT_DISCONN,		/* AT+QMTDISC=<client_idx> */




}Modem_Commands_t;

#endif /* INC_ATCOMMANDS_H_ */
