"""This module performs partial IoTConnect OTA FW Update."""

import argparse
import requests

from authentication import authenticate
from constants import (
    DEVICE_TEMPLATES_DIR,
    TEMPLATES_TAIL,
    API_DEVICE_URL,
    DEVICE_TEMPLATE_CREATE,
    DEVICE_TEMPLATE_LIST,
    DEVICE_TEMPLATE_ALREADY_EXISTS,
    DEVICE_ALREADY_EXISTS,
    DEVICE_WITH_SOUND_GEN,
    DEVICE_WITH_SOUND_CLASS,
    DEVICE_CREATE,
    ENTITY_LIST,
    API_USER_URL,
    FW_OTA_FILE,
    API_FW_URL,
    FW_ADD,
    FW_PREFIX
)
from check_status import check_status, BadHttpStatusException
from common import (
    get_template_guid,
    get_entity_guid
)


def iot_connect_fw_ota():
    """Perform IoTConnect FW OTA"""
    args = parse_arguments()
    access_token = authenticate(args)
    print("Successful login - now create OTA update for the device.")
    template_guid = get_template_guid(DEVICE_WITH_SOUND_CLASS, access_token)
    create_firmware(template_guid, access_token, "1.1.99")
    # entity_guid = get_entity_guid(args.entity_name, access_token)

def parse_arguments() -> argparse.Namespace:
    """
    Parse CLI arguments - 4 obligatory positional argiments
     - username
     - password
     - solution key
     - entity name
    """
    print("Parse command line arguments")
    parser=argparse.ArgumentParser()
    parser.add_argument("username")
    parser.add_argument("password")
    parser.add_argument("solution_key")
    parser.add_argument("entity_name")
    return parser.parse_args()

def create_firmware(device_template_guid: str, access_token: str, fw_version: str) -> str:
    """Create new firmware in IoTConnect"""
    firmwareName = FW_PREFIX + fw_version.replace('.', '')
    data = {
        "deviceTemplateGuid": device_template_guid,
        "software": fw_version,
        "firmwareName": firmwareName,
        "hardware": "1.0.0"
    }

    headers = {
        "Content-type": "application/json",
        "Accept": "*/*",
        "Authorization": access_token
    }
    response = requests.post(API_FW_URL + FW_ADD, json = data, headers = headers)
    check_status(response)
    response_json = response.json()
    guid = response_json["data"][0]["firmwareUpgradeGuid"]

    print(f"Firmware {firmwareName} created with guid {guid}")
    return guid


if __name__ == "__main__":
    iot_connect_fw_ota()
