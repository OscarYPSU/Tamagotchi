#include <WiFi.h> // wifi functionality
#include <WiFiClientSecure.h> // TLS encryption
#define MQTT_MAX_PACKET_SIZE 2048 // Adjust based on expected AI response length
#include <PubSubClient.h> // MQTT protocol
#include <Arduino.h>


// wifi configs
const char* WIFI_SSID = "psu-personal";
// const char* WIFI_PASSWORD = "9172913763";  dont need since im using school wifi that doesnt require password

// AWS IOT configs
const char* AWS_ENDPOINT = "abuwn28a3fsb9-ats.iot.us-east-2.amazonaws.com";
// AWS certificate
const char* AWS_ROOT_CA = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
"rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
"-----END CERTIFICATE-----\n";
// Device cert
const char* DEVICE_CERT = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDWTCCAkGgAwIBAgIUQsWqKNV3f8nYApUxEKfUP4bMy9QwDQYJKoZIhvcNAQEL\n" \
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n" \
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI2MDMwNzAxNDgw\n" \
"NFoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n" \
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALv+i+dv2xjDwpXlYalc\n" \
"WWPB0v8qKalU5JebjIj1AR04wesuHoBvRG/nmIIr8RdoVbfIG7QCA9KQcoftpeaG\n" \
"aRvh+LU8Zul2WCYpiN5xtN2eOqgO1kZE5CEe4oMv+oQEHy68WVOOWdgyzAs3njk+\n" \
"X6XBpK4h4yb+pJdARGDeajJjqfMa+wNHd55CZXTBQFNPnuyn2JtGB5LGwjwIc0fK\n" \
"8KTYwlvQXiEVrIoXuW6u03aptuYwg5uozp07IYKy5lEyJgbDFklC2jJIWtfjDpn6\n" \
"o6O+diTxUd3MPdnVcwJZFzGoJggSoN1vxLqav3HDU5FoRInr6nYe22m4wrdmr/Yp\n" \
"LbUCAwEAAaNgMF4wHwYDVR0jBBgwFoAUbJV0VDsW9PGgwK9Q6t6OdBzF8+EwHQYD\n" \
"VR0OBBYEFGcRh1a9LsSv56OivUL2C25A+APkMAwGA1UdEwEB/wQCMAAwDgYDVR0P\n" \
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQCEDSrRZjVdH8R2Quuv4XS6vJhx\n" \
"kC5/PocAci+29Chm7HLnB+ZTxY2PG+3DptS+hQBwkPiS0KdI+/CDbSWLMwn7SfYW\n" \
"m759mWUi44mhudR7r2aCqRjBZXdYPsxOR7KqrDxkUMWwDn/5nbiJR0QQ/1jjY8lH\n" \
"5fvO0GTLfHivxpTZfKkHVbh7T5Zep4NRrC1em2935jlVZUiPYimRuayHipazJpHJ\n" \
"HENHAqgJ9U+Kh2wBmtt/BNUE1gIU4GVJ5cT5BFEKrzyQpGnrCsoJYKQ3yYcUbueN\n" \
"AhBEziAXegCEic7fYlDdPDKafSUmc+FK754ciUPzrAGtqfylQJXxPik5hP3L\n" \
"-----END CERTIFICATE-----\n";
// private key
const char* PRIVATE_KEY = \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEpAIBAAKCAQEAu/6L52/bGMPCleVhqVxZY8HS/yopqVTkl5uMiPUBHTjB6y4e\n" \
"gG9Eb+eYgivxF2hVt8gbtAID0pByh+2l5oZpG+H4tTxm6XZYJimI3nG03Z46qA7W\n" \
"RkTkIR7igy/6hAQfLrxZU45Z2DLMCzeeOT5fpcGkriHjJv6kl0BEYN5qMmOp8xr7\n" \
"A0d3nkJldMFAU0+e7KfYm0YHksbCPAhzR8rwpNjCW9BeIRWsihe5bq7Tdqm25jCD\n" \
"m6jOnTshgrLmUTImBsMWSULaMkha1+MOmfqjo752JPFR3cw92dVzAlkXMagmCBKg\n" \
"3W/Eupq/ccNTkWhEievqdh7babjCt2av9ikttQIDAQABAoIBAGsVtPWxJpl1sRqX\n" \
"XckOHLERUDIe/zpAbDHb3fKJtQfDM9rWG3PHbbFct+e2Rg+yU9lq+DTSTQEdXNAG\n" \
"09B7UdYbIwOBk5F64v1h/V/QX1k572mZgf7m8RfYsuhNZSLcAjMqAKvy00SussHU\n" \
"/aDX4b3WjwrNrMmilXi+l/Sh3NY0KwNYUlJCz+bcMAD+XszmABo3NqDz2TBA1yIn\n" \
"C+R3qznvif5NLIvNlt7jub6zfJpF1rpV9pLs7OgK4BrS3fkB9fDJhFx8nTKILN+U\n" \
"T9wvwsHllrC9tGxTqOen8a1Iyxh1cbCXDhyEfQoSry5EDE22hVqCS3ZXtMwelxC5\n" \
"1+LVDOUCgYEA5aicjpfguLKEBZXhlYaNGv2AgmXMdEBJfVWV/NPPVK59KUCLIsVA\n" \
"Mji3BkNk668a1Tb9GCYSYMnC4XvF/I8dChST0llEI5S1TkRXukdebU8S7aIoktZ8\n" \
"f+yJG1yPpr819uIHvJfkMO8F22QFsRJwS+1jYAsqXRtXwi2kIO7YBbMCgYEA0Y6P\n" \
"TyP8VfKJ9MZQbXpDvUvaJiSdn/CAVPJrURprSzJSb9T3KNKnc7e1jRYJFeX5rK5W\n" \
"w6mEPm8APdkxRxOq3Xttzi+j6FT4gs818VgNBQi00/1qWAAMezNbL821W5BjrCsz\n" \
"gRfx8z+wZRKGmU02F+qDYk2xUM6wMuzmiUN6mvcCgYEAtDeQQ0VjjwxnFYr8Lr4b\n" \
"VKbBhZIk0sTv5m6W+IOuGyGiDhEcHfjz23UuCrgwJdKPF+nCyoR1v4YUa6UZRIt5\n" \
"mkNzjImIMvOrIvP2c7M1okl+7QNnG3M815XiMZp9D7jUvBu7Pn9jDhNDYVooLT/u\n" \
"52YtqVyZrd6bC0GNnlpPIokCgYBvTMBg0EQdggwzrYDaJK4FxFvBFoGBiUMf2wGU\n" \
"uetyrO+L5hi/3eKCW3hZzvJUZykMxivfEBHk9x+xucGqdo+xwvS3JiZwJ3E2SMfl\n" \
"v1Zq7gzc6yOoSZ5XcRdldGR2lWHa97cLQaSIBbOyevxyP8gTO8M8Wvdqa9y5r4li\n" \
"LizYUQKBgQCIiJd8y1/fHIf1sFUb9DAKlELrbYxN5ZQF+DVX1gLsE/WPgE+gkhf+\n" \
"h9yeu2XhM1FpSN/yWLwUkpYfa08nyIP2qFtJ2eqxrfA6Uz594qs0ZGfR8EJJXIC+\n" \
"zlqzSSRQcx/h2KvcR/vrLAyR8bTG8GIZ8q/YJ61142rQh9ZSEH8owA==\n" \
"-----END RSA PRIVATE KEY-----\n";


// endpoint for publishing data
const char* MQTT_TOPIC_PUB = "device/data";
const char* MQTT_TOPIC_SUB = "openAI/response"; // receiving the openai response from aws 

WiFiClientSecure net; // TCP encrypted by TLS
PubSubClient MQTT_client(net); // tells the MQTT protocol to use this TCP/IP with TLS

// connecting to wifi
void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void send_message_to_aws(const String& message) {
  if (MQTT_client.connected()) {
    String msg = "{\"message\":\"" + message + "\"}";
    MQTT_client.publish(MQTT_TOPIC_PUB, msg.c_str());
    Serial.println("Published: " + msg);
  } else {
    Serial.println("MQTT not connected, cannot publish");
  }
}

void receive_response_from_AWS(char* topic, byte* payload, unsigned int length) {
  Serial.print("Response received on topic: ");
  Serial.println(topic);

  // Convert the byte payload into a readable String
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Print the ChatGPT response to your Serial Monitor
  Serial.println("--- OpenAI Response ---");
  Serial.println(message);
  Serial.println("-----------------------");
}

// connecting to AWS IOT CORE via MQTT
void connectAWS() {
  net.setCACert(AWS_ROOT_CA);
  net.setCertificate(DEVICE_CERT);
  net.setPrivateKey(PRIVATE_KEY);

  MQTT_client.setServer(AWS_ENDPOINT, 8883); // AWS IoT Core MQTT port
  MQTT_client.setCallback(receive_response_from_AWS); // set the callback function to handle incoming messages

  Serial.println("Connecting to AWS IoT...");
  while (!MQTT_client.connected()) {
    if (MQTT_client.connect("ESP_S3_Device")) {
      // SUBSCRIBE HERE
      // The topic must match what your Lambda is publishing to
      MQTT_client.subscribe("openAI/response"); // subscribe to the topic where AWS Lambda will publish the ChatGPT response
      
      // Logs 
      Serial.println("Subscribed to OpenAI response topic.");
      Serial.println("Connected to AWS IoT!");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(MQTT_client.state());
      Serial.println(" retrying in 2s");
      delay(2000);
    }
  }
}

// advertise its signal to allow python script connection to manually receive datas
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEServer.h>
#include <BLEDevice.h>

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Device connected! 📱");
    }
    
    // need to refactor this 
    void onDisconnect(BLEServer* pServer) {
      Serial.println("Device disconnected... 🔌");
      // This is the key: tell the ESP32 to start advertising again
      BLEDevice::startAdvertising();
      Serial.println("Restarted advertising!");
    }
};
// 2. Callback Class: This is the "brain" that reacts to your phone
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      
      if (value.length() > 0) {
        std::string command = value;
        Serial.print("Received Value: ");
        Serial.println(command.c_str());
        
        // sends message to AWS IOT CORE
        send_message_to_aws(String(command.c_str()));
      }
    }
};

void setup() {
  Serial.begin(115200);
  connectWiFi();
  connectAWS();

  // First we connect to the MCU via python script
  BLEDevice::init("ESP32-S3-Client");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService("7a9e19c4-1234-4a5b-8c6d-9e0f1a2b3c4d");

  // Create the characteristic BEFORE starting the service
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
    "1b2c3d4e-5f6a-7b8c-9d0e-1f2a3b4c5d6e",
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );

  // Link our Callback class to the characteristic
  pServer->setCallbacks(new MyServerCallbacks());
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Set initial value
  pCharacteristic->setValue("Send some messages");
  
  pService->start(); // Now we "open the doors"

  // Start Advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID("7a9e19c4-1234-4a5b-8c6d-9e0f1a2b3c4d");
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Ready to advertise...");

}

void loop() {
  // ensure connection stays alive
  if (!MQTT_client.connected()) {
    connectAWS();
  }
  MQTT_client.loop();
}