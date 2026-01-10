#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "MQTTClient.h"
#include <cjson/cJSON.h>
#include "ak_common.h"

//MQTT declerations
#define QOS         1
#define TIMEOUT     10000L
MQTTClient client;
char cmd_topic[30];
char avt_topic[35];
char cam_conf_topic[60];
//char* mqtt_cmd;

//Function pointer to the proccess cmd in the main file.
int(*mqtt_process_cmd)(char*);

int ha_discovery(char* topic, char* manufacturer, char* device_name){

	if (client == NULL ){
		ak_print_error_ex("Error, MQTT client not initialized.\n");
		return -1;
	}

	ak_print_normal_ex("Creating JSON discovery request.\n");
	char *buffer = NULL;
	char topic_state[40];
	strcpy(topic_state,topic);
	strcat(topic_state,"_state");


	//Device identifiers
	cJSON *device = NULL;
	cJSON *identifiers = NULL;
	cJSON *identifiers_o = NULL;

	cJSON *mainjson =  cJSON_CreateObject();
	cJSON_AddItemToObject(mainjson, "device", device = cJSON_CreateObject());
	cJSON_AddItemToObject(device, "identifiers", identifiers = cJSON_CreateArray());
	identifiers_o = cJSON_CreateString(topic);
	cJSON_AddItemToArray(identifiers,identifiers_o);
	cJSON_AddStringToObject(device, "manufacturer", manufacturer);
	cJSON_AddStringToObject(device, "name", device_name);
	cJSON_AddStringToObject(device, "model", "AnykaV380 AK3918");

	cJSON_AddStringToObject(mainjson, "~", topic);
	cJSON_AddStringToObject(mainjson, "avty_t", "~/connected");

	cJSON_AddStringToObject(mainjson, "name", "Connection State");
	cJSON_AddStringToObject(mainjson, "unique_id", topic_state);
	cJSON_AddStringToObject(mainjson, "state_topic", "~/connected");
	cJSON_AddStringToObject(mainjson, "entity_category", "diagnostic");
	cJSON_AddStringToObject(mainjson, "payload_on", "online");
	cJSON_AddStringToObject(mainjson, "payload_off", "offline");
	cJSON_AddStringToObject(mainjson, "value_template", "{{ value_json.state }}");


	buffer = cJSON_Print(mainjson);
	if (buffer == NULL)
	{
		ak_print_error_ex("Failed to parse JSON data.\n");
		cJSON_Delete(mainjson);
		return -1;
	}

	int rc;
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	MQTTClient_deliveryToken token;
	pubmsg.payload = buffer;
	pubmsg.payloadlen = (int)strlen(buffer);
	pubmsg.qos = QOS;
	pubmsg.retained = 1;
	if ((rc = MQTTClient_publishMessage(client, cam_conf_topic, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
	{
		 ak_print_error_ex("Failed to publish message, return code %d\n", rc);
		 return -1;
	}
	if((rc = MQTTClient_waitForCompletion(client, token, TIMEOUT)) != MQTTCLIENT_SUCCESS)
	{
		ak_print_error_ex("Undeliverable message, return code: %d\n", rc);
		return -1;
	}
	//ak_print_normal_ex("Successfully parsed.\n%s\n",buffer);
	ak_print_normal_ex("Discovery request sent successfully");
	cJSON_Delete(mainjson);
	return 0;
}

int mqtt_publish_callback(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
	printf("Message received on topic: %s\n", topicName);
	printf("Payload: %.*s\n", message->payloadlen, (char*)message->payload);
	if (message->payloadlen <= 0){
		ak_print_error_ex("Invalid message length.\n");
		MQTTClient_freeMessage(&message);
		MQTTClient_free(topicName);
		return -1;
	}

	char msg[20];
	strcpy(msg,message->payload);
	mqtt_process_cmd(msg);
	//strcpy(mqtt_cmd,message->payload);
	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);
	return 1;
}

//Init MQTT
int init_mqtt(char* address, char* _port, char* username, char* password, char* _topic, int(*cmd_addr)(char*)) {
	char addr[50];
	char topic[20];
	const char* clientid = "hackedcamera";


	ak_print_normal_ex("Initializing MQTT client. Connecting to broker.\n");
	/* get address (argv[1] if present) */
	//addr=config.mqtt_b->ip;
	//port=config.mqtt_b->port;

	strcpy(addr,address);
	strcpy(topic,_topic);
	if ((addr == NULL) || (_port == NULL)) {
		ak_print_error_ex("Missing address or port from config.\n");
		return -1;
	}
	strcat(addr,":");
	strcat(addr,_port);
	memcpy(cmd_topic,topic,sizeof(cmd_topic));
	memcpy(avt_topic,topic,sizeof(cmd_topic));
	strcat(cmd_topic,"/ptz_cmd");
	strcat(avt_topic,"/connected");
	sprintf(cam_conf_topic,"homeassistant/binary_sensor/%s/config", topic);
	//ak_print_normal_ex("Conf topic:%s\n", cam_conf_topic);

	//ak_print_normal_ex("attempting connection with the following: username:%s\npassword:%s\nip:%s\nport:%s\ntopic:%s\n",usr,pwd,addr,port,topic);
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	MQTTClient_willOptions rip = MQTTClient_willOptions_initializer;
	int rc;

	// --- Step 1: Create the Client ---
	int mc = MQTTClient_create(&client, addr, clientid,
		MQTTCLIENT_PERSISTENCE_NONE, NULL);
	ak_print_normal_ex("MQTT client creation response: %s\n", MQTTClient_strerror(mc));

	// Set the message callback function
	//*mqtt_cmd=cmd_addr;
	mqtt_process_cmd=cmd_addr;
	MQTTClient_setCallbacks(client, NULL, NULL, mqtt_publish_callback, NULL);

	// --- Step 2: Configure Connection Options ---
	rip.topicName = avt_topic;
	rip.message = "offline";
	rip.retained = 1;
	conn_opts.keepAliveInterval = 20; // Send PINGREQ every 20 seconds
	conn_opts.cleansession = 1;       // Start a new session every time
	if(username != NULL){
		conn_opts.username = username;
	}
	if(password != NULL){
		conn_opts.password = password;
	}
	conn_opts.will = &rip;

	// --- Step 3: Connect to the Broker ---
	ak_print_normal_ex("Attempting to connect to EMQX Broker %s...\n", addr);
	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
		ak_print_error_ex("Failed to connect to EMQX, return code %d\n", rc);
		return rc;
	}
    if ((rc = MQTTClient_subscribe(client, cmd_topic, QOS)) != MQTTCLIENT_SUCCESS)
    {
    	printf("Failed to subscribe, return code %d\n", rc);
    	rc = EXIT_FAILURE;
    }

    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;

    pubmsg.payload = (char*)"online";
    pubmsg.payloadlen = 6;
    pubmsg.qos = QOS;
    pubmsg.retained = 1;
    if ((rc = MQTTClient_publishMessage(client, avt_topic, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
    {
    	ak_print_error_ex("Failed to publish message, return code %d\n", rc);
    }
	ak_print_normal_ex("Successfully connected to EMQX!\n");

	return 0;
}

int _destroy_mqtt(){
	if(client == NULL){
		ak_print_normal_ex("No MQTT client running. Skipping.\n");
		return -1;
	}
	MQTTClient_destroy(&client);
	ak_print_normal_ex("Killed MQTT client.\n");
	return 0;
}
