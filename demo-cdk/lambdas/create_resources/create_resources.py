"""Create resources lambda handler."""

import os
import requests
import secrets
import boto3

from common.authentication import authenticate
from common.constants import (
    API_DEVICE_URL,
    API_EVENT_URL,
    DEVICE_SOUND_CLASS,
    RULE,
    SEVERETY_LOOKUP
)
from common.check_status import check_status, BadHttpStatusException
from common.common import (
    get_template_guid,
    get_rule_guid,
    get_entity_guid
)


KEY_LENGTH = 32

def create_resources_handler(event, context):
    # Create necessary resources
    print("event")
    print(event)
    create_s3_api_key_and_save()
    create_webhook_rules()

def create_s3_api_key_and_save():
    # Create S3 API Key and save to secret
    secret_name = os.environ['S3_KEY_SECRET_NAME']
    key_placeholder = os.environ['KEY_PLACEHOLDER']
    region = os.environ['REGION']
    session = boto3.session.Session()
    secrets_client = session.client(
        service_name='secretsmanager',
        region_name=region
    )
    print("Check if key already exists")
    key_value = secrets_client.get_secret_value(
        SecretId=secret_name
    )["SecretString"]

    if(key_value == key_placeholder):
        print("Create a new API Key")
        generated_key = secrets.token_urlsafe(KEY_LENGTH)
        secrets_client.put_secret_value(
            SecretId=secret_name,
            SecretString=generated_key
        )
        print("Key stored in the secret")
    else:
        print("Key already exists, skip new key creation")

def create_webhook_rules():
    # Create Webhook URL in the IoTConnect
    username = os.environ['IOTCONNECT_USERNAME']
    password = os.environ['IOTCONNECT_PASSWORD']
    solution_key = os.environ['IOTCONNECT_SOLUTION_KEY']
    entity = os.environ['IOTCONNECT_ENTITY']
    webhook_endpoint = os.environ['WEBHOOK_ENDPOINT']
    # access_token = authenticate(username, password, solution_key)
    access_token = authenticate(username, password, solution_key)
    print("Successful login - now create webhook rule.")
    template_guid = get_template_guid(DEVICE_SOUND_CLASS, access_token)
    entity_guid = get_entity_guid(entity, access_token)
    delete_rule_if_exists(DEVICE_SOUND_CLASS, access_token)
    creat_rule(DEVICE_SOUND_CLASS, webhook_endpoint, template_guid, entity_guid, access_token)

def delete_rule_if_exists(rule_name: str, access_token: str):
    # Delete rule if exists
    guid = get_rule_guid(rule_name, access_token)
    if (len(guid) == 0):
        print("Rule does not exist")
        return
    else:
        print(f"Found rule with guid {guid}.\r\nNow delete it.")
        delete_rule(guid, access_token)

def delete_rule(guid: str, access_token: str):
    # Delete rule
    headers = {
        "Content-type": "application/json",
        "Accept": "*/*",
        "Authorization": access_token
    }
    response = requests.delete(API_DEVICE_URL + RULE + "/" + guid, headers = headers)
    check_status(response)
    print("Rule deleted")

def creat_rule(rule_name: str, webhook_url: str, template_guid: str, entity_guid: str, access_token: str):
    """Create device in IoTConnect from given data"""
    major_guid = get_major_guid(access_token)

    data = {
        "ruleType": 1,
        "templateGuid": template_guid,
        "name": rule_name,
        "severityLevelGuid": major_guid,
        "conditionText": 'requests3 = "True"',
        "ignorePreference": False,
        "applyTo": 1,
        "entityGuid": entity_guid,
        "deliveryMethod": ["WebHook"],
        "url": webhook_url,
        "webhookMsgFormat": 2,
    }

    headers = {
        "Content-type": "application/json",
        "Accept": "*/*",
        "Authorization": access_token
    }
    response = requests.post(API_DEVICE_URL + RULE, json = data, headers = headers)

    check_status(response)
    rule_guid = get_rule_guid_from_response(response)
    print(f"Rule {rule_name} created with guid {rule_guid}")

    data = {
        "ruleType": 1,
        "templateGuid": template_guid,
        "name": "testint",
        "severityLevelGuid": major_guid,
        "conditionText": 'confidence > 100',
        "ignorePreference": False,
        "applyTo": 1,
        "entityGuid": entity_guid,
        "deliveryMethod": ["WebHook"],
        "url": webhook_url,
        "webhookMsgFormat": 2,
    }

    headers = {
        "Content-type": "application/json",
        "Accept": "*/*",
        "Authorization": access_token
    }
    response = requests.post(API_DEVICE_URL + RULE, json = data, headers = headers)

    check_status(response)
    rule_guid = get_rule_guid_from_response(response)
    print(f"Rule {rule_name} created with guid {rule_guid}")

def get_rule_guid_from_response(response: requests.Response) -> str:
    """Returns template guid from the response"""
    response_json = response.json()
    # print(response_json)
    guid = response_json["data"][0]["ruleGuid"]
    return guid

def get_major_guid(access_token: str) -> str:
    """Returns major severty level guid from the IoTConnect"""
    headers = {
        "Authorization": access_token
    }
    response = requests.get(API_EVENT_URL + SEVERETY_LOOKUP, headers = headers)
    check_status(response)
    response_json = response.json()
    # print(response_json)

    for sev_level in response_json["data"]:
        if sev_level["SeverityLevel"] == "Major":
            print(f"Found Major severety level guid {sev_level["guid"]}")
            return sev_level["guid"]

    print("Major severety level guid was not found")
    return ""

