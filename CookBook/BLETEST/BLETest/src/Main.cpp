#include "AWS_CONFIGS.h" // contains the device certs and private keys for both devices, as well as the mqtt topics for each device
#include "BLE.h" // contains the BLE setup and callbacks for device interaction
#include "device_state.h"
#include "NVS.h" // contains the NVS setup and functions for saving and reading from non volatile storage
#include "AP.h"
#include "Main.h"

const char* state_tag = "state"; // tag for logging the device state

// configs for appropiate device using MAC address for AWS IOT Core
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
        CUR_MQTT_DEVICE_INFO_TOPIC_PUB = DEVICE_1_MQTT_DEVICE_INFO_PUB;
        CUR_MQTT_DEVICE_INFO_TOPIC_SUB = DEVICE_1_MQTT_DEVICE_INFO_SUB;
        DEVICE_NAME_AWS = "Device1";
        DEVICE_NAME = "ESP32-S3-Device1";
    } else {
        CUR_DEVICE_CERT = DEVICE_2_CERT;
        CUR_PRIVATE_KEY  = DEVICE_2_PRIVATE_KEY;
        CUR_MQTT_TOPIC_PUB = DEVICE_2_MQTT_TOPIC_PUB;
        CUR_MQTT_TOPIC_SUB = DEVICE_2_MQTT_TOPIC_SUB;
        CUR_MQTT_PERSONALITY_TOPIC_PUB = DEVICE_2_MQTT_PERSONALITY_TOPIC_PUB;
        CUR_MQTT_PERSONALITY_TOPIC_SUB = DEVICE_2_MQTT_PERSONALITY_TOPIC_SUB;
        CUR_MQTT_DEVICE_INFO_TOPIC_PUB = DEVICE_2_MQTT_DEVICE_INFO_PUB;
        CUR_MQTT_DEVICE_INFO_TOPIC_SUB = DEVICE_2_MQTT_DEVICE_INFO_SUB;
        DEVICE_NAME_AWS = "Device2";
        DEVICE_NAME = "ESP32-S3-Device2";
    }
    // Now proceed to connectAWS() using these selected variables
    connectAWS(CUR_DEVICE_CERT, CUR_PRIVATE_KEY, CUR_MQTT_TOPIC_SUB, CUR_MQTT_PERSONALITY_TOPIC_SUB, DEVICE_NAME_AWS);
    // once connected, set up the personality of the device
    set_up_personality(device_mac, CUR_MQTT_PERSONALITY_TOPIC_SUB, CUR_MQTT_PERSONALITY_TOPIC_PUB);
   
};

const unsigned long MESSAGE_DELAY = 15000; // 15 seconds delay between forwarding messages to AWS, this is to prevent spamming AWS with too many messages in a short period of time which can cause issues with rate limits and also gives some time for the device to process the previous message and response before sending another one
unsigned long last_message_time = 0; // keeping track of the last time we processed a message and send to aws
// When a message arrives but we're still waiting for MESSAGE_DELAY, log the waiting message only once
bool waiting_message_logged = false;

void loop(){

    static unsigned long last_loop_heartbeat = 0;
    const unsigned long LOOP_HEARTBEAT_INTERVAL = 10000;

    // periodic logging to make sure loop is running and not stuck
    if (millis() - last_loop_heartbeat >= LOOP_HEARTBEAT_INTERVAL) {
        last_loop_heartbeat = millis();
        Serial.println("Main loop is running, current state: " + String(DEVICE_STATE));
    }

    // ensure connection stays alive
    if (!MQTT_client.connected()) {
        connectAWS(CUR_DEVICE_CERT, CUR_PRIVATE_KEY, CUR_MQTT_TOPIC_SUB, CUR_MQTT_PERSONALITY_TOPIC_SUB, DEVICE_NAME_AWS);
    } 
    MQTT_client.loop(); // checks for incoming messages and keeps the connection alive, this should be called regularly in the main loop
    
    // if device is disconnected from each other
    if(disonnected_from_client){
        Serial.println("Disconnected from client, restarting BLE setup...");
        DEVICE_STATE = START_BLE; // restart the BLE setup to start advertising and scanning again to find and connect to devices again
        disonnected_from_client = false; // reset the flag after handling the disconnection
    }


    switch(DEVICE_STATE){
        case CHECK_REGISTRATION:
            ESP_LOGI(state_tag, "Current state: CHECK_REGISTRATION");

            // checks for wifi credentials first (if saved in NVS)
            WIFI_SSID = read_from_nvs("wifi_credentials", "ssid");
            WIFI_PASSWORD = read_from_nvs("wifi_credentials", "password");

            // if wifi credentials isnt already in NVS, then set device as AP point to grab the wifi credentials from users
            if(WIFI_SSID == "None" || WIFI_PASSWORD == "None"){
                ESP_LOGI(state_tag, "Wifi credentials not found, switching to AP mode");
                DEVICE_STATE = SETTING_AP_MODE; 
                break;
            }

            check_for_registration(WiFi.macAddress(), CUR_MQTT_DEVICE_INFO_TOPIC_SUB, CUR_MQTT_DEVICE_INFO_TOPIC_PUB); // check if the device has been registered with the aws, if it is, automatically switches state to PERSONALITY_SET_UP. If not it switches state to NEED_REGISTRATION TO START THE REGISTRATRATION MODE
            DEVICE_STATE = WAITING_FOR_REGISTRATION;
            ESP_LOGI(state_tag, "Checking for device registration, switching to state: WAITING_FOR_REGISTRATION");
            break;
        
        // AP mode to grab wifi credentials and account details from user 
        case SETTING_AP_MODE:
            ESP_LOGI(state_tag, "Current state: SETTING_AP_MODE");
            setup_ap_mode(); // sets up the access point and web server to grab the wifi credentials and account details from the user
            start_ap_mode(); // starts the access point and web server to grab the wifi credentials and account details from the user
            DEVICE_STATE = REGRISTRATION_MODE; // switch to the registration mode to wait for the user to input the wifi credentials and account details through the web server
            ESP_LOGI(state_tag, "Switching to state: REGRISTRATION_MODE");
            break;
        
        case REGRISTRATION_MODE:
            // waiting for the user to input the wifi credentials and account details through the web server
            // no action here for the device, automatically handled when user gives an in put
            break;

        // setting up personality of mcu and also  before setting up BLE
        case SETTING_CONFIGURATION:
            if(received_personality_from_aws){
                ESP_LOGI(state_tag, "Received personality data from AWS, proceeding to set up BLE...");
                DEVICE_STATE = SETUP_BLE; // move to the next state to start BLE setup
            } else {
                ESP_LOGI(state_tag, "Waiting for personality data from AWS...");
            }

            break;
        //sets up BLE
        case SETUP_BLE:
            ESP_LOGI(state_tag, "Current state: SETUP_BLE");
            setup_advertising_mode();
            setup_scanning_mode();
            Serial.println("Switching to state: SCANNING_AND_ADVERTISING");
            DEVICE_STATE = START_BLE;
            break;
        //  currently scanning and advertising for device
        case SCANNING_AND_ADVERTISING:
            if(found_device){ // if device is the connector 
                Serial.println("Found a device to connect to, switching to state: FOUND_DEVICE");
                DEVICE_STATE = FOUND_DEVICE;
            }
            if(connected){
                Serial.println("A device has connected to us, switching to state: CONNECTED");
                DEVICE_STATE = NEED_AUTHENTICATION;
            }
            break;
        
        // starts advertising and scanning
        case START_BLE:
            Serial.println("Current state: START_BLE, starting advertising and scanning...");
            start_advertising();
            start_scanning();
            DEVICE_STATE = SCANNING_AND_ADVERTISING;
            break;
        
        // found a target device to connect to
        case FOUND_DEVICE:
            Serial.println("Current state: FOUND_DEVICE, attempting to connect...");
            if (connect_to_server(target_device)) { // attempt to connect to the found device, if connection is successful, the onConnect callback will be triggered and the state will be updated to CONNECTED, if connection fails, we will go back to scanning for devices
                DEVICE_STATE = NEED_TO_AUTHENTICATE;
            } else {
                Serial.println("Failed to connect to the found device, going back to scanning for devices...");
                DEVICE_STATE = START_BLE;
            }
            break;
        
        // need hand shake protocol from the connected device
        case NEED_AUTHENTICATION:
            Serial.println("Current state: NEED_AUTHENTICATION");
            if(allowed_connection){
                Serial.println("Handshake complete, switching to state: NOTIFYING_FOR_WEB_OR_MCU");
                DEVICE_STATE = NOTIFYING_FOR_WEB_OR_MCU;
            }
            break;
        case NOTIFYING_FOR_WEB_OR_MCU:
            Serial.println("Current state: NOTIFYING_FOR_WEB_OR_MCU");
            if(need_web_or_mcu_identification){
                notifiy_for_web_or_mcu();
            } 
            if(web_or_mcu == 0 || web_or_mcu == 1){ // if we have identified the connected device as web or MCU, we can move to the ALL_READY state
                Serial.println("Connected device identified as Web or MCU, switching to state: ALL_READY");
                DEVICE_STATE = ALL_READY;
            }
            break;
        // current device need to send hand shake protocol to the connected device
        case NEED_TO_AUTHENTICATE:
            Serial.println("Current state: NEED_TO_AUTHENTICATE, performing handshake...");
            if(need_handshake){
                perform_handshake();
            }
            else{
                DEVICE_STATE = IDENTIFYING_AS_WEB_OR_MCU; // if handshake is complete, move to the next state to identify if the connected device is web or MCU
            }
            break;
        case IDENTIFYING_AS_WEB_OR_MCU:
            Serial.println("Current state: IDENTIFYING_AS_WEB_OR_MCU");
            if(need_web_or_mcu_identification){
                authenticate_as_mcu();
            }
            DEVICE_STATE = ALL_READY; 
            break;

        // ready to receive and send data to the connected device
        case ALL_READY:
            if (have_message_from_connector) {

                // checks for messages periodically and sends the message to aws to process through openai api
                if (millis() - last_message_time >= MESSAGE_DELAY) {
                    Serial.println("Processing message from connected device and sending to AWS...");
                    send_message_to_aws(message_from_connector, CUR_DEVICE_PERSONALITY, CUR_MQTT_TOPIC_SUB, CUR_MQTT_TOPIC_PUB);
                    have_message_from_connector = false; // reset the flag after processing the message
                    last_message_time = millis(); // update the last message time
                    // Reset the one-time waiting log so the next pending message will print once
                    waiting_message_logged = false;
                } else {
                    // Print the waiting message only once while we're waiting for MESSAGE_DELAY
                    if (!waiting_message_logged) {
                        Serial.println("Received a message from connected device but waiting for MESSAGE_DELAY before sending to AWS...");
                        waiting_message_logged = true;
                    }
                }
            } else {
                // No pending message 
                waiting_message_logged = false;
            }
            break;
        
        // disconnected from the connected device
        case DISCONNECTED:
            Serial.println("Current state: DISCONNECTED");
            ESP_LOGW(state_tag, "Disconneted from connected device, Current state: DISCONNECTED");
            if(disonnected_from_client){
                ESP_LOGI(state_tag, "restarting BLE setup...");
                DEVICE_STATE = START_BLE; // restart the BLE setup to start advertising and scanning again to find and connect to devices again
                disonnected_from_client = false; // reset the flag after handling the disconnection
            }
    }   
}