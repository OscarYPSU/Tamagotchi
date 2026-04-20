#ifndef BLE_H
#define BLE_H

#include <Arduino.h>
#include <NimBLEDevice.h> // Only one include needed
#include "AWS_CONFIGS.h"

#define SECRET 0x5A 

extern bool allowed_connection; 
extern uint8_t nonce; 
extern std::string DEVICE_NAME; 

// Global pointers
extern NimBLECharacteristic *p_advertiser_characteristic; 
extern NimBLECharacteristic *p_advertiser_auth_characteristic; 
extern NimBLEAdvertising *p_advertise; // Removed the duplicate line here
extern NimBLEAdvertisementData advertising_data; 
extern NimBLEScan* p_scanner;
extern NimBLEAdvertisedDevice* target_device;
extern NimBLEClient* p_connector; 

// State variables
extern bool found_device; 
extern bool connected;
extern bool need_handshake;
extern unsigned long lastSwitchTime;
extern unsigned long nextInterval; 
extern char current_state; 

// SCANNER
class scanner_scan_callbacks: public NimBLEAdvertisedDeviceCallbacks {
    // FIX 1: Must be a pointer (*)
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) override;
};

void connect_to_server(NimBLEAdvertisedDevice* device);
void setup_scanning_mode();
void perform_handshake();

// ADVERTISER
class advertiser_interaction_callbacks : public NimBLECharacteristicCallbacks {
    public:
        void onWrite(NimBLECharacteristic *pCharacteristic) override;
};

class advertiser_system_call_backs : public NimBLEServerCallbacks {
    public:
        // Already correct: NimBLE needs the 'desc'
        void onConnect(NimBLEServer* self, ble_gap_conn_desc* desc) override;
        void onDisconnect(NimBLEServer* self) override;
};

class advertiser_authentication_callbacks : public NimBLECharacteristicCallbacks {
    public:
        void onWrite(NimBLECharacteristic *pCharacteristic) override;
};

void stop_advertising_signal();
void send_advertising_signal();
void setup_advertising_mode();

#endif