#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEServer.h>

// for mac address
#include <WiFi.h>

// The server to connect to
// The UUIDs of the service and characteristic you want to talk to
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

// Server setup for python script to connect to
bool is_connected; // if its connected to python script to be interacted with 

// scanning configs
int scanTime = 5; // Scan duration in seconds
BLEScan* pBLEScan;

// configs for connecting to other MCU
static BLEAdvertisedDevice* myDevice;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLERemoteService* pRemoteService;
bool found_device = false;
bool connect_to_device = false;

// configs for messaging to server
bool new_data = false; 
String new_data_string;

// attempts to send message to server that MCU is connected to
void send_data_to_server(String message){
  pRemoteCharacteristic->writeValue((uint8_t*)message.c_str(), message.length(), true);     
  Serial.println("Sent: " + message);
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
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        Serial.println("Connecting to other ESP32-s3");
        found_device = true;
    }
    
      // Print the basic info: Name, Address, and Signal Strength (RSSI)
      Serial.printf("Found Device: %s \n", advertisedDevice.toString().c_str());
    }
};

//attempts to connect to the server given the myDevice
void connect_to_server(){
  BLEClient* pClient  = BLEDevice::createClient();
  pClient->connect(myDevice); // Connect to the remote BLE Server
  pRemoteService = pClient->getService(serviceUUID);
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);

  if(pRemoteCharacteristic->canRead()) {
    std::string value = pRemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }
}

BLECharacteristic *pCharacteristic;

void setup() {
  Serial.begin(115200);

  // grabbing the MAC Address
  Serial.print("ESP32-S3 MAC Address: ");
  Serial.println(WiFi.macAddress());

  // First we connect to the MCU then we open the MCU up to scan other devices
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
  if (is_connected && !connect_to_device){
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

  if(new_data){
    // sends message to other connected ESP-S3
    send_data_to_server(String(pCharacteristic->getValue().c_str()));
    new_data = false;
  }

}
