#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Arduino.h>

int scanTime = 5; // In seconds
BLEScan* pBLEScan;

// target service id we are looking
static BLEUUID target_serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");

// storing of pointer for found device to connect t 
BLEAdvertisedDevice* targetDevice = nullptr;
// Callback class to handle found devices
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  // For each of scanned device choose the device that matches manufacture data and connects to it
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.printf("Device found: %s \n", advertisedDevice.toString().c_str());

      //found target service uuid, attemptin to connect
      if (advertisedDevice.haveManufacturerData()){ 
        std::string scanned_device_manufacture_data = advertisedDevice.getManufacturerData();

         // Make sure it has at least 3 bytes
        if (scanned_device_manufacture_data.length() >= 3) {
            uint8_t companyID0 = (uint8_t)scanned_device_manufacture_data[0];
            uint8_t companyID1 = (uint8_t)scanned_device_manufacture_data[1];

            // Example check for your test devices
            if (companyID0 == 0xFF && companyID1 == 0xFF) {
                Serial.println("Found device");
                // connect code here
                targetDevice = new BLEAdvertisedDevice(advertisedDevice); // store it
                pBLEScan->stop(); // stop scanning
            }
        }
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