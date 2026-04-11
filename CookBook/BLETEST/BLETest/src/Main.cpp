#include "AWS_CONFIGS.h" // contains the device certs and private keys for both devices, as well as the mqtt topics for each device
#include "BLE.h" // contains the BLE setup and callbacks for device interaction

// configs for appropiate device using MAC addrress for AWS IOT Core
void setup() {
    Serial.begin(115200); 
    connectWiFi(); // connect to wifi first to get the mac address and then use it to determine which certs and keys to use for AWS IOT CORE connection
    String device_mac = WiFi.macAddress();
    // prints out the mac address of the device for scanning purposes
    Serial.print("Device MAC Address: ");
    Serial.println(device_mac);
    
    // -----------
    // Sets up configs for AWS IOT then Connects to it
    // -----------
    if (device_mac == "1C:DB:D4:AE:D4:F4") { 
        CUR_DEVICE_CERT = DEVICE_1_CERT;
        CUR_PRIVATE_KEY  = DEVICE_1_PRIVATE_KEY;
        CUR_MQTT_TOPIC_SUB = DEVICE_1_MQTT_TOPIC_PUB;
        CUR_MQTT_TOPIC_PUB = DEVICE_1_MQTT_TOPIC_SUB;
        DEVICE_NAME = "ESP32-S3-Device1";
    } else {
        CUR_DEVICE_CERT = DEVICE_2_CERT;
        CUR_PRIVATE_KEY  = DEVICE_2_PRIVATE_KEY;
        CUR_MQTT_TOPIC_SUB = DEVICE_2_MQTT_TOPIC_PUB;
        CUR_MQTT_TOPIC_PUB = DEVICE_2_MQTT_TOPIC_SUB;
        DEVICE_NAME = "ESP32-S3-Device2";
    }
    // Now proceed to connectAWS() using these selected variables
    connectAWS(CUR_DEVICE_CERT, CUR_PRIVATE_KEY, CUR_MQTT_TOPIC_SUB);

    // -----------
    // BLE SETUP for advertiser mode
    // -----------
    setup_advertising_mode();

    send_advertising_signal(); // start advertising so other device can find and connect to it
}

void loop(){
    // ensure connection stays alive
    if (!MQTT_client.connected()) {
        connectAWS(CUR_DEVICE_CERT, CUR_PRIVATE_KEY, CUR_MQTT_TOPIC_SUB);
    }
    MQTT_client.loop();
}