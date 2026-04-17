#ifndef BLE_H
#define BLE_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <arduino.h>
#include "AWS_CONFIGS.h"

#define SECRET 0x5A // secret for handshake protocl

extern bool allowed_connection; // only true after handshake taht allows connected device to start sending data
extern uint8_t nonce; // random nonce for authentication
extern std::string DEVICE_NAME; // specific device name to initilize BLE with, can be used for scanning purposes to identify which device is which

// Global pointers so we can reference them if needed
extern BLECharacteristic *p_advertiser_characteristic; // characteristics of advertiser device that other device will interact with 
extern BLECharacteristic *p_advertiser_auth_characteristic; // for authentication process
extern BLEAdvertising *p_advertise; // advertising pointer to start and stop advertising when needed
extern BLEAdvertising *p_advertise; // gets adveritising pointer from device so we can customize and start it
extern BLEAdvertisementData advertising_data; // the advertising signal data so we can change and update it with information
extern BLEScan* p_scanner;
extern BLEAdvertisedDevice* target_device;
extern BLEClient* p_connector; // the connector client that will be used to connect to the other device as a client, this is used in the scanning mode

// Variables that is used for switching between states in the main loop
extern bool found_device; // whether the device has found the target device, used to determine when to connect to it
extern unsigned long lastSwitchTime;
extern unsigned long nextInterval; // Start with 3 seconds
// (0 = idle, 1 = scanning, 2 = advertiser)
extern char current_state; // variable to keep track of the current state of the device, can be used for more complex interactions in the future



// SCANNER
// This class handles what happens when a device is found
class scanner_scan_callbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice);
};

void connect_to_server(BLEAdvertisedDevice* device);
void setup_scanning_mode();

// ADVERTISER
// determines what happens data is received from connected devices
class advertiser_interaction_callbacks : public BLECharacteristicCallbacks {
    public:
        void onWrite(BLECharacteristic *pCharacteristic) override;
};

// startup callbacks for what happens the first time the other device connects to current device
class advertiser_system_call_backs : public BLEServerCallbacks {
    public:
        void onConnect(BLEServer* self) override;
        void onDisconnect(BLEServer* self) override;
};

// advertiser authentication callback to verify that the connected device is of own devices
class advertiser_authentication_callbacks : public BLECharacteristicCallbacks {
    public:
        void onWrite(BLECharacteristic *pCharacteristic) override;
};

void stop_advertising_signal();
void send_advertising_signal();
void setup_advertising_mode();

void check_dual_mode();
#endif 