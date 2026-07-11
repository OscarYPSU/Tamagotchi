#pragma once 

#include "esp_log.h"
#include "preferences.h"

extern Preferences preferences; // Declare the preferences object as extern
extern const char* nvs_tag; // Declare the NVS tag as extern

void save_to_nvs(String ns, String key, String& value); // Saves a string value to NVS under the specified namespace and key
void clear_all_data_nvs(String ns); // Clears all data in the specified namespace
String read_from_nvs(String ns, String key); // reads a string value from NVS 