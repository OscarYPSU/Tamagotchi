#ifndef DEVICE_STATE_H
#define DEVICE_STATE_H

#include <Arduino.h>

enum device_state {
    CHECK_REGISTRATION, // check for registration AKA looking for existing wifi credentials in NVS and also account details
    NEED_REGISTRATION, // if no existing wifi credentials or account details, need to register device, autoppmatically goes to AP MODE
    WAITING_FOR_REGISTRATION, // grabbing registration details
    SETTING_AP_MODE, // setting and starting ap mode
    REGRISTRATION_MODE, // currently in AP mode to display web frontend to grab user credentials
    SETTING_CONFIGURATION, // getting everything like the wifi configs, account details, and personality data 
    SETUP_BLE, // SET UP BLE
    START_BLE, // restart BLE setup to start advertising/scanning
    SCANNING_AND_ADVERTISING, // currently scanning and advertising
    FOUND_DEVICE,             // found a device to connect to as a client
    NEED_TO_AUTHENTICATE,     // device is scanner, need to perform handshake
    NEED_AUTHENTICATION,      // device is advertiser, waiting for handshake
    IDENTIFYING_AS_WEB_OR_MCU,// after handshake, identify if web or MCU
    NOTIFYING_FOR_WEB_OR_MCU, // notifies connected device to send type
    ALL_READY,                // ready to send/receive messages
    DISCONNECTED  // Disconnected from the current connected device, routes back to START_BLE
};

extern enum device_state DEVICE_STATE; // declare the variable to keep track of the current state of the device, this will be defined in device_state.cpp and can be accessed and modified in main.cpp and other files that include this header

#endif
