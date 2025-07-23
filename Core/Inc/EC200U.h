/*
 * EC200U.h
 *
 *  Created on: Jun 21, 2025
 *      Author: sai
 */

#ifndef INC_EC200U_H_
#define INC_EC200U_H_

#include <stdint.h>

typedef struct
{
    char utc_time[11];         // UTC time (HHMMSS.SSS)
    double latitude;           // Decimal degrees
    double longitude;          // Decimal degrees
    double hdop;               // Horizontal dilution of precision
    double altitude;           // Altitude (meters)
    int fix;                   // Fix type (1=2D, 2=3D, etc.)
    double cog;                // Course over ground (degrees)
    double spkm;               // Speed in km/h
    double spkn;               // Speed in knots
    char date[7];              // Date (DDMMYY)
    int nsat;                  // Number of satellites used
} GpsData;

typedef struct
{
	char mqtt_username[20];
	char mqtt_password[20];
	char mqtt_client_id[20];
	uint16_t mqtt_port;
}mqtt_conf_t;
extern char MQTT_PUB_Buff[512];
extern GpsData GpsInfo_t;
extern struct modem_info modem_info_t;

uint8_t modem_check_resp(const char *str,char *find_str);
void modem_send_msg(const char* msg);
void modem_initiate_cmd(uint8_t cmd);
void get_modem_info();
void modem_set_sim_configurations();
void modem_reset();
void modem_mqtt_init();
void modem_mqtt_publish();
void modem_mqtt_connect();
void modem_mqtt_disconnect();
void format_json_message();
double convertDMMtoDecimal(const char *dmmStr, char direction);
int modem_parse_gps_location(const char *response, GpsData *data);
void modem_handle_mqtt_urc_codes();
#endif /* INC_EC200U_H_ */
