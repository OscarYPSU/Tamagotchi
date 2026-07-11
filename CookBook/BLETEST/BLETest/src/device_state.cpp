#include "device_state.h"

enum device_state DEVICE_STATE = CHECK_WIFI_CREDS; // start at the beginning of the flow where the device needs to be registered with AWS DynamoDB to set up the personality and other information for the device that will be used in AWS Lambda when processing messages from the device and generating responses from ChatGPT, we will use this variable to keep track of where we are in the flow and what actions to perform in the main loop based on the current state of the device
