#ifndef AWS_CONFIG_H    // Header Guard: If not defined...
#define AWS_CONFIG_H    // ...then define it

#include <Arduino.h>

// WIFI CONFIGS
#include <WiFi.h>
extern const char* WIFI_SSID;
void connectWiFi();

// AWS IOT configs
extern const char* AWS_ENDPOINT;
extern const char* AWS_ROOT_CA;

// DEVICE 1 CONFIGS
extern const char* DEVICE_1_CERT;
extern const char* DEVICE_1_PRIVATE_KEY;
extern const char* DEVICE_1_MQTT_TOPIC_PUB;
extern const char* DEVICE_1_MQTT_TOPIC_SUB;

// DEVICE 2 CONFIGS
extern const char* DEVICE_2_CERT;
extern const char* DEVICE_2_PRIVATE_KEY;
extern const char* DEVICE_2_MQTT_TOPIC_PUB;
extern const char* DEVICE_2_MQTT_TOPIC_SUB;

// Current Device configs
extern const char* CUR_DEVICE_CERT;
extern const char* CUR_PRIVATE_KEY;
extern const char* CUR_MQTT_TOPIC_PUB;
extern const char* CUR_MQTT_TOPIC_SUB;
extern const char* DEVICE_NAME_AWS; // specific device name to use for AWS IOT CORE connection, can be used for logging purposes in AWS IOT CORE to identify which device is which

// for AWS IOT CORE connection and MQTT protocl
#include <WiFiClientSecure.h> // TLS encryption
#define MQTT_MAX_PACKET_SIZE 2048 // Adjust based on expected AI response length
#include <PubSubClient.h> // MQTT protocol

extern WiFiClientSecure net; // TCP encrypted by TLS
extern PubSubClient MQTT_client; // tells the MQTT protocol to use this TCP/IP with TLS

void connectAWS(const char* DEVICE_CERT, const char* PRIVATE_KEY, const char* MQTT_TOPIC_SUB, const char* DEVICE_NAME);
void send_message_to_aws(const String& message, const char* MQTT_TOPIC_SUB, const char* MQTT_TOPIC_PUB);
void receive_response_from_AWS(char* topic, byte* payload, unsigned int length);
#endif
