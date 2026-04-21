import json
import boto3 # library for AWS services
import random

# sets up the configs for boto3
boto3_client = boto3.client('iot-data', region_name='us-east-2')

dynamodb = boto3.resource('dynamodb')
personality_table = dynamodb.Table('device_personality') # dynamo table name that holds device personality

PERSONALITIES = ["Cheerful", "Grumpy", "Shy", "Sarcastic", "Optimistic", "Pessimistic", "Friendly", "Moody", "Energetic", "Calm"]

def lambda_handler(event, context):
    # AWS IoT Core passes the MQTT payload into the 'event' variable.
    # event data structure should be like the follow {
    # "device_id": "MAC id of device to get personality for", 
    # "topic_response": "Topic to publish response to" ,
    #}    
    try:
        device_id = event.get('device_id', 'No device_id found')

        # Try to find the device in DynamoDB
        response = personality_table.get_item(Key={'device_id': device_id})
        item = response.get('Item')

        if not item:
            print(f"Did not find personality, setting up personality for device_id: {device_id}\n")
            personality = random.choice(PERSONALITIES) # Randomly assign a personality from the list
            print(f"Assigned personality: {personality} to device_id: {device_id}\n")
            personality_table.put_item(
                Item={
                    'device_id': device_id,
                    'personality': personality
                }
            )
        else: # If the device already has a personality, we can print it out or use it as needed
            personality = item.get('personality', 'No personality found')
            print(f"Found existing personality: {personality} for device_id: {device_id}\n")

        topic_response = event.get('topic_response', 'no topic_response section found in data')

        print(f"sending response to topic: {topic_response}\n")
        # Define the payload to send back
        payload = {
            "personality": personality
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
