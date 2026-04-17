// ----------------
// BLE SET UP
// ----------------
#include "BLE.h"
#define SECRET 0x5A // secret for handshake protocl


bool allowed_connection = false; // only true after handshake that allows connected device to start sending data
uint8_t nonce; // random nonce for authentication
std::string DEVICE_NAME; // specific device name to initilize BLE with, can be used for scanning purposes to identify which device is which
BLECharacteristic *p_advertiser_characteristic;
BLECharacteristic *p_advertiser_auth_characteristic; // for authentication process
BLEAdvertising *p_advertise;
BLEAdvertisementData advertising_data; // the advertising signal data so we can change and update it with information
unsigned long lastSwitchTime = 0;
unsigned long nextInterval = 3000; // Start with 3 seconds
// (0 = idle, 1 = scanning, 2 = advertiser)
char current_state = 0; // variable to keep track of the current state of the device, can be used for more complex interactions in the future
BLEAdvertisedDevice* target_device;;
// -----------
// BLE SETUP for advertiser mode
// -----------


void setup_advertising_mode(){
    Serial.println("Setting up advertising mode...");
    BLEDevice::init(DEVICE_NAME);
    BLEServer *p_advertiser = BLEDevice::createServer();
    BLEService *p_advertiser_service = p_advertiser->createService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
    
    // create and attach the data to attatch to advertising data
    std::string manufacture_data = "";
    manufacture_data += (char)0xFF; // Company ID byte 1
    manufacture_data += (char)0xFF; // Company ID byte 2

    // Create the characteristic 
    p_advertiser_characteristic = p_advertiser_service->createCharacteristic(
        "beb5483e-36e1-4688-b7f5-ea07361b26a8",
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );
    p_advertiser_auth_characteristic = p_advertiser_service->createCharacteristic(
        "aeb5483e-36e1-4688-b7f5-ea07361b26a8", // auth UUID
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );

    // Link our Callback class to the characteristic
    p_advertiser->setCallbacks(new advertiser_system_call_backs());
    p_advertiser_characteristic->setCallbacks(new advertiser_interaction_callbacks());
    p_advertiser_auth_characteristic->setCallbacks(new advertiser_authentication_callbacks()); // attach the authenication behavioral code to the characterstics

    p_advertiser_service->start(); // Finish creating the service
    
    p_advertise = BLEDevice::getAdvertising(); // gets adveritising pointer from device so we can customize and start it

    // create and attach the data to attatch to advertising data
    advertising_data.setManufacturerData(manufacture_data); // attach the data to advertising data
    p_advertise->setAdvertisementData(advertising_data); // attach the adveritising data to advertise pointer
    p_advertise->setScanResponse(true); // "bonus data to send in response to active scanning, can be used to send more data about the device since advertising data has a strict size limit"
}   

void send_advertising_signal(){
    p_advertise->start();
    Serial.println("Started advertising...");
}

void stop_advertising_signal(){
    p_advertise->stop();
    Serial.println("Stopped advertising...");
}

// determines what happens data is received from connected devices
void advertiser_interaction_callbacks::onWrite(BLECharacteristic *self_characterstic) {
      if (allowed_connection){
        std::string value = self_characterstic->getValue();
        Serial.print("Received value: ");
        Serial.println(value.c_str());

        // sends message to AWS IOT CORE to then be processed by Lambda and ChatGPT
        send_message_to_aws(String(value.c_str()), CUR_MQTT_TOPIC_SUB, CUR_MQTT_TOPIC_PUB); 
      }
};

void advertiser_system_call_backs::onConnect(BLEServer* self) {
    Serial.println("Device attempting to connect, sending authentication challenge!");      

    // Generate a new random nonce
    nonce = random(1, 255);
    // Write it to the auth characteristic
    p_advertiser_auth_characteristic->setValue(&nonce, 1); // setting the value of the auth char so client can grab it and sent the answer
    Serial.print("Nonce sent to client: ");
    Serial.println(nonce);

    Serial.println("closing advertising so no other devices can connect...");
    BLEDevice::getAdvertising()->stop(); // stop advertising so other devices cant connect while one is already connected
};

void advertiser_system_call_backs::onDisconnect(BLEServer* self) {
    Serial.println("Device disconnected... 🔌");
      
    allowed_connection = false;  // reset auth
    // This is the key: tell the ESP32 to start advertising again
    BLEDevice::startAdvertising();
    Serial.println("Restarted advertising!");
};


// advertiser authentication callback to verify that the connected device is of own devices
void advertiser_authentication_callbacks::onWrite(BLECharacteristic *self_characterstic) {
    std::string value = self_characterstic->getValue();
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
BLEScan* p_scanner;
bool found_device = false;
// This function handles what happens when a device is found
void scanner_scan_callbacks::onResult(BLEAdvertisedDevice advertisedDevice) {      
    // Print the basic info: Name, Address, and Signal Strength (RSSI)
    Serial.printf("Found Device: %s \n", advertisedDevice.toString().c_str());
    if (advertisedDevice.haveManufacturerData()){ 
        std::string scanned_device_manufacture_data = advertisedDevice.getManufacturerData();
        uint8_t companyID0 = (uint8_t)scanned_device_manufacture_data[0];
        uint8_t companyID1 = (uint8_t)scanned_device_manufacture_data[1];
        // Check for correct manufacture data and stop scanning and connect to it
        if (companyID0 == 0xFF && companyID1 == 0xFF) {
            found_device = true;
            Serial.printf("attempting to connect to device: %s\n", advertisedDevice.toString().c_str());
            Serial.println();
            BLEDevice::getScan()->stop();
            target_device = new BLEAdvertisedDevice(advertisedDevice);
            connect_to_server(target_device);
        }    
    }
};

// sets up the device to be able to scan and connect to other BLE devices as a client, WARNING, ALWAYS SET UP ADVERTISING MODE FIRST BEFORE CALLING THIS FUNCTION, 
void setup_scanning_mode(){
    Serial.println("Setting up scanning mode...");
    p_scanner = BLEDevice::getScan();
    p_scanner->setAdvertisedDeviceCallbacks(new scanner_scan_callbacks()); // attaches scan callback to handle what happens when a device is found

    p_scanner->setActiveScan(true); // Active scan gathers more data (like names) but uses more power
    p_scanner->setInterval(100);
    p_scanner->setWindow(99);
}

//attempts to connect to the server given the myDevice
void connect_to_server(BLEAdvertisedDevice* target_device){
    Serial.println("Forming a connection to the target device...");
    BLEClient* p_connector = BLEDevice::createClient();
    if(p_connector->connect(target_device)) { // Connect to the remote BLE Servers
        Serial.println("Connected to the target device!");
    } else {
        Serial.println("Failed to connect to the target device.");
        found_device = false; // reset found device so it can try to find and connect again
    }
}

// an functiont that should be repeatdely ran to check for switching between advertising and scanning mode every few seconds, this is to ensure that both devices can find each other and connect even if they start up at different times, or if the connection drops and they need to find each other again
void check_dual_mode(){
    if (!found_device && millis() - lastSwitchTime >= nextInterval) {
        // 1. Reset the timer
        lastSwitchTime = millis();
        
        // 2. Pick a new random interval for the NEXT switch (e.g., 2-5 seconds)
        nextInterval = 2000 + random(0, 3000); 
        
        // scanner mode
        if (current_state == 1 || current_state == 0) { // if currently scanning or idle, switch to advertising
            Serial.println("Current Mode: SCANs/IDLE, Switching to ADVERTISING");
            p_scanner->stop(); // stop scanning so it can start advertising without issues
            p_scanner->clearResults(); // clear results to reset found devices so it can find again in the future if needed
            send_advertising_signal();
            current_state = 2;
        } elif (current_state == 2 && p_advertise->isAdvertising()){ // if currently advertising, switch to scanning
            Serial.println("Current Mode: ADVERTISING, Switching to SCANNING");
            stop_advertising_signal();
            // Logging
            Serial.println("Starting scanner...");
            p_scanner->start(2, false);
            Serial.println("Scanner started!");

            current_state = 1;
        }
        Serial.print("Next switch in: ");
        Serial.println(nextInterval);
    } 
}