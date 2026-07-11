#ifndef AWS_CONFIG_H    // Header Guard: If not defined...
#define AWS_CONFIG_H    // ...then define it


#include <Arduino.h>
#include "BLE.h"
#include <ArduinoJson.h> // for parsing the response from AWS Lambda, this library is included in the platformio.ini file and can be used to parse the JSON response from Lambda into a more readable format, you can also choose to parse it manually without this library if you prefer

// WIFI CONFIGS
#include <WiFi.h>
extern String WIFI_SSID;
extern String WIFI_PASSWORD;
void connectWiFi();

// AWS IOT configs
extern const char* AWS_ENDPOINT;
extern const char* AWS_ROOT_CA;

// DEVICE 1 CONFIGS
extern const char* DEVICE_1_CERT;
extern const char* DEVICE_1_PRIVATE_KEY;
extern const char* DEVICE_1_MQTT_TOPIC_PUB;
extern const char* DEVICE_1_MQTT_TOPIC_SUB;
extern const char* DEVICE_1_MQTT_PERSONALITY_TOPIC_PUB;
extern const char* DEVICE_1_MQTT_PERSONALITY_TOPIC_SUB;
extern const char* DEVICE_1_MQTT_DEVICE_INFO_PUB;
extern const char* DEVICE_1_MQTT_DEVICE_INFO_SUB;

// DEVICE 2 CONFIGS
extern const char* DEVICE_2_CERT;
extern const char* DEVICE_2_PRIVATE_KEY;
extern const char* DEVICE_2_MQTT_TOPIC_PUB;
extern const char* DEVICE_2_MQTT_TOPIC_SUB;
extern const char* DEVICE_2_MQTT_PERSONALITY_TOPIC_PUB;
extern const char* DEVICE_2_MQTT_PERSONALITY_TOPIC_SUB;
extern const char* DEVICE_2_MQTT_DEVICE_INFO_PUB;
extern const char* DEVICE_2_MQTT_DEVICE_INFO_SUB;

// Current Device configs
extern const char* CUR_DEVICE_CERT;
extern const char* CUR_PRIVATE_KEY;
extern const char* CUR_MQTT_TOPIC_PUB;
extern const char* CUR_MQTT_TOPIC_SUB;
extern const char* CUR_MQTT_PERSONALITY_TOPIC_PUB;
extern const char* CUR_MQTT_PERSONALITY_TOPIC_SUB;
extern const char* CUR_MQTT_DEVICE_INFO_TOPIC_PUB;
extern const char* CUR_MQTT_DEVICE_INFO_TOPIC_SUB;
extern const char* DEVICE_NAME_AWS; // specific device name to use for AWS IOT CORE connection, can be used for logging purposes in AWS IOT CORE to identify which device is which
extern String CUR_DEVICE_PERSONALITY; // this is the personality that will be sent to AWS Lambda to set up the device's personality in the conversation with ChatGPT, this can be used to make the two devices have different personalities and therefore have more interesting interactions with each other when they receive the response from ChatGPT and forward it to each other via BLE

// state variables
extern bool received_personality_from_aws; // flag to indicate if we have received the personality data from AWS yet, we want to wait to receive this before sending any messages to AWS so that the personality can be set up in AWS Lambda before we start sending messages and receiving responses that rely on the personality being set up

// for AWS IOT CORE connection and MQTT protocl
#include <WiFiClientSecure.h> // TLS encryption
#define MQTT_MAX_PACKET_SIZE 2048 // Adjust based on expected AI response length
#include <PubSubClient.h> // MQTT protocol

extern WiFiClientSecure net; // TCP encrypted by TLS
extern PubSubClient MQTT_client; // tells the MQTT protocol to use this TCP/IP with TLS

void connectAWS(const char* DEVICE_CERT, const char* PRIVATE_KEY, const char* MQTT_TOPIC_SUB, const char* MQTT_PERSONALITY_TOPIC_SUB, const char* DEVICE_NAME);
void send_message_to_aws(const String& message, const String& personality_data, const char* MQTT_TOPIC_SUB, const char* MQTT_TOPIC_PUB);
void set_up_personality(String& device_mac_id, const char* MQTT_PERSONALITY_TOPIC_SUB, const char* MQTT_PERSONALITY_TOPIC_PUB);
void receive_response_from_AWS(char* topic, byte* payload, unsigned int length);
void check_for_registration(const String& device_mac_id, const char* MQTT_TOPIC_SUB, const char* MQTT_TOPIC_PUB); // checks if device is registered in AWS dynamoDB by sending device_mac_id as primary ID, this will trigger a lambda function that checks if the device is registered and if it is, it will send back the personality data for the device to set up the personality of the device which will be used in response generation in AWS Lambda when processing messages from the device
#endif
