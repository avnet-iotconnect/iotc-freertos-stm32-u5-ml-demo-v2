"""This module defines constants."""

STATUS_OK = 200

API_AUTH_URL = "https://awspocauth.iotconnect.io/api/v2.1/"
API_DEVICE_URL = "https://awspocdevice.iotconnect.io/api/v2.1/"
API_USER_URL = "https://awspocuser.iotconnect.io/api/v2.1/"
API_FW_URL = "https://awspocfirmware.iotconnect.io/api/v2.1/"

BASIC_TOKEN = "Auth/basic-token"
LOGIN = "Auth/login"
DEVICE_TEMPLATE_CREATE = "device-template/quick"
DEVICE_TEMPLATE_LIST = "device-template"
DEVICE_CREATE = "Device"
ENTITY_LIST = "Entity"
FW_ADD = "Firmware"

DEVICE_TEMPLATES_DIR = "templates/devices/"
TEMPLATES_TAIL = "_template.JSON"

DEVICE_WITH_SOUND_CLASS = "soundclass"
DEVICE_WITH_SOUND_GEN = "soundgener"

DEVICE_TEMPLATES = [DEVICE_WITH_SOUND_CLASS, DEVICE_WITH_SOUND_GEN]

DEVICE_TEMPLATE_ALREADY_EXISTS = "DeviceTemplateNameAlreadyExists"
DEVICE_ALREADY_EXISTS = "UniqueIdAlreadyExists"

FW_OTA_FILE = "ota/fw.bin"
FW_PREFIX = "SCLASS"
