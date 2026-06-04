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
extern NimBLECharacteristic *p_web_or_mcu_characterstic;
extern NimBLEAdvertising *p_advertise; // Removed the duplicate line here
extern NimBLEAdvertisementData advertising_data; 
extern NimBLEScan* p_scanner;
extern NimBLEAdvertisedDevice* target_device;
extern NimBLEClient* p_connector; 
extern NimBLERemoteCharacteristic  *target_auth_characteristic; // Scanner sending to advertiser for handshake purpose
extern NimBLERemoteCharacteristic  *target_characterstic; // for sending data to server after handshake is complete, we store it here after we discover it in the onConnect callback of the client so we can use it later in the main loop to send data to the server after the handshake is complete
extern NimBLERemoteCharacteristic  *target_web_or_mcu_characterstic; // for differentiating between web and MCU clients
// State variables
extern bool found_device; 
extern bool connected;
extern bool have_message_from_connector;
extern bool need_handshake;
extern String message_from_connector;
extern int current_state; // 0 for being advertiser/server, 1 for being scanner/client
extern bool has_setup_adveriser_mode;
extern bool has_setup_scanning_mode;
extern bool disonnected_from_client; // flag to indicate if we have been disconnected from a client, used to trigger restarting advertising and scanning
extern long advertiser_scanner_interval; // how long to advertise and scan for
extern int web_or_mcu; // 0 for web, 1 for MCU
extern bool need_web_or_mcu_identification; // flag to indicate if we need to identify as web or MCU after handshake is complete

// SCANNER
class scanner_scan_callbacks: public NimBLEAdvertisedDeviceCallbacks {
    // FIX 1: Must be a pointer (*)
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) override;
};

bool connect_to_server(NimBLEAdvertisedDevice* device);
void setup_scanning_mode();
void start_scanning();
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

class advertiser_web_or_mcu_callbacks : public NimBLECharacteristicCallbacks {
    public:
        void onWrite(NimBLECharacteristic *pCharacteristic) override;
};

void start_advertising();
void setup_advertising_mode();
void send_data_to_server(std::string message);
void send_message_to_connected_device(std::string& message);
void notify_call_backs(BLERemoteCharacteristic* target_characterstic, uint8_t* data, size_t length, bool is_notify);
void notify_call_backs_web_or_mcu(BLERemoteCharacteristic* target_characterstic, uint8_t* data, size_t length, bool is_notify);
void notifiy_for_web_or_mcu();
void authenticate_as_mcu();

void master_send_data(String& message);
#endif