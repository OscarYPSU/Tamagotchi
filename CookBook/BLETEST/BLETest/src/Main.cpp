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
        CUR_MQTT_PERSONALITY_TOPIC_PUB = DEVICE_1_MQTT_PERSONALITY_TOPIC_PUB;
        CUR_MQTT_PERSONALITY_TOPIC_SUB = DEVICE_1_MQTT_PERSONALITY_TOPIC_SUB;
        DEVICE_NAME_AWS = "Device1";
    } else {
        CUR_DEVICE_CERT = DEVICE_2_CERT;
        CUR_PRIVATE_KEY  = DEVICE_2_PRIVATE_KEY;
        CUR_MQTT_TOPIC_PUB = DEVICE_2_MQTT_TOPIC_PUB;
        CUR_MQTT_TOPIC_SUB = DEVICE_2_MQTT_TOPIC_SUB;
        CUR_MQTT_PERSONALITY_TOPIC_PUB = DEVICE_2_MQTT_PERSONALITY_TOPIC_PUB;
        CUR_MQTT_PERSONALITY_TOPIC_SUB = DEVICE_2_MQTT_PERSONALITY_TOPIC_SUB;
        DEVICE_NAME_AWS = "Device2";
    }
    // Now proceed to connectAWS() using these selected variables
    connectAWS(CUR_DEVICE_CERT, CUR_PRIVATE_KEY, CUR_MQTT_TOPIC_SUB, CUR_MQTT_PERSONALITY_TOPIC_SUB, DEVICE_NAME_AWS);
    // once connected, set up the personality of the device
    set_up_personality(device_mac, CUR_MQTT_PERSONALITY_TOPIC_SUB, CUR_MQTT_PERSONALITY_TOPIC_PUB);
}

const unsigned long MESSAGE_DELAY = 15000; // 15 seconds delay between forwarding messages to AWS, this is to prevent spamming AWS with too many messages in a short period of time which can cause issues with rate limits and also gives some time for the device to process the previous message and response before sending another one
unsigned long last_message_time = 0; // keeping track of the last time we processed a message and send to aws

void loop(){

    static unsigned long last_loop_heartbeat = 0;
    const unsigned long LOOP_HEARTBEAT_INTERVAL = 5000;

    if (millis() - last_loop_heartbeat >= LOOP_HEARTBEAT_INTERVAL) {
        last_loop_heartbeat = millis();
        Serial.println("Main loop is running...");
    }


    // ensure connection stays alive
    if (!MQTT_client.connected()) {
        connectAWS(CUR_DEVICE_CERT, CUR_PRIVATE_KEY, CUR_MQTT_TOPIC_SUB, CUR_MQTT_PERSONALITY_TOPIC_SUB, DEVICE_NAME_AWS);
    } 
    MQTT_client.loop(); // checks for incoming messages and keeps the connection alive, this should be called regularly in the main loop
    
    if (received_personality_from_aws && !has_setup_adveriser_mode && !has_setup_scanning_mode) { // we wait to receive the personality data from AWS before setting up BLE so that we can use the personality data in the BLE setup if needed and also to ensure that we don't start accepting connections or sending messages before we have the personality set up in AWS Lambda which can cause issues with how the messages are processed and responded to by Lambda and ChatGPT
        setup_advertising_mode();
        setup_scanning_mode();
    }

    if (disonnected_from_client) {
        disonnected_from_client = false;
        start_advertising();
        start_scanning();
        Serial.println("Resumed advertising... 📢");
        Serial.println("Resumed scanning... 🔍");
    }

    if (found_device && !connected && received_personality_from_aws) {
        connect_to_server(target_device);
    }

    if (need_handshake && connected && received_personality_from_aws) { // for now we need the check from receiving personality because perform hadnshake sends message to aws openai lambda for now for tesing
        perform_handshake(); 
    }

    if (have_message_from_connector) {
        if (millis() - last_message_time >= MESSAGE_DELAY) { // check if enough time has passed since the last message was sent to AWS
            last_message_time = millis(); // update the last message time
            send_message_to_aws(message_from_connector, CUR_DEVICE_PERSONALITY, CUR_MQTT_TOPIC_SUB, CUR_MQTT_TOPIC_PUB); // send message to aws
            have_message_from_connector = false; // reset the flag
        } else {
        }
    }
}