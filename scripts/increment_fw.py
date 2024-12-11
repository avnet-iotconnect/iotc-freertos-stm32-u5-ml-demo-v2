"""This module performs FW Build Version increment."""

from tempfile import mkstemp
from shutil import move, copymode
from os import fdopen, remove


BUILD_VERSION_PREFIX = "#define APP_VERSION_BUILD "
VERSION_FILE_PATH = "stm32/Projects/b_u585i_iot02a_ntz/Src/ota_pal/ota_firmware_version.c"


def increment_version(version_prefix: str, version_file_path: str) -> str:
    """Increments FW version in given file"""
    #Create temp file
    fh, abs_path = mkstemp()
    with fdopen(fh,'w') as new_file:
        with open(version_file_path) as old_file:
            for line in old_file:
                fixed_line = line
                if version_prefix in line:
                    current_version = int(line[len(version_prefix):])
                    new_version = current_version + 1
                    fixed_line = version_prefix + str(new_version) + "\n"
                    print(f"FW build version was changed from {current_version} to {new_version}")

                new_file.write(fixed_line)
    #Copy the file permissions from the old file to the new file
    copymode(version_file_path, abs_path)
    #Remove original file
    remove(version_file_path)
    #Move new file
    move(abs_path, version_file_path)


if __name__ == "__main__":
    increment_version(BUILD_VERSION_PREFIX, VERSION_FILE_PATH)
