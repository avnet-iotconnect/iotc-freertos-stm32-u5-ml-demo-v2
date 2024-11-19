"""This module defines constants."""

STATUS_OK = 200

API_AUTH_URL = "https://awspocauth.iotconnect.io/api/v2.1/"
API_DEVICE_URL = "https://awspocdevice.iotconnect.io/api/v2.1/"
API_USER_URL = "https://awspocuser.iotconnect.io/api/v2.1/"

BASIC_TOKEN = "Auth/basic-token"
LOGIN = "Auth/login"
CREATE_DEVICE_TEMPLATE = "device-template/quick"
DEVICE_TEMPLATE_LIST = "device-template"
CREATE_DEVICE = "Device"
ENTITY_LIST = "Entity"

DEVICE_TEMPLATES_DIR = "templates/devices/"
TEMPLATES_TAIL = "_template.JSON"

DEVICE_WITH_OWN_CERT = "soundclass"
DEVICE_WITH_AWS_CERT = "soundgener"

DEVICE_TEMPLATES = [DEVICE_WITH_OWN_CERT, DEVICE_WITH_AWS_CERT]

DEVICE_TEMPLATE_ALREADY_EXISTS = "DeviceTemplateNameAlreadyExists"
DEVICE_ALREADY_EXISTS = "UniqueIdAlreadyExists"
