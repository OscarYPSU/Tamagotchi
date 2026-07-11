#include "AP.h"
#include "NVS.h"

// Set your Wi-Fi Network Name (SSID). Leave password empty "" for an open network.
const char* ssid = "ESP32-S3-Portalss";
const char* password = ""; 

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dns_server;
WebServer web_server(80);

const char* ap_tag = "AP";

void handle_root(){
    // Open the HTML file from flash storage
    File file = LittleFS.open("/index.html", "r");
    
    if (file) {
        // Stream the file directly to the browser
        web_server.streamFile(file, "text/html");
        file.close();
    } else {
        ESP_LOGE(ap_tag, "Internal File Error");
        web_server.send(404, "text/plain", "Internal File Error");
    }
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

void stop_ap_mode(){
  ESP_LOGI(ap_tag, "Stopping AP mode");
  dns_server.stop();           // 1. Stop the DNS server
  web_server.stop();           // 2. Stop the web server
  WiFi.softAPdisconnect(true); // 3. Turn off the Access Point broadcasting
  WiFi.mode(WIFI_STA);         // 4. Switch Wi-Fi back to normal station mode
  ESP_LOGI(ap_tag, "AP mode stopped");
  DEVICE_STATE = CHECK_REGISTRATION; // 5. Switch back to the CHECK_REGISTRATION state to check for wifi credentials and account details
  return;
}

void start_ap_mode(){
    ESP_LOGI(ap_tag, "Starting AP mode");
    dns_server.start(DNS_PORT, "*", apIP);
    web_server.begin();
    ESP_LOGI(ap_tag, "AP mode started");
    return;
}

void setup_ap_mode() {

    ESP_LOGI(ap_tag, "Setting up AP mode");

    // reads the file from littleFS which contains the web frontend
    if (!LittleFS.begin(true)) {
        ESP_LOGE(ap_tag, "LittleFS Mount Failed!");
        return;
    }
    ESP_LOGI(ap_tag, "LittleFS Mount Successful!");

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(ssid, password);

    // Start DNS Server redirecting ALL traffic (*) to the ESP32 IP
    dns_server.start(DNS_PORT, "*", apIP);

    // Web Server Routes
    web_server.on("/", handle_root); 

    web_server.on("/save_wifi_config", HTTP_POST, handle_form_submit);   // Set up the route to handle the incoming form submission
    web_server.onNotFound(handle_root); // Captive portal trick: any unknown URL goes to root

    ESP_LOGI(ap_tag, "Finished setting up AP mode");
    return;
}