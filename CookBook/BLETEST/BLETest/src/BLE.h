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
#endif 