#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>

extern bool received_personality_from_aws; // flag to indicate if we have received the personality data from AWS yet, we want to wait to receive this before sending any messages to AWS so that the personality can be set up in AWS Lambda before we start sending messages and receiving responses that rely on the personality being set up


#endif
