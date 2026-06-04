#ifndef DEVICE_STATE_H
#define DEVICE_STATE_H

#include <Arduino.h>

enum device_state {
    CHECK_REGISTRATION,       // check for registration 
    WAITING_FOR_REGISTRATION, // waiting for user to register device after being prompted
    NEED_REGISTRATION,        // registered with AWS DynamoDB via user username and password
    PERSONALITY_SET_UP,       // getting personality etc
    SETUP_BLE,                // SET UP BLE
    START_BLE,                // restart BLE setup to start advertising/scanning
    SCANNING_AND_ADVERTISING, // currently scanning and advertising
    FOUND_DEVICE,             // found a device to connect to as a client
    NEED_TO_AUTHENTICATE,     // device is scanner, need to perform handshake
    NEED_AUTHENTICATION,      // device is advertiser, waiting for handshake
    IDENTIFYING_AS_WEB_OR_MCU,// after handshake, identify if web or MCU
    NOTIFYING_FOR_WEB_OR_MCU, // notifies connected device to send type
    ALL_READY,                // ready to send/receive messages
    DISCONNECTED              // disconnected, need to start over
};

extern enum device_state DEVICE_STATE; // declare the variable to keep track of the current state of the device, this will be defined in device_state.cpp and can be accessed and modified in main.cpp and other files that include this header

#endif
