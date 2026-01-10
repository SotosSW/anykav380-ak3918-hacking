/*
 * mqtt.h
 *
 *  Created on: Jan 10, 2026
 *      Author: sotossw
 */

#ifndef _MQTT_H_
#define _MQTT_H_

#if defined(__cplusplus)
extern "C" {
#endif

//extern struct MQTTClient_message;
//extern int MQTTClient_messageArrived(void*, char*, int, MQTTClient_message*);

//extern int MQTTClient_messageArrived;
/*
 * Initialize MQTT connection.
 *
 * @param[IN] address
 *  MQTT broker address.
 *
 * @param[IN] _port
 *  MQTT broker port.
 *
 * @param[IN] username
 *  Username to use for the authentication. set to NULL if connecting to an open broker
 *
 * @param[IN] password
 *  Password to use for the authentication. set to NULL if connecting to an open broker
 *
 * @param[IN] _topic
 *  Main topic. Sub-topics are auto generated.
 *
 * @return
 *  If Success Return 0, Else -1.
 */
int init_mqtt(char* address, char* _port, char* username, char* password, char* _topic, int(*cmd_addr)(char*));

/*
 * Send discovery request to Home Assistant.
 */
int ha_discovery(char* topic, char* manufacturer, char* device_name);

/*
 * Destroy the MQTT client
 */
int _destroy_mqtt();

#if defined(__cplusplus)
}
#endif

#endif /* _MQTT_H_ */
