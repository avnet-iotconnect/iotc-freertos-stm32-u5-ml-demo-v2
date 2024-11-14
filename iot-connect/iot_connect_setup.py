"""This module performs partial IoTConnect solution setup for audio classification demo."""

from authentication import authenticate


def iot_connect_setup():
    """Perform IoTConnect solution setup"""
    access_token = authenticate()
    print("Successful login - continuation tbd.")

if __name__ == "__main__":
    iot_connect_setup()
