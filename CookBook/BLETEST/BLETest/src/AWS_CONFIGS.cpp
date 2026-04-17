#include "AWS_CONFIGS.h" // contains the device certs and private keys for both devices, as well as the mqtt topics for each device


// -----------------
// WIFI CONFIGS
// -----------------
const char* WIFI_SSID = "psu-personal";
// const char* WIFI_PASSWORD = "9172913763";  dont need since im using school wifi that doesnt require password

// AWS IOT configs
const char* AWS_ENDPOINT = "abuwn28a3fsb9-ats.iot.us-east-2.amazonaws.com";
// AWS certificate
const char* AWS_ROOT_CA = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
"rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
"-----END CERTIFICATE-----\n";

// ---------
// DEVICE 1 CONFIGS
// ---------

// Device1 cert
const char* DEVICE_1_CERT = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDWTCCAkGgAwIBAgIUQsWqKNV3f8nYApUxEKfUP4bMy9QwDQYJKoZIhvcNAQEL\n" \
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n" \
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI2MDMwNzAxNDgw\n" \
"NFoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n" \
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALv+i+dv2xjDwpXlYalc\n" \
"WWPB0v8qKalU5JebjIj1AR04wesuHoBvRG/nmIIr8RdoVbfIG7QCA9KQcoftpeaG\n" \
"aRvh+LU8Zul2WCYpiN5xtN2eOqgO1kZE5CEe4oMv+oQEHy68WVOOWdgyzAs3njk+\n" \
"X6XBpK4h4yb+pJdARGDeajJjqfMa+wNHd55CZXTBQFNPnuyn2JtGB5LGwjwIc0fK\n" \
"8KTYwlvQXiEVrIoXuW6u03aptuYwg5uozp07IYKy5lEyJgbDFklC2jJIWtfjDpn6\n" \
"o6O+diTxUd3MPdnVcwJZFzGoJggSoN1vxLqav3HDU5FoRInr6nYe22m4wrdmr/Yp\n" \
"LbUCAwEAAaNgMF4wHwYDVR0jBBgwFoAUbJV0VDsW9PGgwK9Q6t6OdBzF8+EwHQYD\n" \
"VR0OBBYEFGcRh1a9LsSv56OivUL2C25A+APkMAwGA1UdEwEB/wQCMAAwDgYDVR0P\n" \
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQCEDSrRZjVdH8R2Quuv4XS6vJhx\n" \
"kC5/PocAci+29Chm7HLnB+ZTxY2PG+3DptS+hQBwkPiS0KdI+/CDbSWLMwn7SfYW\n" \
"m759mWUi44mhudR7r2aCqRjBZXdYPsxOR7KqrDxkUMWwDn/5nbiJR0QQ/1jjY8lH\n" \
"5fvO0GTLfHivxpTZfKkHVbh7T5Zep4NRrC1em2935jlVZUiPYimRuayHipazJpHJ\n" \
"HENHAqgJ9U+Kh2wBmtt/BNUE1gIU4GVJ5cT5BFEKrzyQpGnrCsoJYKQ3yYcUbueN\n" \
"AhBEziAXegCEic7fYlDdPDKafSUmc+FK754ciUPzrAGtqfylQJXxPik5hP3L\n" \
"-----END CERTIFICATE-----\n";
// device 1 private key
const char* DEVICE_1_PRIVATE_KEY = \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEpAIBAAKCAQEAu/6L52/bGMPCleVhqVxZY8HS/yopqVTkl5uMiPUBHTjB6y4e\n" \
"gG9Eb+eYgivxF2hVt8gbtAID0pByh+2l5oZpG+H4tTxm6XZYJimI3nG03Z46qA7W\n" \
"RkTkIR7igy/6hAQfLrxZU45Z2DLMCzeeOT5fpcGkriHjJv6kl0BEYN5qMmOp8xr7\n" \
"A0d3nkJldMFAU0+e7KfYm0YHksbCPAhzR8rwpNjCW9BeIRWsihe5bq7Tdqm25jCD\n" \
"m6jOnTshgrLmUTImBsMWSULaMkha1+MOmfqjo752JPFR3cw92dVzAlkXMagmCBKg\n" \
"3W/Eupq/ccNTkWhEievqdh7babjCt2av9ikttQIDAQABAoIBAGsVtPWxJpl1sRqX\n" \
"XckOHLERUDIe/zpAbDHb3fKJtQfDM9rWG3PHbbFct+e2Rg+yU9lq+DTSTQEdXNAG\n" \
"09B7UdYbIwOBk5F64v1h/V/QX1k572mZgf7m8RfYsuhNZSLcAjMqAKvy00SussHU\n" \
"/aDX4b3WjwrNrMmilXi+l/Sh3NY0KwNYUlJCz+bcMAD+XszmABo3NqDz2TBA1yIn\n" \
"C+R3qznvif5NLIvNlt7jub6zfJpF1rpV9pLs7OgK4BrS3fkB9fDJhFx8nTKILN+U\n" \
"T9wvwsHllrC9tGxTqOen8a1Iyxh1cbCXDhyEfQoSry5EDE22hVqCS3ZXtMwelxC5\n" \
"1+LVDOUCgYEA5aicjpfguLKEBZXhlYaNGv2AgmXMdEBJfVWV/NPPVK59KUCLIsVA\n" \
"Mji3BkNk668a1Tb9GCYSYMnC4XvF/I8dChST0llEI5S1TkRXukdebU8S7aIoktZ8\n" \
"f+yJG1yPpr819uIHvJfkMO8F22QFsRJwS+1jYAsqXRtXwi2kIO7YBbMCgYEA0Y6P\n" \
"TyP8VfKJ9MZQbXpDvUvaJiSdn/CAVPJrURprSzJSb9T3KNKnc7e1jRYJFeX5rK5W\n" \
"w6mEPm8APdkxRxOq3Xttzi+j6FT4gs818VgNBQi00/1qWAAMezNbL821W5BjrCsz\n" \
"gRfx8z+wZRKGmU02F+qDYk2xUM6wMuzmiUN6mvcCgYEAtDeQQ0VjjwxnFYr8Lr4b\n" \
"VKbBhZIk0sTv5m6W+IOuGyGiDhEcHfjz23UuCrgwJdKPF+nCyoR1v4YUa6UZRIt5\n" \
"mkNzjImIMvOrIvP2c7M1okl+7QNnG3M815XiMZp9D7jUvBu7Pn9jDhNDYVooLT/u\n" \
"52YtqVyZrd6bC0GNnlpPIokCgYBvTMBg0EQdggwzrYDaJK4FxFvBFoGBiUMf2wGU\n" \
"uetyrO+L5hi/3eKCW3hZzvJUZykMxivfEBHk9x+xucGqdo+xwvS3JiZwJ3E2SMfl\n" \
"v1Zq7gzc6yOoSZ5XcRdldGR2lWHa97cLQaSIBbOyevxyP8gTO8M8Wvdqa9y5r4li\n" \
"LizYUQKBgQCIiJd8y1/fHIf1sFUb9DAKlELrbYxN5ZQF+DVX1gLsE/WPgE+gkhf+\n" \
"h9yeu2XhM1FpSN/yWLwUkpYfa08nyIP2qFtJ2eqxrfA6Uz594qs0ZGfR8EJJXIC+\n" \
"zlqzSSRQcx/h2KvcR/vrLAyR8bTG8GIZ8q/YJ61142rQh9ZSEH8owA==\n" \
"-----END RSA PRIVATE KEY-----\n";

const char* DEVICE_1_MQTT_TOPIC_PUB = "device/d1/input"; // endpoint for publishing data to openai for response generation
const char* DEVICE_1_MQTT_TOPIC_SUB = "device/d1/output"; // receiving the openai response from aws 


// ---------
// DEVICE 2 CONFIGS
// ---------

// Device cert
const char* DEVICE_2_CERT = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDWTCCAkGgAwIBAgIUWpd/RG9zLTkBmL6+0wFS4hzZof0wDQYJKoZIhvcNAQEL\n" \
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n" \
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI2MDQwOTA0MTk1\n" \
"N1oXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n" \
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALcXWHLS7qpKqYchvx03\n" \
"Keye62dhMIzTJT8P/eWkDTwoBVVnw+Yn4pd/63sGje/SQlYcba22bElFx9Tm6UcL\n" \
"hNTXGQ8Rmd52plWTTjctY4NHpUUJle7uOXx1CGeSivoUU+4MWSCdHa9rOdzQ4W3e\n" \
"7XoBuzn47Os/X3vZ/MwDoZ4EMM9t+VOAxgbpltpmBk3N3Hkw6LdhZbbYfPoBM01A\n" \
"f2eGoZQQ1Mf6q0s1qDwjUAoqJuwNnqL+XQt2+7jE6K1yqEQqZ5qywuJPgHQJDTZC\n" \
"1Px8dDojLMe4GpjawjNFXLvqWfmhKrWbBNY9p8sMXBvBqQhWZdR48RuUdYTXSP7C\n" \
"gj0CAwEAAaNgMF4wHwYDVR0jBBgwFoAUCPVNUboSVEUMWIfETGuiuB7ZhF8wHQYD\n" \
"VR0OBBYEFM/JmfC7j0GNI0SHsX/gGnVEAU9LMAwGA1UdEwEB/wQCMAAwDgYDVR0P\n" \
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQB+nKdsYqw6m6TfCs8sT5t1qTUu\n" \
"tq7IjpBMZJs0eAcfmI5B8ulO3AQX7AmKwNiIEBc0o9e1RxQFPcTFGN667u7AmEc6\n" \
"24o3SqqFn3RBDIPAlsvZ63bKGWUil3DYMJka/8hf6t/I6IYtOr3UrUKbTDiRqFML\n" \
"PqWpVWtnv76rnKv9klV5us+j3ZIY8E3cXSDP4MdqhaRHWdY2VFxpNSd5b+A0Ek8G\n" \
"ZZ2VklhSpHfh529vcIRtPRVfS3XqSgMD4BQDK1B9kWYd60aWGzuykI4pVGpInNFk\n" \
"Km3o0jy6D7sBN50ySPHlvKuovs/HvfLB5StAN7mOk81EVmlvLxGMnYsaWGTb\n" \
"-----END CERTIFICATE-----\n";
// private key
const char* DEVICE_2_PRIVATE_KEY = \
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEpAIBAAKCAQEAtxdYctLuqkqphyG/HTcp7J7rZ2EwjNMlPw/95aQNPCgFVWfD\n"\
"5ifil3/rewaN79JCVhxtrbZsSUXH1ObpRwuE1NcZDxGZ3namVZNONy1jg0elRQmV\n"\
"7u45fHUIZ5KK+hRT7gxZIJ0dr2s53NDhbd7tegG7Ofjs6z9fe9n8zAOhngQwz235\n"\
"U4DGBumW2mYGTc3ceTDot2Fltth8+gEzTUB/Z4ahlBDUx/qrSzWoPCNQCiom7A2e\n"\
"ov5dC3b7uMTorXKoRCpnmrLC4k+AdAkNNkLU/Hx0OiMsx7gamNrCM0Vcu+pZ+aEq\n"\
"tZsE1j2nywxcG8GpCFZl1HjxG5R1hNdI/sKCPQIDAQABAoIBAArMNKoo6GxglNjk\n"\
"U5oMe8t5n9Zr8+oKNcMfVVDquOEYVyAvS21SYtmecKvDujEvjDv51zoMbRxxIhTb\n"\
"PVy0QDQxL0glOJbXFn4rfsxgP+MpIYUNyKrbn8ZIPKOnD80pz71VOxhiS7Lhtvrf\n"\
"0ruhDw5H9du4Y3iIN0BdnbwGZsyjxxa6qg0pTLAnLGrrsvA15jEDlKQVAewwCMOf\n"\
"PiLud2Htgr2ZrRmT8OfIZPCn9koSZZEfXIsz0F//z/wmOSR5mEHNgf2xurel/oOk\n"\
"SxhQFg0y2qFdGvNMVylUxLfg0uHFWWtXxSoPf54RRJ2CcgPMKdMOojyfU2ld78do\n"\
"gHCamSECgYEA3KvrV/dd1g5xnfqJirwXLPW8lfzXi/h/u0mlcuweV8rfBK90Zjbv\n"\
"9I5BpI0JpZLlf8a+DlpeXoXVbKpGf1pbMLa5dGJz19d7TPbDIP2LzzFyszn2qrt/\n"\
"5JvtnZrjIYvx+XDOxeVm0n5MA31GwPhUa2hrtLvoVHW1ggFiEnFeRikCgYEA1Gc4\n"\
"IUWozydwUTnvCbdpj5gAoff2RxXYugdCRE8T07tULZwwKCQ23GDMwVAPEmMWT+EX\n"\
"0MJ5eSLck11M2RVxoZXvV0nAHcDt48QXcWGBBEA3AQkzC2Sw3IaReEYmTdOOdDoO\n"\
"K7rEFNTOKR7fiRszpBOIk96BYCELt3s54E6IFfUCgYA1NDoczmZQatEX7sTkry7D\n"\
"R0g9vWVWuQZK6Jm8WJOERUR6A7eDwXEfPIE1JFAUHJO6t/cwzLb+ATSQ64jtwaJE\n"\
"33ldRzN48donl0M6nAbuYJSwA6SmS2itfK7Qlfx3JR+lLX6dFg8xZwP6v2Skt/ra\n"\
"nqWBQODmJC8r9htoKO866QKBgQCW+H7m91JTW57zvQ/wghNf4xSgC1VblWWkZEBv\n"\
"uR1Io5/jg16fSY5M+ejPho4P7aoQQNfipDgfJ+5MOEbFDf7kcWPbUSpie50bBWf8\n"\
"SOehE9uEuvszH/Ct7mA7cvEK5FIevp0P7AIvJEsc4zrTgygjeVbcc4zVvOdNFSNA\n"\
"EdGceQKBgQCfxSJmnNwQX4uDQ9nVLcZVSTjX9GU4XZjfXGAy4dhKf4PpGt0YgWIU\n"\
"8pkT0Cy4XN3vTs913HyX5ADJdaZayL5XxduR5j71PJ1CceA99mtm6wm1n1xvHsb5\n"\
"OBLvXgLrtDAtI+8GqI1rt4iWO6JVws95RsCx1J9weQl8Qu/i7Qi2zw==\n"\
"-----END RSA PRIVATE KEY-----\n";

const char* DEVICE_2_MQTT_TOPIC_PUB = "device/d2/input"; // endpoint for publishing data to openai for response generation
const char* DEVICE_2_MQTT_TOPIC_SUB = "device/d2/output"; // receiving the openai response from aws 

// Intializes Current Device configs pointers
const char* CUR_DEVICE_CERT;
const char* CUR_PRIVATE_KEY;
const char* CUR_MQTT_TOPIC_PUB;
const char* CUR_MQTT_TOPIC_SUB;
const char* DEVICE_NAME_AWS; // for logging purposes to identify which device is which in the serial monitor

WiFiClientSecure net;
PubSubClient MQTT_client(net);

// connecting to wifi
void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// connecting to AWS IOT CORE via MQTT
void connectAWS(const char* DEVICE_CERT, const char* PRIVATE_KEY, const char* MQTT_TOPIC_SUB, const char* DEVICE_NAME_AWS) {
  net.setCACert(AWS_ROOT_CA);
  net.setCertificate(DEVICE_CERT);
  net.setPrivateKey(PRIVATE_KEY);

  MQTT_client.setServer(AWS_ENDPOINT, 8883); // AWS IoT Core MQTT port
  MQTT_client.setCallback(receive_response_from_AWS); // set the callback function to handle incoming messages

  Serial.println("Connecting to AWS IoT...");
  while (!MQTT_client.connected()) {
    if (MQTT_client.connect(DEVICE_NAME_AWS)) {
      // SUBSCRIBE HERE
      // The topic must match what your Lambda is publishing to
      MQTT_client.subscribe(MQTT_TOPIC_SUB); // subscribe to the topic where AWS Lambda will publish the ChatGPT response
      
      // Logs 
      Serial.print("Device: ");
      Serial.println(DEVICE_NAME_AWS);
      Serial.println("Subscribed to OpenAI response topic.");
      Serial.println("Connected to AWS IoT!");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(MQTT_client.state());
      Serial.println(" retrying in 2s");
    }
  }
}

void send_message_to_aws(const String& message, const char* MQTT_TOPIC_SUB, const char* MQTT_TOPIC_PUB) {
  if (MQTT_client.connected()) {
    String msg = "{\"message\":\"" + message + "\", \"topic_response\":\"" + MQTT_TOPIC_SUB + "\"}"; // packaging the message into a json format to be processed by lambda and then sent to openai
    MQTT_client.publish(MQTT_TOPIC_PUB, msg.c_str());
    Serial.println("Published to MQTT IOT CORE: " + msg);
  } else {
    Serial.println("MQTT not connected, cannot publish");
  }
}

void receive_response_from_AWS(char* topic, byte* payload, unsigned int length) {
  Serial.print("Response received on topic: ");
  Serial.println(topic);

  // Convert the byte payload into a readable String
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Print the ChatGPT response to your Serial Monitor
  Serial.println("--- OpenAI Response ---");
  Serial.println(message);
  Serial.println("-----------------------");
}

