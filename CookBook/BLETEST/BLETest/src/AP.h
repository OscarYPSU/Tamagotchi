#pragma once 

#include <WiFi.h> // allows us to create a WiFi access point
#include <DNSServer.h> // allows us to create a DNS server for the access point 
#include <WebServer.h> // allows us to create a web server for access point for the mcu
#include <Arduino.h>
#include <LittleFS.h> // file system to store web frontend 
#include<Preferences.h> // NVS storage 
#include "esp_log.h" // Include the ESP-IDF logging header for debuggings
#include "device_state.h"

// configuration variables for setting up access point mode 
extern const char* ssid; // SSID for the access point
extern const char* password; // Password for the access point
extern const byte DNS_PORT;
extern IPAddress apIP;
extern DNSServer dns_server;
extern WebServer web_server;

extern const char* ap_tag; // tag for logging

void handle_root(); // handles the root page of the access point, serves the index.html file from littleFS
void handle_form_submit(); // handles how the form submission is processed and how the data is saved to NVS
void stop_ap_mode(); // stops the ap mode
void start_ap_mode(); // starts the ap mode, ONLY after the setup ap mode function has been ran at least once
void setup_ap_mode(); // sets up the access point using the configuartoin variables
