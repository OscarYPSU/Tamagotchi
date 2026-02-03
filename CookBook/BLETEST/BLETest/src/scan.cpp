#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Arduino.h>

int scanTime = 5; // In seconds
BLEScan* pBLEScan;

// target service id we are looking
static BLEUUID target_serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");


// Callback class to handle found devices
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.printf("Device found: %s \n", advertisedDevice.toString().c_str());

      //found target service uuid, attemptin to connect
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(target_serviceUUID)){ 

      }

    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning for BLE devices...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); // Create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); // Active scan uses more power, but gets results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // Less than or equal to setInterval
}

void loop() {
  Serial.println("Starting Scan...");
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
  
  pBLEScan->clearResults();   // Delete results from BLEScan buffer to release memory
  delay(2000);
}