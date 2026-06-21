#include <Arduino.h>
#include <Preferences.h>


Preferences preferences;

void save_to_nvs(String& ns, String& key, String& value) {
  Serial.println("Saving to NVS:");
  Serial.print("Namespace: ");
  Serial.println(ns);
  Serial.print("Key: ");
  Serial.println(key);
  Serial.print("Value: ");
  Serial.println(value);

  preferences.begin(ns.c_str(), false);
  preferences.putString(key.c_str(), value);
  preferences.end();
  
  Serial.println("Finished saving to NVS.");
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Wait for Serial Monitor to open

  // 1. Open NVS namespace "wifi-creds" in read-write mode (false)
  preferences.begin("wifi-creds", false);

  // 2. Check if the key "ssid" exists in memory
  if (!preferences.isKey("ssid")) {
    Serial.println("Memory Check: No credentials found.");
    Serial.println("Status: UNPROVISIONED");
    
    // For testing purposes, you can save temporary credentials like this:
    // preferences.putString("ssid", "Your_WiFi_Name");
    // preferences.putString("password", "Your_Password");
  } else {
    Serial.println("Memory Check: Credentials found!");
    
    // Retrieve them (the second argument "" is the default value if not found)
    String ssid = preferences.getString("ssid", "");
    String password = preferences.getString("password", "");
    
    Serial.print("SSID: ");
    Serial.println(ssid);
  }

  // 3. Close the preferences
  preferences.end();
}

void loop() {
  // Clear loop for this test
}