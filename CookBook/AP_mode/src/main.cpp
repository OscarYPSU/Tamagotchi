#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <Arduino.h>
#include <LittleFS.h>
#include<Preferences.h>
#include "esp_log.h" // Include the ESP-IDF logging header for debugging

// intializes nvs
Preferences preferences;


// Set your Wi-Fi Network Name (SSID). Leave password empty "" for an open network.
const char* ssid = "ESP32-S3-Portalss";
const char* password = ""; 
bool ap_mode = false;

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dns_server;
WebServer web_server(80);

// tag for level logging
const char* nvs_tag = "NVS preferences";

// tag for ap mode
const char* ap_tag = "AP mode";

// saving to non volatile storage via preferences
void save_to_nvs(String ns, String key, String& value) {
  ESP_LOGI(nvs_tag, "Saving to NVS:");
  ESP_LOGI(nvs_tag, "Namespace: %s", ns.c_str());
  ESP_LOGI(nvs_tag, "Key: %s", key.c_str());
  ESP_LOGI(nvs_tag, "Value: %s", value.c_str());

  // Initialize the preferences 
  if (!preferences.begin(ns.c_str(), false)) {
    ESP_LOGE(nvs_tag, "Failed to begin preferences");
    return;
  } 
  
  if(preferences.putString(key.c_str(), value) == 0 ) {
    ESP_LOGW(nvs_tag, "Failed to put string");
    return;
  }

  ESP_LOGI(nvs_tag, "Ending preferences");
  preferences.end();
}

String read_from_nvs(String ns, String key){
  ESP_LOGI(nvs_tag, "Reading from NVS:");
  ESP_LOGI(nvs_tag, "Namespace: %s", ns.c_str());
  ESP_LOGI(nvs_tag, "Key: %s", key.c_str());

  if(!preferences.begin(ns.c_str(), true)) { // True for read only
    ESP_LOGE(nvs_tag, "Failed to begin preferences");
    return "None";
  }

  String value = preferences.getString(key.c_str(), "None");
  
  if(value == "None") {
    ESP_LOGW(nvs_tag, "Key not found");
  }

  ESP_LOGI(nvs_tag, "Value: %s", value.c_str());
  preferences.end();
  return value;
}

bool erase_from_nvs(String ns, String key){
  ESP_LOGI(nvs_tag, "Erasing from NVS:");
  ESP_LOGI(nvs_tag, "Namespace: %s", ns.c_str());
  ESP_LOGI(nvs_tag, "Key: %s", key.c_str());

  if(!preferences.begin(ns.c_str(), false)) {
    ESP_LOGE(nvs_tag, "Failed to begin preferences");
    return false;
  }

  if(!preferences.remove(key.c_str())) {
    ESP_LOGW(nvs_tag, "Failed to remove key");
    return false;
  }

  ESP_LOGI(nvs_tag, "Key removed successfully");
  preferences.end();

  return true;
}

bool erase_completely_from_nvs(String ns){
  ESP_LOGI(nvs_tag, "Erasing completely from NVS:");
  ESP_LOGI(nvs_tag, "Namespace: %s", ns.c_str());

  if(!preferences.begin(ns.c_str(), false)) {
    ESP_LOGE(nvs_tag, "Failed to begin preferences");
    return false;
  }

  if(!preferences.clear()) {
    ESP_LOGW(nvs_tag, "Failed to clear namespace");
    return false;
  }

  ESP_LOGI(nvs_tag, "Namespace cleared successfully");
  preferences.end();

  return true;
}

void handleRoot() {
  // Open the HTML file from flash storage
  File file = LittleFS.open("/index.html", "r");
  
  if (file) {
    // Stream the file directly to the browser
    web_server.streamFile(file, "text/html");
    file.close();
  } else {
    web_server.send(404, "text/plain", "Internal File Error");
  }
}

void stop_ap_mode(){
  ESP_LOGI(ap_tag, "Stopping AP mode");
  ap_mode = false;
  dns_server.stop();           // 1. Stop the DNS server
  web_server.stop();           // 2. Stop the web server
  WiFi.softAPdisconnect(true); // 3. Turn off the Access Point broadcasting
  WiFi.mode(WIFI_STA);         // 4. Switch Wi-Fi back to normal station mode
  ESP_LOGI(ap_tag, "AP mode stopped");
}

void handle_form_submit(){
  if(web_server.hasArg("wifi_name") && web_server.hasArg("wifi_password")){ // if the page has a form input name called wifi_name
    String wifi_name = web_server.arg("wifi_name");
    String wifi_password = web_server.arg("wifi_password");
    ESP_LOGI(ap_tag, "Received WiFi Name");
    ESP_LOGI(ap_tag, "Received WiFi Password");
    ESP_LOGI(ap_tag, "Attempting to save WiFi name to preferences NVS...");

    save_to_nvs("wifi-creds", "ssid", wifi_name);
    save_to_nvs("wifi-creds", "password", wifi_password);

    web_server.send(200, "text/plain", "Credentials saved successfully! Reconnecting... and turning off AP mode");

    delay(2000); // delay so data can get across without abrupt disconnetion

    // proceed to turn off acesspoint after getting these configs
    stop_ap_mode();
  } else {
    ESP_LOGE(ap_tag, "No wifi_name argument received");
    ESP_LOGE(ap_tag, "Expected POST parameter: wifi_name");
    web_server.send(400, "text/plain", "No wifi_name argument received");
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount Failed!");
    return;
  }

  erase_completely_from_nvs("wifi-creds");

  String existing_ssid = read_from_nvs("wifi-creds", "ssid");
  if(existing_ssid != "None") { // there is existing wifi credentials, no need to start access poiont mode
    String existing_password = read_from_nvs("wifi-creds", "password");
    ap_mode = false;
  } else { // if theres no existing wifi creds, set up access point to get configs
    ESP_LOGI(ap_tag, "Starting AP mode");
    ap_mode = true;
    // 1. Set up the ESP32 as an Access Point
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(ssid, password);

    // 2. Start DNS Server redirecting ALL traffic (*) to the ESP32 IP
    dns_server.start(DNS_PORT, "*", apIP);

    // 3. Web Server Routes
    web_server.on("/", handleRoot);

    web_server.on("/save_wifi_config", HTTP_POST, handle_form_submit);   // Set up the route to handle the incoming form submission
    web_server.onNotFound(handleRoot); // Captive portal trick: any unknown URL goes to root
    web_server.begin();
    ESP_LOGI(ap_tag, "AP mode started");
  }
}

void loop() {
  if (ap_mode) {
    // Handle access point specific logic here
    // Keep the DNS and Web Server running
    dns_server.processNextRequest();
    web_server.handleClient();
  }
}