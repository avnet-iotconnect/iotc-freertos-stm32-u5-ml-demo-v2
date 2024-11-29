"""Credentials provider lambda handler."""

import os
import requests
import json
import secrets
import boto3

from common.authentication import authenticate
from common.constants import (
    API_DEVICE_URL,
    DEVICE_SOUND_CLASS,
    COMMAND_SEND,
    S3_ENDPOINT_HEADER,
    S3_APIKEY_HEADER
)
from common.check_status import check_status, BadHttpStatusException
from common.common import (
    get_template_guid,
    get_entity_guid,
    get_command_guid
)

def creds_provider_handler(event, context):
    # Send credentials to the devices via the IoTConnect command
    print("event")
    print(event)
    username = os.environ['IOTCONNECT_USERNAME']
    password = os.environ['IOTCONNECT_PASSWORD']
    solution_key = os.environ['IOTCONNECT_SOLUTION_KEY']
    endpoint_url = os.environ['S3_ENDPOINT']
    api_key_secret = os.environ['S3_KEY_SECRET_NAME']
    region = os.environ['REGION']
    session = boto3.session.Session()
    secrets_client = session.client(
        service_name='secretsmanager',
        region_name=region
    )
    api_key = secrets_client.get_secret_value(
        SecretId=api_key_secret
    )["SecretString"]
    # access_token = authenticate(username, password, solution_key)
    authenticate(username, password, solution_key)
    print(f"Successful login - send command to the device {DEVICE_SOUND_CLASS}")
    template_guid = get_template_guid(DEVICE_SOUND_CLASS, access_token)
    entity_guid = get_entity_guid(args.entity_name, access_token)
    command_guid = get_command_guid(template_guid, access_token)
    send_command(command_guid, endpoint_url, api_key, entity_guid, access_token)

def send_command(command_guid: str, endpoint_url: str, api_key: str, entity_guid: str, access_token: str):
    """Create device in IoTConnect from given data"""

    parameter_value = {
        S3_ENDPOINT_HEADER: endpoint_url,
        S3_APIKEY_HEADER: api_key,
    }

    parameter_value_json = json.dumps(parameter_value) 

    data = {
        "commandGuid": command_guid,
        "entityGuid": entity_guid,
        "applyTo": 1,
        "isRecursive": False,
        "parameterValue": parameter_value_json
    }

    headers = {
        "Content-type": "application/json",
        "Accept": "*/*",
        "Authorization": access_token
    }
    response = requests.post(API_DEVICE_URL + COMMAND_SEND, json = data, headers = headers)

    check_status(response)
    print(f"Command sent")

def get_rule_guid_from_response(response: requests.Response) -> str:
    """Returns template guid from the response"""
    response_json = response.json()
    # print(response_json)
    guid = response_json["data"][0]["ruleGuid"]
    return guid
