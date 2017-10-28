"""
AWS Lambda Alexa integration

1. Alexa sends voice command to AWS Lambda, where this script is hosted
2. Lambda connects to Shadow and updates value stored
3. Hub server must poll for updates to Shadow
"""

from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTShadowClient

SHADOW_CLIENT_ID = ""
SHADOW_ENDPOINT = ""
SHADOW_PORT = 8333
SHADOW_THING_NAME = "lux-light-status"


def generate_response(message):
    """
    Generate a response for Alexa
    This contains the text for Alexa's verbal response, as well as instructions on ending the session
    :param message: Verbal response for Alexa
    :return: Response
    """

    return {
        "version": "1.0",
        "response": {
            "outputSpeech": {
                "type": "PlainText",
                "text": message
            },
            "card": {
                "content": message,
                "title": "Lux Light",
                "type": "Simple"
            },
            "reprompt": {
                "outputSpeech": {
                    "type": "PlainText",
                    "text": ""
                }
            },
            "shouldEndSession": True
        },
        "sessionAttributes": {}
    }


def update_shadow(light_name, status):
    """
    Update the IoT Thing Shadow
    :param light_name: The name of the light to update
    :param status: The new status of the light (on|off)
    :return: None
    """

    # Configure and connect to shadow device
    client = AWSIoTMQTTShadowClient(SHADOW_CLIENT_ID)
    client.configureEndpoint(SHADOW_ENDPOINT, SHADOW_PORT)
    client.connect()

    # Update the value stored in the shadow
    handler = client.createShadowHandlerWithName(SHADOW_THING_NAME, True)
    payload = '{{}:"{}"}'.format(light_name, status)
    handler.shadowUpdate(payload, None, 5)


def lambda_handler(event, context):
    """
    AWS Lambda entry point
    :param event: Event information
    :param context: Session context
    :return: Response to caller (Alexa)
    """

    # If the request was an intent from Alexa
    if event['request']['type'] == 'IntentRequest':
        intent = event['request']['intent']

        # Determine the intent type and update the shadow accordingly
        if intent['name'] == 'LuxTurnOn':
            update_shadow(intent['slots']['Light']['value'], 'on')
        elif intent['name'] == 'LuxTurnOff':
            update_shadow(intent['slots']['Light']['value'], 'off')
        else:
            # This really shouldn't happen, and would likely be due to a configuration error
            return generate_response('I\'m not sure what you mean')

    return generate_response('OK, I will do that.')

