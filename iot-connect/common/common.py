"""This module provides some useful commands for IoTConnect REST API."""

import requests

from .constants import (
    API_DEVICE_URL,
    DEVICE_TEMPLATE_LIST,
    API_USER_URL,
    ENTITY_LIST,
    DEVICE_CREATE
)
from .check_status import check_status


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

def get_device_guid(device_name: str, access_token: str) -> str:
    """Returns device guid from the IoTConnect"""
    headers = {
        "Authorization": access_token
    }
    params = {
        "UniqueId": device_name
    }
    response = requests.get(API_DEVICE_URL + DEVICE_CREATE, headers = headers, params = params)
    check_status(response)
    response_json = response.json()
    device_guid = response_json["data"][0]["guid"]
    return device_guid

def get_entity_guid(entity_name: str, access_token: str) -> str:
    """Returns entity guid for the provided entity name"""
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
