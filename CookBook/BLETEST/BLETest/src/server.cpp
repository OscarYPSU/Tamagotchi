#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>


// just for getting mac address of the MCU
#include <WiFi.h>

#define LED_PIN 4 // Define our GPIO pin

// 2. Callback Class: This is the "brain" that reacts to your phone
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

class server_system_call_backs: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Device connected! 📱");
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("Device disconnected... 🔌");
      // This is the key: tell the ESP32 to start advertising again
      BLEDevice::startAdvertising();
      Serial.println("Restarted advertising!");
    }
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

  // Start Advertising
  BLEAdvertising *p_advertise = BLEDevice::getAdvertising();
  p_advertise->addServiceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
  p_advertise->setScanResponse(true);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now advertising...");

  // turns and connect to led
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:

}
