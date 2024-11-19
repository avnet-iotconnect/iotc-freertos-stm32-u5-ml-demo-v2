"""This module performs partial IoTConnect solution setup for audio classification demo."""

import json
import requests

from authentication import authenticate
from constants import DEVICE_TEMPLATES_DIR, TEMPLATES_TAIL, DEVICE_TEMPLATES, API_DEVICE_URL, CREATE_DEVICE_TEMPLATE, DEVICE_TEMPLATE_ALREADY_EXISTS
from check_status import check_status, BadHttpStatusException


def iot_connect_setup():
    """Perform IoTConnect solution setup"""
    access_token = authenticate()
    print("Successful login - now create device templates.")
    create_device_templates(access_token)

def create_device_templates(access_token: str):
    """Create devices templates in IoTConnect"""
    headers = {
        "Authorization": access_token
    }
    for template in DEVICE_TEMPLATES:
        files = {'file': open(DEVICE_TEMPLATES_DIR + template + TEMPLATES_TAIL, 'rb')}
        response = requests.post(API_DEVICE_URL + CREATE_DEVICE_TEMPLATE, files = files, headers = headers)

        try: 
            check_status(response)
            print(f"Template {template} created")
        except BadHttpStatusException as e:
            if DEVICE_TEMPLATE_ALREADY_EXISTS in response.text:
                print(f"Template {template} already exists")
            else:
                raise e

if __name__ == "__main__":
    iot_connect_setup()
