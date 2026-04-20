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
        CUR_MQTT_TOPIC_PUB = DEVICE_1_MQTT_TOPIC_PUB;
        CUR_MQTT_TOPIC_SUB = DEVICE_1_MQTT_TOPIC_SUB;
        DEVICE_NAME_AWS = "Device1";
    } else {
        CUR_DEVICE_CERT = DEVICE_2_CERT;
        CUR_PRIVATE_KEY  = DEVICE_2_PRIVATE_KEY;
        CUR_MQTT_TOPIC_PUB = DEVICE_2_MQTT_TOPIC_PUB;
        CUR_MQTT_TOPIC_SUB = DEVICE_2_MQTT_TOPIC_SUB;
        DEVICE_NAME_AWS = "Device2";
    }
    // Now proceed to connectAWS() using these selected variables
    connectAWS(CUR_DEVICE_CERT, CUR_PRIVATE_KEY, CUR_MQTT_TOPIC_SUB, DEVICE_NAME_AWS);

    // -----------
    // BLE SETUP for advertiser mode
    // -----------
    setup_advertising_mode();
    setup_scanning_mode();
}

const unsigned long MESSAGE_DELAY = 5000; // 5 seconds delay between forwarding messages to AWS, this is to prevent spamming AWS with too many messages in a short period of time which can cause issues with rate limits and also gives some time for the device to process the previous message and response before sending another one
unsigned long last_message_time = 0; // keeping track of the last time we processed a message and send to aws

void loop(){
    // ensure connection stays alive
    if (!MQTT_client.connected()) {
        connectAWS(CUR_DEVICE_CERT, CUR_PRIVATE_KEY, CUR_MQTT_TOPIC_SUB, DEVICE_NAME_AWS);
    } 
    MQTT_client.loop(); // checks for incoming messages and keeps the connection alive, this should be called regularly in the main loop

    if (found_device && !connected) {
        connect_to_server(target_device);
    }

    if (need_handshake && connected) {
        perform_handshake();
    }

    if (have_message_from_connector) {
        if (millis() - last_message_time >= MESSAGE_DELAY) { // check if enough time has passed since the last message was sent to AWS
            last_message_time = millis(); // update the last message time
            send_message_to_aws(message_from_connector, CUR_MQTT_TOPIC_SUB, CUR_MQTT_TOPIC_PUB); // send message to aws
            have_message_from_connector = false; // reset the flag
        } else {
        }
    }
}