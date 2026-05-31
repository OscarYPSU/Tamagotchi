// ----------------
// BLE SET UP
// ----------------
#include "BLE.h"
#define SECRET 0x5A // secret for handshake protocl

bool allowed_connection = false; // only true after handshake that allows connected device to start sending data
uint8_t nonce; // random nonce for authentication
std::string DEVICE_NAME; // specific device name to initilize BLE with, can be used for scanning purposes to identify which device is which

NimBLECharacteristic *p_advertiser_characteristic; // for sesnding data across connected devic
NimBLECharacteristic *p_advertiser_auth_characteristic; // for authentication process
NimBLECharacteristic *p_web_or_mcu_characterstic; // for communication with web or MCU

NimBLEAdvertising *p_advertise;
NimBLEAdvertisementData advertising_data; // the advertising signal data so we can change and update it with information
NimBLEAdvertisedDevice* target_device; 
int current_state = 2; // 0 for being advertiser/server, 1 for being scanner/client, 2 not connected 
long advertiser_scanner_interval = 2; // start at 2 when no connection is made then increase to 10 after connection is made to allow for faster communication between connected devices
// state variables
bool has_setup_adveriser_mode = false;
bool has_setup_scanning_mode = false;
bool disonnected_from_client = false; // flag to indicate if we have been disconnected from a client, used to trigger restarting advertising and scanning

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
    p_web_or_mcu_characterstic = p_advertiser_service->createCharacteristic(
        "beb5483e-36e1-4688-b7f5-ea07361b26a9", // web/MCU communication UUID
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
    );

    // Link our Callback class to the characteristic
    p_advertiser->setCallbacks(new advertiser_system_call_backs());
    p_advertiser_characteristic->setCallbacks(new advertiser_interaction_callbacks());
    p_advertiser_auth_characteristic->setCallbacks(new advertiser_authentication_callbacks()); // attach the authenication behavioral code to the characterstics
    p_web_or_mcu_characterstic->setCallbacks(new advertiser_web_or_mcu_callbacks()); // attach the web/MCU behavioral code to the characterstics

    p_advertiser_service->start(); // Finish creating the service
    
    p_advertise = NimBLEDevice::getAdvertising(); // NimBLE version

    // create and attach the data to attatch to advertising data
    p_advertise->setManufacturerData(manufacture_data); // NimBLE handles this directly on the advertiser object usually
    p_advertise->setScanResponse(true); 
    p_advertise->addServiceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");

    Serial.println("finished setting up advertising...");
    has_setup_adveriser_mode = true;
}   
 
// to be called if advertising mode is set up but needs to start advertising again
void start_advertising() {
    if (p_advertise != nullptr) {
        p_advertise->start();
        Serial.println("Started advertising...");
    } else {
        Serial.println("Error: Advertiser not initialized.");
    }
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
    found_device = false; // reset found device state so we can find and connect to devices again
    have_message_from_connector = false; // reset message flag
    message_from_connector = ""; // reset message variable
    disonnected_from_client = true; // set flag to indicate we have been disconnected from a client
};

bool need_web_or_mcu_identification = false; // flag to indicate if we need to identify as web or MCU
// advertiser authentication callback to verify that the connected device is of own devices
void advertiser_authentication_callbacks::onWrite(NimBLECharacteristic *self_characteristic) {
    std::string value = self_characteristic->getValue();
    if (value.length() < 1) return; // not valid
    uint8_t response = (uint8_t)value[0]; // process value into response data form (unsigned int 8)
    if (response == (nonce ^ SECRET)) {  // ✅ verify XOR  authentication
        allowed_connection = true;
        need_web_or_mcu_identification = true; // after handshake is complete, we need to identify if the connected device is a web client or an MCU so we set this flag to trigger the identification process in the main loop
        Serial.println("Handshake success ✅, sending notification to connected device to identify as web or MCU...");
    } else {
        allowed_connection = false;
        Serial.println("Handshake failed ❌");
    }
};

int web_or_mcu; // 0 for web, 1 for MCU
void advertiser_web_or_mcu_callbacks::onWrite(NimBLECharacteristic *self_characteristic) { // differentiate between mcu or web as connected device
    if (allowed_connection){
        std::string value = self_characteristic->getValue();
        Serial.print("Received value on web/MCU characteristic: ");
        Serial.println(value.c_str());
        if (value == "0") {
            web_or_mcu = 0;
            Serial.println("Connected device identified as Web.");
        } else if (value == "1") {
            web_or_mcu = 1;
            Serial.println("Connected device identified as MCU.");
        } else {
            Serial.println("Received unrecognized value on web/MCU characteristic.");
        }
    } else {
        Serial.println("Received write on web/MCU characteristic but handshake not complete, ignoring...");
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

void notifiy_for_web_or_mcu(){
    if (connected && allowed_connection) {
        p_web_or_mcu_characterstic->setValue("need to know if web or mcu"); 
        p_web_or_mcu_characterstic->notify(); // notify the connected client that the value has changed so it can read it
        need_web_or_mcu_identification = false; // reset the flag so we don't keep trying to identify as web or mcu
        Serial.println("Notified connected device to identify as web or MCU.");
    } else {
        Serial.println("Cannot notify connected device to identify as web or MCU, no device connected or handshake not complete.");
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
NimBLERemoteCharacteristic  *target_web_or_mcu_characterstic; // for differentiating between web and MCU clients

// --- CLIENT CALLBACKS ---

// Define the callback function that runs when data arrives
void notify_call_backs(BLERemoteCharacteristic* target_characterstic, uint8_t* data, size_t length, bool is_notify) {
    std::string value((char*)data, length);
    Serial.println("Received from Advertiser: " + String(value.c_str()));
    send_message_to_aws(String(value.c_str()), CUR_DEVICE_PERSONALITY, CUR_MQTT_TOPIC_SUB, CUR_MQTT_TOPIC_PUB); // forward the message we received from the advertiser to AWS IOT CORE to then be processed by Lambda and ChatGPT
}

void notify_call_backs_web_or_mcu(BLERemoteCharacteristic* target_characterstic, uint8_t* data, size_t length, bool is_notify) {
    authenticate_as_mcu(); // after handshake is complete, we notify the connected device to identify as web or MCU, we do this because we cannot call this directly from the onConnect callback function since it needs to read and write to the characteristic and that can only be done after the connection is fully established and the characteristic is properly set up, which happens in the main loop after the onConnect callback is called.
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
            target_web_or_mcu_characterstic = target_remote_service->getCharacteristic("beb5483e-36e1-4688-b7f5-ea07361b26a9"); // also grab the web/MCU communication characteristic so we can use it later to differentiate between a web client and an MCU as the connected device
            if (target_auth_characteristic && target_characterstic && target_web_or_mcu_characterstic) {
                if(target_characterstic->canNotify() && target_web_or_mcu_characterstic->canNotify()) {
                    target_characterstic->subscribe(true, notify_call_backs); // subscribe to the  characteristic to receive messages  when the server sends us the messages after handshake is complete, we do this because we cannot call notify_call_backs directly from this callback function since it needs to read and write to the characteristic and that can only be done after the connection is fully established and the characteristic is properly set up, which happens in the main loop after this callback is called.  
                    target_web_or_mcu_characterstic->subscribe(true, notify_call_backs_web_or_mcu); // subscribe to the web/MCU characteristic to receive the notification to identify as web or MCU, we do this because we cannot call notify_call_backs_web_or_mcu directly from this callback function since it needs to read and write to the characteristic and that can only be done after the connection is fully established and the characteristic is properly set up, which happens in the main loop after this callback is called.
                }
                Serial.println("Found authentication and sending and web/MCU data characteristic! Ready to perform handshake.");
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
        found_device = false; // reset found device state so we can find and connect to devices again
        disonnected_from_client = true; // set flag to indicate we have been disconnected from a client
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
    Serial.println("finished setting up scanning mode...");
    has_setup_scanning_mode = true;
}

// to be called if scanning mode is set up but needs to start scanning again
void start_scanning(){
    if (p_scanner != nullptr) {
        p_scanner->start(advertiser_scanner_interval, false);
        Serial.println("Started scanning mode...");
    } else {
        Serial.println("Error: Scanner not initialized.");
    }
}

//atempts to connect to the server  
bool connect_to_server(NimBLEAdvertisedDevice* target_device){
    Serial.println("Forming a connection to the target device...");

    // 1. Create the client
    NimBLEClient* p_connector = NimBLEDevice::createClient();
    p_connector->setClientCallbacks(new client_callbacks(), false); // connect the callbacks tot he client

    if(p_connector->connect(target_device)) { 
        Serial.println("Connected to the target device! ✅");
        return true;
    } else {
        Serial.println("Failed to connect. Retrying next scan cycle... ❌");
        found_device = false; 
    }
    return false;
}

// performs the handshake writing it back to the server
void perform_handshake(){
    // Step 1: read nonce from server
    std::string value = target_auth_characteristic->readValue();
    if (value.length() < 1) return;

    uint8_t nonce = (uint8_t)value[0]; // process info to correct data type = unsigned 8 int 
    Serial.print("Nonce received: ");
    Serial.println(nonce);

    // Step 2: compute response
    uint8_t response = nonce ^ SECRET;

    target_auth_characteristic->writeValue(&response, 1,false);
    Serial.print("Response sent: ");
    Serial.println(response);

    need_handshake = false; // reset handshake state so it doesnt keep trying to perform handshake
}

void authenticate_as_mcu(){
    if(need_handshake == false && target_web_or_mcu_characterstic != nullptr){
        Serial.println("Authenticating as MCUs to the advertiser...");
        String mcu_identifier = "1"; // let's say "1" means MCU and "0" means web client, we will send this after the handshake so the advertiser can differentiate between a web client and an MCU
        target_web_or_mcu_characterstic->writeValue((uint8_t*)mcu_identifier.c_str(), mcu_identifier.length(), false); // write the identifier to the web/MCU characteristic so the advertiser can differentiate between a web client and an MCU
        Serial.println("Sent MCU identification to advertiser.");

        String test_response = "Hello!";
        master_send_data(test_response); // simple test for sending data
    }
}

void send_data_to_server(String& message){
    if (connected){
        target_characterstic->writeValue((uint8_t*)message.c_str(), message.length(), false);     
        Serial.println("Sent: " + message);
    } else {
        Serial.println("Cannot send message, no device connected.");
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