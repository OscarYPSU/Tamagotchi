#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEServer.h>

// for mac address
#include <WiFi.h>

#define SECRET 0x5A // secret for handshake protocl

// The server to connect to
// The UUIDs of the service and characteristic you want to talk to
static BLEUUID target_serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID target_charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");
static BLEUUID target_authentication_UUID("aeb5483e-36e1-4688-b7f5-ea07361b26a8");

// Server setup for python script to connect to
bool is_connected; // if its connected to python script to be interacted with 

// flag if handshake is needed
bool need_handshake = false; 

// scanning configs
int scanTime = 5; // Scan duration in seconds
BLEScan* pBLEScan;

// configs for connecting to other MCU
static BLEAdvertisedDevice* target_device;
static BLERemoteCharacteristic* target_pcharacteristic;
static BLERemoteCharacteristic* target_auth_characteristic;
static BLERemoteService* target_premoteService;
bool found_device = false;
bool connect_to_device = false;

// configs for messaging to server
bool new_data = false; 
String new_data_string;

// attempts to send message to server that MCU is connected to
void send_data_to_server(String message){
  target_pcharacteristic->writeValue((uint8_t*)message.c_str(), message.length(), true);     
  Serial.println("Sent: " + message);
}

//attempts to connect to the server given the myDevice
void connect_to_server(){
  BLEClient* pClient  = BLEDevice::createClient();
  pClient->connect(target_device); // Connect to the remote BLE Server
  target_premoteService = pClient->getService(target_serviceUUID);
  target_pcharacteristic = target_premoteService->getCharacteristic(target_charUUID);
  target_auth_characteristic = target_premoteService->getCharacteristic(target_authentication_UUID);

  if(target_pcharacteristic->canRead()) {
    std::string value = target_pcharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }
}

void perform_handshake(){
  // Step 1: read nonce from server
  std::string value = target_auth_characteristic->readValue();
  if (value.length() < 1) return;

  uint8_t nonce = (uint8_t)value[0]; // process info to correct data type = unsigned 8 int 
  Serial.print("Nonce received: ");
  Serial.println(nonce);

  // Step 2: compute response
  uint8_t response = nonce ^ SECRET;

  // Step 3: write response back
  target_auth_characteristic->writeValue(&response, 1, true);
  Serial.print("Response sent: ");
  Serial.println(response);

  return;
}

// 2. Callback Class: This is the "brain" that reacts to your phone
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      
      if (value.length() > 0) {
        char command = value[0];
        Serial.print("Received Value: ");
        Serial.println(command);
        
        // sets bool to true so code knows there is data avaiable to send
        new_data = true;
        new_data_string = String(value.c_str());
      }
    }
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Device connected! 📱");
      is_connected = true; // enables scanning for device now
      need_handshake = true;
    }
    
    // need to refactor this 
    void onDisconnect(BLEServer* pServer) {
      Serial.println("Device disconnected... 🔌");
      // This is the key: tell the ESP32 to start advertising again
      BLEDevice::startAdvertising();
      Serial.println("Restarted advertising!");
    }
};

// This class handles what happens when a device is found
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {      
      if (advertisedDevice.haveManufacturerData()){ 
        std::string scanned_device_manufacture_data = advertisedDevice.getManufacturerData();

         // Make sure it has at least 3 bytes
        if (scanned_device_manufacture_data.length() >= 3) {
            uint8_t companyID0 = (uint8_t)scanned_device_manufacture_data[0];
            uint8_t companyID1 = (uint8_t)scanned_device_manufacture_data[1];

            // Check for correct manufacture data and stop scanning and connect to it
            if (companyID0 == 0xFF && companyID1 == 0xFF) {
                BLEDevice::getScan()->stop();
                target_device = new BLEAdvertisedDevice(advertisedDevice);
                Serial.printf("attempting to connect to device: %s", advertisedDevice.toString().c_str());
                found_device = true;
            }
        }

      }

      // Print the basic info: Name, Address, and Signal Strength (RSSI)
      // Serial.printf("Found Device: %s \n", advertisedDevice.toString().c_str());
    }
};

BLECharacteristic *pCharacteristic;


/*
Set up for AWS IOT Core and MQTT
*/
#include <WiFiClientSecure.h> // TLS encryption
#define MQTT_MAX_PACKET_SIZE 2048 // Adjust based on expected AI response length
#include <PubSubClient.h> // MQTT protocol

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
"MIIDWTCCAkGgAwIBAgIUWpd/RG9zLTkBmL6+0wFS4hzZof0wDQYJKoZIhvcNAQEL\n" \
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n" \
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI2MDQwOTA0MTk1\n" \
"N1oXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n" \
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALcXWHLS7qpKqYchvx03\n" \
"Keye62dhMIzTJT8P/eWkDTwoBVVnw+Yn4pd/63sGje/SQlYcba22bElFx9Tm6UcL\n" \
"hNTXGQ8Rmd52plWTTjctY4NHpUUJle7uOXx1CGeSivoUU+4MWSCdHa9rOdzQ4W3e\n" \
"7XoBuzn47Os/X3vZ/MwDoZ4EMM9t+VOAxgbpltpmBk3N3Hkw6LdhZbbYfPoBM01A\n" \
"f2eGoZQQ1Mf6q0s1qDwjUAoqJuwNnqL+XQt2+7jE6K1yqEQqZ5qywuJPgHQJDTZC\n" \
"1Px8dDojLMe4GpjawjNFXLvqWfmhKrWbBNY9p8sMXBvBqQhWZdR48RuUdYTXSP7C\n" \
"gj0CAwEAAaNgMF4wHwYDVR0jBBgwFoAUCPVNUboSVEUMWIfETGuiuB7ZhF8wHQYD\n" \
"VR0OBBYEFM/JmfC7j0GNI0SHsX/gGnVEAU9LMAwGA1UdEwEB/wQCMAAwDgYDVR0P\n" \
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQB+nKdsYqw6m6TfCs8sT5t1qTUu\n" \
"tq7IjpBMZJs0eAcfmI5B8ulO3AQX7AmKwNiIEBc0o9e1RxQFPcTFGN667u7AmEc6\n" \
"24o3SqqFn3RBDIPAlsvZ63bKGWUil3DYMJka/8hf6t/I6IYtOr3UrUKbTDiRqFML\n" \
"PqWpVWtnv76rnKv9klV5us+j3ZIY8E3cXSDP4MdqhaRHWdY2VFxpNSd5b+A0Ek8G\n" \
"ZZ2VklhSpHfh529vcIRtPRVfS3XqSgMD4BQDK1B9kWYd60aWGzuykI4pVGpInNFk\n" \
"Km3o0jy6D7sBN50ySPHlvKuovs/HvfLB5StAN7mOk81EVmlvLxGMnYsaWGTb\n" \
"-----END CERTIFICATE-----\n";
// private key
const char* PRIVATE_KEY = \
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEpAIBAAKCAQEAtxdYctLuqkqphyG/HTcp7J7rZ2EwjNMlPw/95aQNPCgFVWfD\n"\
"5ifil3/rewaN79JCVhxtrbZsSUXH1ObpRwuE1NcZDxGZ3namVZNONy1jg0elRQmV\n"\
"7u45fHUIZ5KK+hRT7gxZIJ0dr2s53NDhbd7tegG7Ofjs6z9fe9n8zAOhngQwz235\n"\
"U4DGBumW2mYGTc3ceTDot2Fltth8+gEzTUB/Z4ahlBDUx/qrSzWoPCNQCiom7A2e\n"\
"ov5dC3b7uMTorXKoRCpnmrLC4k+AdAkNNkLU/Hx0OiMsx7gamNrCM0Vcu+pZ+aEq\n"\
"tZsE1j2nywxcG8GpCFZl1HjxG5R1hNdI/sKCPQIDAQABAoIBAArMNKoo6GxglNjk\n"\
"U5oMe8t5n9Zr8+oKNcMfVVDquOEYVyAvS21SYtmecKvDujEvjDv51zoMbRxxIhTb\n"\
"PVy0QDQxL0glOJbXFn4rfsxgP+MpIYUNyKrbn8ZIPKOnD80pz71VOxhiS7Lhtvrf\n"\
"0ruhDw5H9du4Y3iIN0BdnbwGZsyjxxa6qg0pTLAnLGrrsvA15jEDlKQVAewwCMOf\n"\
"PiLud2Htgr2ZrRmT8OfIZPCn9koSZZEfXIsz0F//z/wmOSR5mEHNgf2xurel/oOk\n"\
"SxhQFg0y2qFdGvNMVylUxLfg0uHFWWtXxSoPf54RRJ2CcgPMKdMOojyfU2ld78do\n"\
"gHCamSECgYEA3KvrV/dd1g5xnfqJirwXLPW8lfzXi/h/u0mlcuweV8rfBK90Zjbv\n"\
"9I5BpI0JpZLlf8a+DlpeXoXVbKpGf1pbMLa5dGJz19d7TPbDIP2LzzFyszn2qrt/\n"\
"5JvtnZrjIYvx+XDOxeVm0n5MA31GwPhUa2hrtLvoVHW1ggFiEnFeRikCgYEA1Gc4\n"\
"IUWozydwUTnvCbdpj5gAoff2RxXYugdCRE8T07tULZwwKCQ23GDMwVAPEmMWT+EX\n"\
"0MJ5eSLck11M2RVxoZXvV0nAHcDt48QXcWGBBEA3AQkzC2Sw3IaReEYmTdOOdDoO\n"\
"K7rEFNTOKR7fiRszpBOIk96BYCELt3s54E6IFfUCgYA1NDoczmZQatEX7sTkry7D\n"\
"R0g9vWVWuQZK6Jm8WJOERUR6A7eDwXEfPIE1JFAUHJO6t/cwzLb+ATSQ64jtwaJE\n"\
"33ldRzN48donl0M6nAbuYJSwA6SmS2itfK7Qlfx3JR+lLX6dFg8xZwP6v2Skt/ra\n"\
"nqWBQODmJC8r9htoKO866QKBgQCW+H7m91JTW57zvQ/wghNf4xSgC1VblWWkZEBv\n"\
"uR1Io5/jg16fSY5M+ejPho4P7aoQQNfipDgfJ+5MOEbFDf7kcWPbUSpie50bBWf8\n"\
"SOehE9uEuvszH/Ct7mA7cvEK5FIevp0P7AIvJEsc4zrTgygjeVbcc4zVvOdNFSNA\n"\
"EdGceQKBgQCfxSJmnNwQX4uDQ9nVLcZVSTjX9GU4XZjfXGAy4dhKf4PpGt0YgWIU\n"\
"8pkT0Cy4XN3vTs913HyX5ADJdaZayL5XxduR5j71PJ1CceA99mtm6wm1n1xvHsb5\n"\
"OBLvXgLrtDAtI+8GqI1rt4iWO6JVws95RsCx1J9weQl8Qu/i7Qi2zw==\n"\
"-----END RSA PRIVATE KEY-----\n";

const char* MQTT_TOPIC_PUB = "device/d2/input"; // endpoint for publishing data to openai for response generation
const char* MQTT_TOPIC_SUB = "device/d2/output"; // receiving the openai response from aws 

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
      MQTT_client.subscribe(MQTT_TOPIC_SUB); // subscribe to the topic where AWS Lambda will publish the ChatGPT response
      
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

void setup() {
  Serial.begin(115200);

  // grabbing the MAC Address
  Serial.print("ESP32-S3 MAC Address: ");
  Serial.println(WiFi.macAddress());

  connectWiFi(); // connect to wifi first before doing anything else
  connectAWS(); // connect to AWS IOT CORE so we can send and receive messages from the start of the program

  // First we connect to the MCU via python script then we open the MCU up to scan other devices
  BLEDevice::init("ESP32-S3-Client");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService("7a9e19c4-1234-4a5b-8c6d-9e0f1a2b3c4d");

  // Create the characteristic BEFORE starting the service
  pCharacteristic = pService->createCharacteristic(
    "1b2c3d4e-5f6a-7b8c-9d0e-1f2a3b4c5d6e",
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );

  // Link our Callback class to the characteristic
  pServer->setCallbacks(new MyServerCallbacks());
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Set initial value
  pCharacteristic->setValue("Send 1 or 0");
  
  pService->start(); // Now we "open the doors"

  // Start Advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID("7a9e19c4-1234-4a5b-8c6d-9e0f1a2b3c4d");
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Ready to advertise...");


  // Scanning process
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());

  pBLEScan->setActiveScan(true); // Active scan gathers more data (like names) but uses more power
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

// to prevent it from scanning continuously and overwhelming the device, we will only scan every 30 seconds if its not already connected to the server
unsigned long lastScanTime = 0;
const unsigned long scanInterval = 30000; // 30 seconds

void loop() {
  // put your main code here, to run repeatedly:


  // ensure connection to AWS IOT CORE stays alive
  if (!MQTT_client.connected()) {
    connectAWS();
  }
  MQTT_client.loop();

  unsigned long currentMillis = millis();
  if (!is_connected && !connect_to_device && (currentMillis - lastScanTime > scanInterval || lastScanTime  == 0)) {
    Serial.println("Scanning...");
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    Serial.println("Scan done!");
    
    // Clean up results to free memory
    pBLEScan->clearResults();
    lastScanTime = millis(); // Update the last scan time
  }
  
  if (found_device){
    connect_to_server();
    found_device = false; // so it doesnt repeat and keep trying to connect to it
    connect_to_device = true;
  }
  if (need_handshake){
    perform_handshake(); // performs handshake
    need_handshake = false;
  }
  if(new_data){
    // sends message to other connected ESP-S3
    send_data_to_server(String(pCharacteristic->getValue().c_str()));
    new_data = false;
  }

}
