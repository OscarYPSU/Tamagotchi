#include <NimBLEDevice.h>

// Use the same UUIDs on both devices
#define SERVICE_UUID        "ABCD"
#define CHARACTERISTIC_UUID "1234"

// State variables
static bool connected = false;
static bool doConnect = false;
static NimBLEAdvertisedDevice* targetDevice;

// --- SERVER CALLBACKS ---
// Handles events when a remote device connects to US
class MyServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        connected = true;
        Serial.println(">>> Peer connected to us (We are Peripheral/Server)");
    };
    void onDisconnect(NimBLEServer* pServer) {
        connected = false;
        Serial.println(">>> Peer disconnected. Resuming advertising...");
        NimBLEDevice::getAdvertising()->start();
    }
};

// --- CLIENT CALLBACKS ---
// Handles events when WE connect to a remote device
class MyClientCallbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pClient) {
        Serial.println(">>> We connected to peer (We are Central/Client)");
    }
    void onDisconnect(NimBLEClient* pClient) {
        connected = false;
        Serial.println(">>> Client disconnected. Resuming scan...");
        NimBLEDevice::getScan()->start(0, false);
    }
};

// --- SCAN CALLBACKS ---
// Fires when a device is found during scanning
class MyAdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
        if (advertisedDevice->isAdvertisingService(NimBLEUUID(SERVICE_UUID))) {
            Serial.println("Target Found! Checking connection state...");
            
            // Only trigger a connection if we aren't already connected to them
            if (!connected) {
                NimBLEDevice::getScan()->stop();
                targetDevice = advertisedDevice;
                doConnect = true;
            }
        }
    }
};

void setup() {
    Serial.begin(115200);
    Serial.println("Starting NimBLE Dual-Role Node...");

    // Initialize NimBLE
    NimBLEDevice::init("NimBLE-Peer");

    // 1. SETUP SERVER (Peripheral Role)
    NimBLEServer *pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    
    NimBLEService *pService = pServer->createService(SERVICE_UUID);
    NimBLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                               CHARACTERISTIC_UUID,
                                               NIMBLE_PROPERTY::READ | 
                                               NIMBLE_PROPERTY::WRITE | 
                                               NIMBLE_PROPERTY::NOTIFY
                                             );
    pCharacteristic->setValue("Hello Peer!");
    pService->start();

    // 2. SETUP ADVERTISING
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();
    Serial.println("Advertising started.");

    // 3. SETUP SCANNING (Central Role)
    NimBLEScan* pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(0, false); 
    Serial.println("Scanning started.");
}

void loop() {
    // If a target was found and we aren't already connected, try to connect
    if (doConnect) {
        doConnect = false;
        
        // Double check connection status to prevent race condition crashes
        if (!connected) {
            NimBLEClient* pClient = NimBLEDevice::createClient();
            pClient->setClientCallbacks(new MyClientCallbacks(), false);

            Serial.println("Initiating connection...");
            if (pClient->connect(targetDevice)) {
                connected = true;
                Serial.println("Connection Successful!");
            } else {
                Serial.println("Failed to connect. Restarting Scan.");
                NimBLEDevice::getScan()->start(0, false);
            }
        }
    }
    
    // Heartbeat logic or data processing
    if (connected) {
        // You can add logic here to write to characteristics
    }
    
    delay(500);
}