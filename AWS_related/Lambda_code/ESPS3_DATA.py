import json
from openai import OpenAI #OpenAI functionality
import boto3 # library for AWS services
import random

# The client automatically looks for the "OPENAI_API_KEY" environment variable
openAI_client = OpenAI()

# sets up the configs for boto3
boto3_client = boto3.client('iot-data', region_name='us-east-2')

def lambda_handler(event, context):
    # AWS IoT Core passes the MQTT payload into the 'event' variable.
    # event data structure should be like the follow {
    #"message": "Message from ESP32", 
    #"topic_response": "Topic to publish response to",
    #"personality": "personality",
    #}    
    try:
        # We extract the message your ESP32 sent
        device_message = event.get('message', 'No message found')
        topic_response = event.get('topic_response', 'no topic_response section found in data')
        device_personality = event.get('personality', 'no personality section found in data')
        # ----------
        # RECIEIVNG THE DATA FROM MCU THEN PROCESSING IT VIA OPENAI API AND PRINTING IT IN TERMINAL 
        #-----------

        # For now, we are just printing it. In AWS Lambda, print() statements 
        # are automatically saved to AWS CloudWatch Logs.
        print("SUCCESS! Data received from MCU:")
        print(f"Message: {device_message}, Target Topic: {topic_response}")
        #Call OpenAI with the device message as input
        response = openAI_client.chat.completions.create(
            model="gpt-5.4-nano", # Note: Check if this model name is correct for your tier
            max_completion_tokens=100,       # <--- ADD THIS LINE (limits the response length)
            messages=[
                {"role": "user", "content": device_message},
                {"role": "system", "content": "No emotes, only text. No newlines. Keep it brief. just have a conversation and form a relationship, you are a " + device_personality + " personality"}
            ]
        )

        # Print the response from OpenAI
        print("OpenAI Response:")
        print(response.choices[0].message)

        # SENDS OPENAI RESPONSE TO A TOPIC

        # Define the payload to send back
        payload = {
            "message": response.choices[0].message.content
        }
        
        # Publish to the topic the ESP32 is listening to
        response = boto3_client.publish(
            topic=topic_response,
            qos=0,
            payload=json.dumps(payload)
        )

        print("Message published to topic: " + topic_response)

        return {
            'statusCode': 200,
            'body': json.dumps('Data successfully processed!')
        }
        
    except Exception as e:
        print(f"Error processing data: {e}")
        return {
            'statusCode': 500,
            'body': json.dumps('Error processing data')
        }
