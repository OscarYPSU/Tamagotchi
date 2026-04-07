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

void setup() {
  Serial.begin(115200);

  // grabbing the MAC Address
  Serial.print("ESP32-S3 MAC Address: ");
  Serial.println(WiFi.macAddress());

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

void loop() {
  if (!is_connected && !connect_to_device){
    Serial.println("Scanning...");
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    Serial.println("Scan done!");
    
    // Clean up results to free memory
    pBLEScan->clearResults();
    delay(30000); // wait 30 second before scanning again
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
