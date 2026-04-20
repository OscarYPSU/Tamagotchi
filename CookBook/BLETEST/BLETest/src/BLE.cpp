// ----------------
// BLE SET UP
// ----------------
#include "BLE.h"
#define SECRET 0x5A // secret for handshake protocl

bool allowed_connection = false; // only true after handshake that allows connected device to start sending data
uint8_t nonce; // random nonce for authentication
std::string DEVICE_NAME; // specific device name to initilize BLE with, can be used for scanning purposes to identify which device is which

NimBLECharacteristic *p_advertiser_characteristic;
NimBLECharacteristic *p_advertiser_auth_characteristic; // for authentication process
NimBLEAdvertising *p_advertise;
NimBLEAdvertisementData advertising_data; // the advertising signal data so we can change and update it with information
unsigned long lastSwitchTime = 0;
unsigned long nextInterval = 3000; // Start with 3 seconds
// (0 = idle, 1 = scanning, 2 = advertiser)
char current_state = 0; // variable to keep track of the current state of the device, can be used for more complex interactions in the future
NimBLEAdvertisedDevice* target_device; // Swapped to NimBLE


// -----------
// BLE SETUP for advertiser mode
// -----------


void setup_advertising_mode(){
    Serial.println("Setting up advertising mode...");
    NimBLEDevice::init(DEVICE_NAME); // NimBLE version
    NimBLEServer *p_advertiser = NimBLEDevice::createServer();
    NimBLEService *p_advertiser_service = p_advertiser->createService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
    
    // create and attach the data to attatch to advertising data
    std::string manufacture_data = "";
    manufacture_data += (char)0xFF; // Company ID byte 1
    manufacture_data += (char)0xFF; // Company ID byte 2

    // Create the characteristic - Swapped to NIMBLE_PROPERTY namespace
    p_advertiser_characteristic = p_advertiser_service->createCharacteristic(
        "beb5483e-36e1-4688-b7f5-ea07361b26a8",
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
    );
    p_advertiser_auth_characteristic = p_advertiser_service->createCharacteristic(
        "aeb5483e-36e1-4688-b7f5-ea07361b26a8", // auth UUID
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
    );

    // Link our Callback class to the characteristic
    p_advertiser->setCallbacks(new advertiser_system_call_backs());
    p_advertiser_characteristic->setCallbacks(new advertiser_interaction_callbacks());
    p_advertiser_auth_characteristic->setCallbacks(new advertiser_authentication_callbacks()); // attach the authenication behavioral code to the characterstics

    p_advertiser_service->start(); // Finish creating the service
    
    p_advertise = NimBLEDevice::getAdvertising(); // NimBLE version

    // create and attach the data to attatch to advertising data
    p_advertise->setManufacturerData(manufacture_data); // NimBLE handles this directly on the advertiser object usually
    p_advertise->setScanResponse(true); 

    p_advertise->start();
    Serial.println("Started advertising...");
}   


// determines what happens data is received from connected devices
void advertiser_interaction_callbacks::onWrite(NimBLECharacteristic *self_characteristic) {
      if (allowed_connection){
        std::string value = self_characteristic->getValue();
        Serial.print("Received value: ");
        Serial.println(value.c_str());

        // sends message to AWS IOT CORE to then be processed by Lambda and ChatGPT
        send_message_to_aws(String(value.c_str()), CUR_MQTT_TOPIC_SUB, CUR_MQTT_TOPIC_PUB); 
      }
};

// Updated signature with ble_gap_conn_desc* desc
void advertiser_system_call_backs::onConnect(NimBLEServer* self, ble_gap_conn_desc* desc) {
    Serial.println("Device attempting to connect, sending authentication challenge!");      
    connected = true; // we are now connected, but not necessarily authenticated yet, we will use this variable to keep track of the connection state and avoid multiple connections at onceq

    // Generate a new random nonce
    nonce = random(1, 255);
    // Write it to the auth characteristic
    p_advertiser_auth_characteristic->setValue(&nonce, 1); // setting the value of the auth char so client can grab it and sent the answer
    Serial.print("Nonce sent to client: ");
    Serial.println(nonce);
};

void advertiser_system_call_backs::onDisconnect(NimBLEServer* self) {
    Serial.println("Device disconnected... 🔌");
    connected = false; // reset connection state
    allowed_connection = false;  // reset auth
    NimBLEDevice::getAdvertising()->start(); // restart advertising so other devices can find and connect to us
    Serial.println("Resumed advertising... 📢");
};


// advertiser authentication callback to verify that the connected device is of own devices
void advertiser_authentication_callbacks::onWrite(NimBLECharacteristic *self_characteristic) {
    std::string value = self_characteristic->getValue();
    if (value.length() < 1) return; // not valid
    uint8_t response = (uint8_t)value[0]; // process value into response data form (unsigned int 8)
    if (response == (nonce ^ SECRET)) {  // ✅ verify XOR  authentication
        allowed_connection = true;
        Serial.println("Handshake success ✅");
    } else {
        allowed_connection = false;
        Serial.println("Handshake failed ❌");
    }
};


// ------------
// BLE scanning and connecting to other device as a client
// ------------
NimBLEScan* p_scanner;
bool found_device = false;
bool connected = false;
bool need_handshake = false;
NimBLERemoteCharacteristic  *target_auth_characteristic; // for authentication process

// --- CLIENT CALLBACKS ---
// Handles events when WE connect to a remote device
class client_callbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pClient) {
        Serial.println(">>> We connected to peer (We are Central/Client)");
        need_handshake = true; // we will perform the handshake in the main loop, we do this because we cannot call perform_handshake directly from this callback function since it needs to read and write to the characteristic and that can only be done after the connection is fully established and the characteristic is properly set up, which happens in the main loop after this callback is called.
        connected = true;

        // After connecting, we need to find the correct characteristic for authentication
        NimBLERemoteService* target_remote_service = pClient->getService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
        if (target_remote_service) {
            target_auth_characteristic = target_remote_service->getCharacteristic("aeb5483e-36e1-4688-b7f5-ea07361b26a8");
            if (target_auth_characteristic) {
                Serial.println("Found authentication characteristic! Ready to perform handshake.");
            } else {
                Serial.println("Failed to find authentication characteristic. Disconnecting...");
                pClient->disconnect();
            }
        } else {
            Serial.println("Failed to find target service. Disconnecting...");
            pClient->disconnect();
        }
    }
    void onDisconnect(NimBLEClient* pClient) {
        Serial.println(">>> Client disconnected. Resuming scan...");
        connected = false; 
        need_handshake = false; // reset handshake state
        NimBLEDevice::getScan()->start(0, false);
    }
};


void scanner_scan_callbacks::onResult(NimBLEAdvertisedDevice* advertisedDevice) {      
    // Print the basic info: Name, Address, and Signal Strength (RSSI)
    // Serial.printf("Found Device: %s \n", advertisedDevice->toString().c_str());

    if (advertisedDevice->haveManufacturerData()){ 
        std::string scanned_device_manufacture_data = advertisedDevice->getManufacturerData();
        uint8_t companyID0 = (uint8_t)scanned_device_manufacture_data[0];
        uint8_t companyID1 = (uint8_t)scanned_device_manufacture_data[1];

        // Check for correct manufacture data and stop scanning and connect to it
        if (companyID0 == 0xFF && companyID1 == 0xFF) {
            Serial.print("Target device found: ");
            Serial.print(advertisedDevice->getAddress().toString().c_str());
            Serial.print(" | Name: ");
            Serial.println(advertisedDevice->getName().c_str());

            // --- NEW TIE-BREAKER LOGIC ---
            // Compare my address to the found device's address
            if (NimBLEDevice::getAddress() > advertisedDevice->getAddress()) {
                Serial.println("I have the higher MAC. I will initiate the connection!");
                found_device = true;
                NimBLEDevice::getScan()->stop();
                target_device = advertisedDevice; // store the target device so we can connect to it in the main loop, we do this because we cannot call connect directly from this callback function
            } else {
                Serial.println("I have the lower MAC. I will stay in 'Server mode' and wait for them.");
                // We do nothing here; we just keep scanning/advertising
            }
            // -----------------------------
        }    
    }
};

// sets up the device to be able to scan and connect to other BLE devices as a client
void setup_scanning_mode(){
    Serial.println("Setting up scanning mode...");
    p_scanner = NimBLEDevice::getScan();
    p_scanner->setAdvertisedDeviceCallbacks(new scanner_scan_callbacks()); 

    // '0' means scan forever, 'false' means non-blocking
    p_scanner->setActiveScan(true);
    p_scanner->start(0, false);
}

//atempts to connect to the server  
void connect_to_server(NimBLEAdvertisedDevice* target_device){
    Serial.println("Forming a connection to the target device...");

    // 1. Create the client
    NimBLEClient* p_connector = NimBLEDevice::createClient();
    p_connector->setClientCallbacks(new client_callbacks(), false); // connect the callbacks tot he client

    if(p_connector->connect(target_device)) { 
        Serial.println("Connected to the target device! ✅");
    } else {
        Serial.println("Failed to connect. Retrying next scan cycle... ❌");
        found_device = false; 
        NimBLEDevice::getScan()->start(0, false);
    }
}

// performs the handshake by reading the nonce, computing the response, and writing it back to the server
void perform_handshake(){
    // Step 1: read nonce from server
    std::string value = target_auth_characteristic->readValue();
    if (value.length() < 1) return;

    uint8_t nonce = (uint8_t)value[0]; // process info to correct data type = unsigned 8 int 
    Serial.print("Nonce received: ");
    Serial.println(nonce);

    // Step 2: compute response
    uint8_t response = nonce ^ SECRET;

    target_auth_characteristic->writeValue(&response, 1, true);
    Serial.print("Response sent: ");
    Serial.println(response);

    //send_data_to_server("Hello!");
        need_handshake = false; // reset handshake state so it doesnt keep trying to perform handshake

    return;
}