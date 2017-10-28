def generate_response(message):
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

def update_shadow(lightName, status):
    pass

def lambda_handler(event, context):
    if event['request']['type'] == 'IntentRequest':
        intent = event['request']['intent']

        if intent['name'] == 'LuxTurnOn':
            update_shadow(intent['slots']['Light']['value'], True)
        elif intent['name'] == 'LuxTurnOff':
            update_shadow(intent['slots']['Light']['value'], False)
    
    return generate_response('OK, I will do that.')

