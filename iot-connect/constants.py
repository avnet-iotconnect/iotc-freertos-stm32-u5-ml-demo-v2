"""This module defines constants."""

STATUS_OK = 200

API_AUTH_URL = "https://awspocauth.iotconnect.io/api/v2.1/"
API_DEVICE_URL = "https://awspocdevice.iotconnect.io/api/v2.1/"

BASIC_TOKEN = "Auth/basic-token"
LOGIN = "Auth/login"
CREATE_DEVICE_TEMPLATE = "device-template/quick"

DEVICE_TEMPLATES_DIR = "templates/devices/"
TEMPLATES_TAIL = "_template.JSON"

DEVICE_TEMPLATES = ["soundclass", "soundgener"]

DEVICE_TEMPLATE_ALREADY_EXISTS = "DeviceTemplateNameAlreadyExists"
