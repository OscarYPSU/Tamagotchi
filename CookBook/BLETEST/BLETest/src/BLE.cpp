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
NimBLEAdvertisedDevice* target_device; // Swapped to NimBLE
int current_state; // 0 for being advertiser/server, 1 for being scanner/client

// state variables
bool has_setup_adveriser_mode = false;
bool has_setup_scanning_mode = false;

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
    has_setup_adveriser_mode = true;
}   


bool have_message_from_connector = false; // flag to indicate if we have received a message from the connected device that we want to forward to AWS
String message_from_connector; // variable to store the message we want to forward to AWS

// determines what happens data is received from connected devices
void advertiser_interaction_callbacks::onWrite(NimBLECharacteristic *self_characteristic) {
      if (allowed_connection){
        std::string value = self_characteristic->getValue();
        Serial.print("Received value: ");
        Serial.println(value.c_str());
        
        message_from_connector = String(value.c_str()); // store the message in a global variable so we can forward it to AWS in the main loop, we do this because we cannot call send_message_to_aws directly from this callback function since it needs to read and write to the characteristic and that can only be done after the connection is fully established and the characteristic is properly set up, which happens in the main loop after this callback is called.
        have_message_from_connector = true; // set the flag to indicate we have a message to forward to AWS

        // sends message to AWS IOT CORE to then be processed by Lambda and ChatGPT
        // send_message_to_aws(String(value.c_str()), CUR_MQTT_TOPIC_SUB, CUR_MQTT_TOPIC_PUB); we are moving this to main loop to handle for the implemenatio of a delay logic
      }
};

// Updated signature with ble_gap_conn_desc* desc
void advertiser_system_call_backs::onConnect(NimBLEServer* self, ble_gap_conn_desc* desc) {
    Serial.println("Device attempting to connect, setting state as the server/advertiser, sending authentication challenge!");      
    connected = true; // we are now connected, but not necessarily authenticated yet, we will use this variable to keep track of the connection state and avoid multiple connections at onceq
    current_state = 0; // we are now a server/advertiser, we will use this variable to keep track of the state of the device and avoid multiple connections at once and also to know when to perform certain actions that are specific to being a client vs being a server/advertiser

    // stop scanmning and advertising since we are now connected to a device, we will restart advertising if the device disconnects
    p_advertise->stop(); // stop advertising since we are now connected to a device, we will restart advertising if the device disconnects
    p_scanner->stop(); // stop scanning since we are now connected to a device, we will restart scanning if the device disconnects
    
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
    p_advertise->start(); // restart advertising so other devices can find and connect to us
    p_scanner->start(2, false); // restart scanning so we can find other devices while waiting for a new connection, we will stop scanning again when a device connects
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

void send_message_to_connected_device(std::string& message) {
    if (connected && allowed_connection) {
        p_advertiser_characteristic->setValue(message); // set the value of the characteristic to the message we want to send
        p_advertiser_characteristic->notify(); // notify the connected client that the value has changed so it can read it
        Serial.println("Sent message to connected device: " + String(message.c_str()));
    } else {
        Serial.println("Cannot send message, no device connected or handshake not complete.");
    }
}

// ------------
// BLE scanning and connecting to other device as a client
// ------------
NimBLEScan* p_scanner;
bool found_device = false;
bool connected = false;
bool need_handshake = false;
NimBLERemoteCharacteristic  *target_auth_characteristic; // for authentication process
NimBLERemoteCharacteristic  *target_characterstic; // for sending data to server after handshake is complete, we store it here after we discover it in the onConnect callback of the client so we can use it later in the main loop to send data to the server after the handshake is complete

// --- CLIENT CALLBACKS ---

// Define the callback function that runs when data arrives
void notify_call_backs(BLERemoteCharacteristic* target_characterstic, uint8_t* data, size_t length, bool is_notify) {
    std::string value((char*)data, length);
    Serial.println("Received from Advertiser: " + String(value.c_str()));
    send_message_to_aws(String(value.c_str()), CUR_DEVICE_PERSONALITY, CUR_MQTT_TOPIC_SUB, CUR_MQTT_TOPIC_PUB); // forward the message we received from the advertiser to AWS IOT CORE to then be processed by Lambda and ChatGPT

}
// Handles events when WE connect to a remote device
class client_callbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pClient) {
        Serial.println("setting state as a client");
        need_handshake = true; // we will perform the handshake in the main loop, we do this because we cannot call perform_handshake directly from this callback function since it needs to read and write to the characteristic and that can only be done after the connection is fully established and the characteristic is properly set up, which happens in the main loop after this callback is called.
        connected = true;
        current_state = 1; // we are now a client, we will use this variable to keep track of the state of the device and avoid multiple connections at once and also to know when to perform certain actions that are specific to being a client vs being a server/advertiser

        // stop scanmning and advertising since we are now connected to a device, we will restart advertising if the device disconnects
        p_advertise->stop(); // stop advertising since we are now connected to a device
        p_scanner->stop(); // stop scanning since we are now connected to a device

        // After connecting, we need to find the correct characteristic for authentication
        NimBLERemoteService* target_remote_service = pClient->getService("4fafc201-1fb5-459e-8fcc-c5c9c331914b"); // grab the service that we know has the authentication characteristic on it, we need to do this because we cannot directly grab the characteristic without first grabbing the service it is on, and we also want to verify that the service exists before trying to grab the characteristic
        if (target_remote_service) {
            target_auth_characteristic = target_remote_service->getCharacteristic("aeb5483e-36e1-4688-b7f5-ea07361b26a8"); // grab the authentication characteristic so we can perform the handshake in the main loop, we do this because we cannot call perform_handshake directly from this callback function since it needs to read and write to the characteristic and that can only be done after the connection is fully established and the characteristic is properly set up, which happens in the main loop after this callback is called.
            target_characterstic = target_remote_service->getCharacteristic("beb5483e-36e1-4688-b7f5-ea07361b26a8"); // also grab the data characteristic so we can use it later to send data to the server after handshake is complete
            if (target_auth_characteristic && target_characterstic) {
                if(target_characterstic->canNotify()) {
                    target_characterstic->subscribe(true, notify_call_backs); // subscribe to the  characteristic to receive messages  when the server sends us the messages after handshake is complete, we do this because we cannot call notify_call_backs directly from this callback function since it needs to read and write to the characteristic and that can only be done after the connection is fully established and the characteristic is properly set up, which happens in the main loop after this callback is called.  
                }
                Serial.println("Found authentication and sending data characteristic! Ready to perform handshake.");
            } else {
                Serial.println("Failed to find one or both characteristics. Disconnecting...");
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

        // restart scanning and advertising so we can find other devices while waiting for a new connection, we will stop scanning again when a device connects
        p_scanner->start(2, false);
        p_advertise->start();
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
                target_device = advertisedDevice; // store the target device so we can connect to it in the main loop, we do this because we cannot call connect directly from this callback function
            } else {
                Serial.println("I have the lower MAC. I will stay in 'Server mode' and wait for them.");
                found_device = false; // we will stay in advertiser mode and wait for the other device to connect to us, we do this because we cannot call setup_advertising_mode directly from this callback function since it needs to set up the whole advertising service and characteristics which can only be done in the main loop after this callback is called.
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

    // '2' means scan for 2 seconds, 'false' means non-blocking
    p_scanner->setActiveScan(true);
    p_scanner->start(2, false); // CANNOT SCAN FOR EVER OR IT WILL BE BLOCKING AND PREVENT OTHER CODE FROM RUNNING, MUST BE NON-BLOCKING AND STARTED IN THE MAIN LOOP AFTER THIS SETUP FUNCTION IS CALLED
    Serial.println("Started scanning mode...");
    has_setup_scanning_mode = true;
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

    String test_response = "Hello!";
    master_send_data(test_response); // simple test for sending data
    need_handshake = false; // reset handshake state so it doesnt keep trying to perform handshake
}

void send_data_to_server(String& message){
    if (connected){
        target_characterstic->writeValue((uint8_t*)message.c_str(), message.length(), true);     
        Serial.println("Sent: " + message);
    }
}



// ------------
// MASTER functions
// -----------

void master_send_data(String& message){
    if (connected){
        if (current_state == 0) { // if we are a server/advertiser, we send the data to the connected client'
            // formats the message appropately and sends it to the connected client using the characteristic
            std::string formated_message = std::string(message.c_str()); // convert the String message to std::string so we can send it using the characteristic
            send_message_to_connected_device(formated_message);
        } else if (current_state == 1) { // if we are a client, we send the data to the server
            send_data_to_server(message);
        } else {
            Serial.println("Error: Invalid state. Cannot send data.");
        }
    }
}