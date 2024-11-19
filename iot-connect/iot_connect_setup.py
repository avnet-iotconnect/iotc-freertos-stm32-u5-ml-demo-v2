"""This module performs partial IoTConnect solution setup for audio classification demo."""

import argparse
import requests

from authentication import authenticate
from constants import (
    DEVICE_TEMPLATES_DIR,
    TEMPLATES_TAIL,
    API_DEVICE_URL,
    CREATE_DEVICE_TEMPLATE,
    DEVICE_TEMPLATE_LIST,
    DEVICE_TEMPLATE_ALREADY_EXISTS,
    DEVICE_ALREADY_EXISTS,
    DEVICE_WITH_AWS_CERT,
    DEVICE_WITH_OWN_CERT,
    CREATE_DEVICE,
    ENTITY_LIST,
    API_USER_URL
)
from check_status import check_status, BadHttpStatusException


def iot_connect_setup():
    args = parse_arguments()
    """Perform IoTConnect solution setup"""
    access_token = authenticate(args)
    print("Successful login - now create device templates.")
    entity_guid = get_entity_guid(args.entity_name, access_token)
    soundclass_temp_guid = create_device_templates(access_token, DEVICE_WITH_OWN_CERT)
    create_device_with_own_certificate(args.certificate, soundclass_temp_guid, entity_guid, access_token)
    soundgen_temp_guid = create_device_templates(access_token, DEVICE_WITH_AWS_CERT)
    create_device_with_aws_certificate(args.certificate, soundgen_temp_guid, entity_guid, access_token)

def parse_arguments() -> argparse.Namespace:
    """
    Parse CLI arguments - 4 obligatory positional argiments
     - username
     - password
     - solution key
     - device certificate
     - entity name
    """
    print("Parse command line arguments")
    parser=argparse.ArgumentParser()
    parser.add_argument("username")
    parser.add_argument("password")
    parser.add_argument("solution_key")
    parser.add_argument("entity_name")
    parser.add_argument("certificate")
    return parser.parse_args()

def get_entity_guid(entity_name: str, access_token: str) -> str:
    """Returns entity guid "for the provided entity name"""
    headers = {
        "Authorization": access_token
    }
    response = requests.get(API_USER_URL + ENTITY_LIST, headers = headers)
    check_status(response)
    response_json = response.json()
    entities = response_json["data"]

    entity_guid = ""
    for entity in entities:
        if entity["name"] == entity_name:
            entity_guid = entity["guid"]
            break
    
    print(f"Entity guid for {entity_name} entity is {entity_guid}")

    return entity_guid

def get_template_guid(template_name: str, access_token: str) -> str:
    """Returns template guid from the IoTConnect"""
    headers = {
        "Authorization": access_token
    }
    params = {
        "DeviceTemplateName": template_name
    }
    response = requests.get(API_DEVICE_URL + DEVICE_TEMPLATE_LIST, headers = headers, params = params)
    check_status(response)
    response_json = response.json()
    template_guid = response_json["data"][0]["guid"]
    return template_guid

def get_template_guid_from_response(response: requests.Response) -> str:
    """Returns template guid from the response"""
    response_json = response.json()
    guid = response_json["data"][0]["deviceTemplateGuid"]
    return guid

def create_device_templates(access_token: str, template_name: str):
    """Create devices templates in IoTConnect"""
    headers = {
        "Authorization": access_token
    }

    files = {'file': open(DEVICE_TEMPLATES_DIR + template_name + TEMPLATES_TAIL, 'rb')}
    response = requests.post(API_DEVICE_URL + CREATE_DEVICE_TEMPLATE, files = files, headers = headers)

    template_guid = ""
    try: 
        check_status(response)
        template_guid = get_template_guid_from_response(response)
        print(f"Template {template_name} created with guid {template_guid}")
    except BadHttpStatusException as e:
        response_json = response.json()
        error_type = response_json["error"][0]["param"]
        if DEVICE_TEMPLATE_ALREADY_EXISTS == error_type:
            template_guid = get_template_guid(template_name, access_token)
            print(f"Template {template_name} already exists with guid {template_guid}")
        else:
            raise e

    return template_guid

def create_device_with_own_certificate(certificate: str, soundclass_temp_guid: str, entity_guid: str, access_token: str):
    """Create device with own certificate in IoTConnect"""
    data = {
        "entityGuid": entity_guid,
        "uniqueId": DEVICE_WITH_OWN_CERT,
        "deviceTemplateGuid": soundclass_temp_guid,
        "certificateText": certificate,
        "displayName": DEVICE_WITH_OWN_CERT
    }
    headers = {
        "Content-type": "application/json",
        "Accept": "*/*",
        "Authorization": access_token
    }
    response = requests.post(API_DEVICE_URL + CREATE_DEVICE, json = data, headers = headers)

    try: 
        check_status(response)
        print(f"Device {DEVICE_WITH_OWN_CERT} created")
    except BadHttpStatusException as e:
        response_json = response.json()
        error_type = response_json["error"][0]["param"]
        if DEVICE_ALREADY_EXISTS == error_type:
            print(f"Device {DEVICE_WITH_OWN_CERT} already exists")
        else:
            raise e

def create_device_with_aws_certificate(certificate: str, soundclass_temp_guid: str, entity_guid: str, access_token: str):
    """Create device with aws certificate in IoTConnect"""
    data = {
        "entityGuid": entity_guid,
        "uniqueId": DEVICE_WITH_AWS_CERT,
        "deviceTemplateGuid": soundclass_temp_guid,
        "isAutoGeneratedSelfSignCertificate": True,
        "displayName": DEVICE_WITH_AWS_CERT
    }
    headers = {
        "Content-type": "application/json",
        "Accept": "*/*",
        "Authorization": access_token
    }
    response = requests.post(API_DEVICE_URL + CREATE_DEVICE, json = data, headers = headers)

    try: 
        check_status(response)
        print(f"Device {DEVICE_WITH_AWS_CERT} created")
    except BadHttpStatusException as e:
        response_json = response.json()
        error_type = response_json["error"][0]["param"]
        if DEVICE_ALREADY_EXISTS == error_type:
            print(f"Device {DEVICE_WITH_AWS_CERT} already exists")
        else:
            raise e

if __name__ == "__main__":
    iot_connect_setup()
