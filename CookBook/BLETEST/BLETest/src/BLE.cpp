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

// -----------
// BLE SETUP for advertiser mode
// -----------
void setup_advertising_mode(){
    Serial.println("Setting up advertising mode...");
    BLEDevice::init(DEVICE_NAME);
    BLEServer *p_advertiser = BLEDevice::createServer();
    BLEService *p_advertiser_service = p_advertiser->createService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");

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
    std::string manufacturerData = "Oscar's BLE Device"; // this is the data that will be broadcasted in the advertising signal, can be used to identify the device or for other purposes
    advertising_data.setManufacturerData(manufacturerData); // attach the data to advertising data
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



