#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// just for getting mac address of the MCU
#include <WiFi.h>

#define LED_PIN 4 // Define our GPIO pin
#define DEVICE_ID 1 // BLE scanning usage to recognize which is our device

// determines what happens data is received from connected devices
class server_interaction_callbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        char command = value[0];
        Serial.print("Received Value: ");
        Serial.println(command);

        // Logic to toggle the LED based on the message
        if (command == '1') {
          digitalWrite(LED_PIN, HIGH);
          Serial.println("Action: LED ON 💡");
        } else if (command == '0') {
          digitalWrite(LED_PIN, LOW);
          Serial.println("Action: LED OFF 🌑");
        }
      }
    }
};

// startup callbacks for what happens the first time the other device connects to current device
class server_system_call_backs: public BLEServerCallbacks {
    // on connect, will display that a device has been connected
    void onConnect(BLEServer* pServer) {
      Serial.println("Device connected! 📱");
    };
    // when disconnected, it will open up adveritising again to allow other device to connect !!! need to work on multi connection and see if thats possible
    void onDisconnect(BLEServer* pServer) {
      Serial.println("Device disconnected... 🔌");
      // This is the key: tell the ESP32 to start advertising again
      BLEDevice::startAdvertising();
      Serial.println("Restarted advertising!");
    }
};

// UNFINISHED 
// server authentication callback to verify that the connected device is of own devices
class server_authentication_callbacks: public BLEServerCallbacks{
  
};

// Global pointers so we can reference them if needed
BLECharacteristic *server_p_characteristic;

void setup() {
  Serial.begin(115200);
  BLEDevice::init("ESP32-S3-Server");
  BLEServer *p_server = BLEDevice::createServer();
  BLEService *p_server_service = p_server->createService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");

  // Create the characteristic BEFORE starting the service
  server_p_characteristic = p_server_service->createCharacteristic(
    "beb5483e-36e1-4688-b7f5-ea07361b26a8",
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );

  // Link our Callback class to the characteristic
  p_server->setCallbacks(new server_system_call_backs());
  server_p_characteristic->setCallbacks(new server_interaction_callbacks());

  // Set initial value
  server_p_characteristic->setValue("Send 1 or 0");
  
  p_server_service->start(); // Now we "open the doors"

  BLEAdvertising *p_advertise = BLEDevice::getAdvertising(); // gets adveritising pointer from device so we can customize and start it
  BLEAdvertisementData advertising_data; // advertising data so other device can reconigize which device to connect to
  
  // create and attach the data to attatch to advertising data
  std::string manufacturerData = "";
  manufacturerData += (char)0xFF; // Company ID byte 1
  manufacturerData += (char)0xFF; // Company ID byte 2
  manufacturerData += (char)DEVICE_ID; // unique device ID

  advertising_data.setManufacturerData(manufacturerData); // attach the data to advertising data
  p_advertise->setAdvertisementData(advertising_data); // attach the adveritising data to advertise pointer
  p_advertise->addServiceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b"); 
  p_advertise->setScanResponse(true);
  BLEDevice::startAdvertising(); // starts the advertise
  Serial.println("Characteristic defined! Now advertising...");

  // turns and connect to led
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:

}
