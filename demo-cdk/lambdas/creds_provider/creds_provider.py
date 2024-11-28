"""Credentials provider lambda handler."""

import os

from common.authentication import authenticate

def creds_provider_handler(event, context):
    # Send credentials to the devices via the IoTConnect command
    print("event")
    print(event)
    username = os.environ['IOTCONNECT_USERNAME']
    password = os.environ['IOTCONNECT_PASSWORD']
    solution_key = os.environ['IOTCONNECT_SOLUTION_KEY']
    # access_token = authenticate(username, password, solution_key)
    authenticate(username, password, solution_key)
    print("Successful login - now create device templates.")

    res = {
        "statusCode": 200,
        "headers": {
            "Content-Type": "*/*"
        },
        "body": "Hello, "
    }

    return res
