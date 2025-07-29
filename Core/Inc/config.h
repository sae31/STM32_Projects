/*
 * config.h
 *
 *  Created on: Jun 28, 2025
 *      Author: SAI KUMAR
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

#include "main.h"

#define AIRTEL_APN 					"airtelgprs.com"

#define TIMERS_EN 					1
// ==== MQTT CONFIGURATION ====


#define MQTT_CLIENT_IDX      		0
#define MQTT_CLIENT_ID       		"STM32Client"
//#define MQTT_HOSTNAME        		"broker.hivemq.com"
#define MQTT_HOSTNAME        		"demo.thingsboard.io"
#define MQTT_PORT            		1883
#define MQTT_USERNAME        		"STM32DEMODEV"
#define MQTT_PASSWORD        		"STM32DEMODEV"
#define MQTT_TOPIC_PUB       		"v1/devices/me/telemetry"
#define MQTT_TOPIC_SUB       		"v1/devices/me/attributes"
#define MQTT_QOS             		1
#define MQTT_MSG_ID       	 		1
#define MQTT_RETAIN_FLAG  	 		0
#define TELEMETRY_SEND_TIME  		10   //seconds

// ==== BLE CONFIGURATION ====
#define MAX_INTERVAL  				120
#define MIN_INTERVAL  				60
#define DEFAULT_BLE_DEVICENAME  	"Quectel"

#define GATTS_MQTT_SERVICE_UUID  	0xABF0
#define MAX_CHAR_IN_MQTT_SVC     	6


// ==== GPS/GNSS CONFIGURATION ====
#define GPS_EN   					1
#define GPS_SEND_TIME        		5    //seconds



#endif /* INC_CONFIG_H_ */
