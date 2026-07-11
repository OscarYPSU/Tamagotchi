#include "NVS.h"

// intializes nvs
Preferences preferences;
const char* nvs_tag = "NVS"; // Tag for logging

// reads from nvs 
// ns = namespace, key = 
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
  } else {
    ESP_LOGI(nvs_tag, "Value saved successfully");
  }

  ESP_LOGI(nvs_tag, "Ending preferences");
  preferences.end();
}

// clears all data inside the nvs MAINLY FGOR TESTING PURPOSE 
void clear_all_data_nvs(String ns){ // ns is the namespace (the place) where you want to delete all the data
    ESP_LOGI(nvs_tag, "Clearing all data from NVS:");
    ESP_LOGI(nvs_tag, "Namspace: %s", ns.c_str());
    preferences.begin(ns.c_str(), false);
    preferences.clear();
    preferences.end();
}   

