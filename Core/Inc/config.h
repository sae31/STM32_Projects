/*
 * config.h
 *
 *  Created on: Jun 28, 2025
 *      Author: sai
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

#define AIRTEL_APN "airtelgprs.com"

// ==== MQTT CONFIGURATION ====


#define MQTT_CLIENT_IDX      0                       // MQTT client index (use 0)
#define MQTT_CLIENT_ID       "ec200u_client01"       // Unique MQTT client ID
#define MQTT_HOSTNAME        "broker.hivemq.com"     // MQTT broker hostname or IP
#define MQTT_PORT            1883                    // MQTT broker port (1883 for TCP, 8883 for TLS)
#define MQTT_USERNAME        ""                      // MQTT username
#define MQTT_PASSWORD        ""                      // MQTT password
#define MQTT_TOPIC_PUB       "test/topic"            // Topic to publish
#define MQTT_TOPIC_SUB       "test/sub"            // Topic to subscribe
#define MQTT_QOS             1                       // Quality of Service level (0, 1, or 2)
#define MQTT_MSG_ID       1
#define MQTT_RETAIN_FLAG  0

#endif /* INC_CONFIG_H_ */
