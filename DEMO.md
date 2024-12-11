# ML Audio Classifier: Demonstration Setup

[TOC]

## Overview

This guide is designed to assist users in setting up and demonstrating an ML-based audio classification system using AWS, Avnet IoTConnect powered by AWS and the STMicroelectronics STM32U5 Discovery Kit built on the ultra-low-power STM32U585AII6Q microcontroller based on the Arm® Cortex®-M33 core with Arm® TrustZone®. This system demonstrates real-time machine learning inference on edge devices and model retraining, highlighting the integration of cloud and IoT technologies.

While the foundation of the technology can support the classification of events based on most any sensor input (temperature, sound, motion, current, ect.), this solution utilizes the Discovery Kit's 2 microphones for audio classification. The trained audio models are stored in the Models folder of the project, and with automation of the included *../scripts/setup-project.sh* script, these models are incorporated into the project and the MCU binary is built. The out-of-the-box demo incorporates 5 audio classifications to identify the following scenarios: *Honking, Car Racing, Alarms, Dog(s) Barking, and Rushing Water*. When an event is triggered by the corresponding sound, the event name, confidence level, and GPS location of the audio classifier node is sent to the cloud to alert the user.

This guide outlines the steps to recreate the demo.

## Deployment Prerequisites

The project is built using GitHub actions, CDK IaC and IoTConnect REST API. So it can be deployed to your AWS account directly from the GitHub.

**Note** This Project was only tested on region `eu-west-2`

### AWS Setup

#### GitHub Connection

It is necessary to set up a connection between the pipeline and your forked Repo

1. Follow the steps to create & verify the connection [here](https://docs.aws.amazon.com/dtconsole/latest/userguide/connections-create-github.html)
2. Copy created Connection Arn. We will need it during the GitHub Secrets setup

#### Credentials

Now lets create AWS Credentials. In AWS go to IAM Service.

<img src="media/AWS/iam.png" alt="drawing"/>

Start with creating the Policy.

1. Go to Policies and press "Create policy"
   <img src="media/AWS/press-create-policy.png" alt="drawing"/>

2. In Policy editor switch to JSON and paste the bellow text into it. Check https://github.com/aws/aws-cdk/issues/21937 for possible changes in required policies. And press "Next".

   ```
   {
       "Version": "2012-10-17",
       "Statement": [
           {
               "Effect": "Allow",
               "Action": "codebuild:ImportSourceCredentials",
               "Resource": "*"
           },
           {
               "Effect": "Allow",
               "Action": "sts:AssumeRole",
               "Resource": "arn:aws:iam::*:role/cdk-*"
           },
           {
               "Action": [
                   "cloudformation:CreateChangeSet",
                   "cloudformation:DeleteStack",
                   "cloudformation:DescribeChangeSet",
                   "cloudformation:DescribeStackEvents",
                   "cloudformation:DescribeStacks",
                   "cloudformation:ExecuteChangeSet",
                   "cloudformation:GetTemplate"
               ],
               "Resource": "arn:aws:cloudformation:*:*:stack/CDKToolkit-StMicro/*",
               "Effect": "Allow",
               "Sid": "CloudFormationPermissions"
           },
           {
               "Action": [
                   "iam:CreateRole",
                   "iam:DeleteRole",
                   "iam:GetRole",
                   "iam:AttachRolePolicy",
                   "iam:DetachRolePolicy",
                   "iam:DeleteRolePolicy",
                   "iam:PutRolePolicy",
                   "iam:TagRole"
               ],
               "Effect": "Allow",
               "Resource": [
                   "arn:aws:iam::*:policy/*",
                   "arn:aws:iam::*:role/cdk-*"
               ]
           },
           {
               "Action": [
                   "s3:CreateBucket",
                   "s3:DeleteBucket",
                   "s3:PutBucketPolicy",
                   "s3:DeleteBucketPolicy",
                   "s3:PutBucketPublicAccessBlock",
                   "s3:PutBucketVersioning",
                   "s3:PutEncryptionConfiguration",
                   "s3:PutLifecycleConfiguration"
               ],
               "Effect": "Allow",
               "Resource": [
                   "arn:aws:s3:::cdk-*"
               ]
           },
           {
               "Action": [
                   "ssm:DeleteParameter",
                   "ssm:GetParameter",
                   "ssm:GetParameters",
                   "ssm:PutParameter"
               ],
               "Effect": "Allow",
               "Resource": [
                   "arn:aws:ssm:*:*:parameter/cdk-bootstrap/*"
               ]
           },
           {
               "Action": [
                   "ecr:CreateRepository",
                   "ecr:DeleteRepository",
                   "ecr:DescribeRepositories",
                   "ecr:SetRepositoryPolicy",
                   "ecr:PutLifecyclePolicy"
               ],
               "Effect": "Allow",
               "Resource": [
                   "arn:aws:ecr:*:*:repository/cdk-*"
               ]
           }
       ]
   }
   ```

   <img src="media/AWS/policy-permissions.png" alt="drawing"/>

3. Give the policy a name, e.g. "CDK_Deploy", and press "Create policy"
   <img src="media/AWS/policy-name.png" alt="drawing"/>

Now create the user group

1. In IAM go to User groups and press "Create group"
   <img src="media/AWS/press-create-group.png" alt="drawing"/>
2. Give group a name, e.g. "CDK_Deploy" and in "Attach permissions policies" find and check the polucy you created earlier
   <img src="media/AWS/group.png" alt="drawing"/>
3. Press "Create user group"

Now create the user

1. In IAM go to Users and press "Create user"
   <img src="media/AWS/press-create-user.png" alt="drawing"/>
2. Give user a name, e.g. "cdk-deploy" and press "Next"
   <img src="media/AWS/user-name.png" alt="drawing"/>
3. On the next step find a check the group you previously created, press "Next"
   <img src="media/AWS/user-add-group.png" alt="drawing"/>
4. On the next step press "Create user"
   <img src="media/AWS/user-create.png" alt="drawing"/>

Now create an access key and a secret access key for the new user.

1. In IAM go to Users and press on the prebiously created user
   <img src="media/AWS/new-user.png" alt="drawing"/>
2. Go to Security credentials and press "Create access key"
   <img src="media/AWS/press-create-key.png" alt="drawing"/>
3. Choose "Command Line Interface (CLI)" Use case, check the confirmation and press "Next"
   <img src="media/AWS/key-use-case.png" alt="drawing"/>
4. Fill the Description tag value if you want and press "Create access key"
   <img src="media/AWS/key-create.png" alt="drawing"/>
5. Copy and save both "Access key" and "Secret access key", you will need them later and press "Done"
   <img src="media/AWS/key-copy.png" alt="drawing"/>

### ST Account Setup

ST Cloud credentials:

- Create an account in [STM32Cube.AI Developer Cloud](https://stm32ai-cs.st.com/home)
- Login and accept an agreement
- Copy login and password, you will need them later

### IoTConnect Setup

You should have account in Avnet IoTConnect powered by AWS. Retreive the following things from IoTConnect account:

- Solution key
- Login
- Password
- Entity name

Uou will need it later.

If you don't have the solution key - in IoT Connect go to Support Ticket -> Other Support and create ticket with title "Please, create the solution key" and description "Hello! We need to use REST API for our solution. Can you, please, create the solution key?" and press "Save". Support team will create the solution key for you.

<img src="media/IoTConnect/create-ticket.png" alt="drawing"/>

<img src="media/IoTConnect/save-ticket.png" alt="drawing"/>

### Firmware Flashing and Getting Certificate

- Download and install a serial console application (TeraTerm for example) and STM32CubeProgrammer
- Download and extract [b_u585i_iot02a_ntz_v1_4_1.zip](https://saleshosted.z13.web.core.windows.net/demo/st/b_u585i_iot02a_ntz_v1_4_0.zip) !!!!!!!!!TBD!!!!!!!!!
- This is a non-TFM project. If tustzone is enabled on your board, please disable before programming.
- Connect the board with a Micro USB to a PC cable located on the top of the board. Do not connect the USB-C port or the micro USB port at the bottom.
- Open the STM32CubeProgrammer and connect it to the board by clicking the *Connect* button on the top right.
- Click the *Erasing&Programming* button (second button on the left sidebar)
  - It is recommended to run a *Full Chip Erase* (top right of the screen), but this step is optional.
  - Click *Browse* and navigate to your unzipped .bin file.
  - *Start Address* should be auto-detected to 0x08000000.
  - Click the *Start Programming* button (see the screenshot below).
- Once flashing is complete Disconnect the board from the programmer and re-plug the device.

<img src="media/programmer-flash.png" alt="drawing"/>

#### Configuring Device Certificates

- To configure the Server CA certificate, a commands needs to be executed on the serial terminal console that will take the certificate as input.
- Open the serial console terminal. If you are seeing no output, you can type **help** on the terminal to trigger the device to respond.
- Next, we need to configure the Server CA certificate. The command will expect the certificate to be pasted along with the BEGIN and END lines. If this process is successful, a log line will print *Success: Certificate loaded to label: 'root_ca_cert'*. Ignore the "Could not open" warnings on the screen during the process. Run the following command and paste the [Starfield Root CA G2 Certificate](https://github.com/avnet-iotconnect/iotc-freertos-stm32-u5-ml-demo/blob/main/QUICKSTART.md#Starfield-Root-CA-G2) below at the ">" prompt:
  - pki import cert root_ca_cert
- To generate the device's X509 credentials, we will need to generate a private key and a self-signed certificate based on the public/private key pair that's also generated internally. Enter the following commands:
  - pki generate key
  - pki generate cert
- Capture the certificate output of the pki generate cert command along with BEGIN CERTIFICATE and END CERTIFICATE lines and paste it into a file - we will need it during the GitHub Secrets setup.
- You can always obtain the same generated certificate by executing **pki export cert tls_cert** at the command prompt.
- Keep the terminal application open. We will need it again to configure the device connection details.

### Credentials Preparation

Before the solution can be deployed to the cloud it is necessary to provide the credentials into the GitHub secrets.

In GitHub repository go to Settings -> Secrets and variables -> Actions and fill the secret values:

- AWS_ACCESS_KEY: AWS access key you previously
- AWS_ACCOUNT_NUMBER: AWS account number
- AWS_REGION: AWS region in which you want to deploy the demo
- AWS_SECRET_ACCESS_KEY: AWS secret access key you previously
- GIT_ARN: ARN of the GitHub connection in the AWS
- IOT_CONNECT_CERTIFICATE: the certificate you captured from the device
- IOT_CONNECT_ENTITY_NAME: your Entity name in the IoTConnect account
- IOT_CONNECT_PASSWORD: your password for IoTConnect account
- IOT_CONNECT_SOLUTION_KEY: your solution key in the IoTConnect account
- IOT_CONNECT_USERNAME: your login for IoTConnect account
- STDEVCLOUD_PASSWORD: your password for [STM32Cube.AI Developer Cloud](https://stm32ai-cs.st.com/home) account
- STDEVCLOUD_USERNAME: your login for [STM32Cube.AI Developer Cloud](https://stm32ai-cs.st.com/home) account

<img src="media/GitHub/secrets.png" alt="drawing"/>

## Deployment

### CDK Bootstrap

The deploy should start from the CDK bootstrap. In repository go to Actions -> Bootstrap CDK. Press on the "Run workflow" dropdown, choose main branch and press "Run workflow".

<img src="media/GitHub/bootstrap.png" alt="drawing"/>

Wait till the action is finished.

<img src="media/GitHub/bootstrap-finished.png" alt="drawing"/>

#### Troubleshooting

If action is failed most likely credentials in secrets are wrong or policy created previously is wrong. AWS sometimes update permissions list needed for the CDK bootstrap. You can avoid this issue by changing the policy to allow all possible action on all possible resources - but keep in mind that it is not good practise.

```
{
  "Version": "2012-10-17",
  "Statement": {
    "Effect": "Allow",
    "Action": "*",
    "Resource": "*"
  }
}
```

### Deploy

Now the solution could be deployed.

In repository go to Actions -> Solution Deploy. Press on the "Run workflow" dropdown, choose main branch and press "Run workflow".

This action will deploy infrastructure to the AWS and IoTConnect.

<img src="media/GitHub/deploy.png" alt="drawing"/>

Wait till the action is finished.

<img src="media/GitHub/deploy-finished.png" alt="drawing"/>

After action is successfuly finished, the audio files download process will be automatically started in the AWS. You can't start retraining process before this process is finished. It usually takes from 30 to 60 minutes. To check the downloading progress, you can go to CodeBuild in AWS.

<img src="media/AWS/aws-codebuild.png" alt="drawing"/>

Go to "Build projects" and press on the "DownloadBuild" project.

<img src="media/AWS/download-build.png" alt="drawing"/>

Press on the build run which is "in Progress"

<img src="media/AWS/download-in-progress.png" alt="drawing"/>

You will see the downloading log. The process order:

1. Downloading Dev Audio (6 parts)
2. Downloading Eval Audio
3. Downloading Ground Truth
4. Unzipping
5. Uploading to S3

When you will see in the log "Phase complete: BUILD State: SUCCEEDED" - the files are in S3, which means that you can start the retraining.

### Configuring Device Connection Parameters

Now it is necessary to configure the Device Connection Parameters. In IoTConnect go to the device Info tab

<img src="media/IoTConnect/device.png" alt="drawing"/>

Go to the Devices and press on the "soundclass" device.

<img src="media/IoTConnect/device-soundclass.png" alt="drawing"/>

Press on the "Connection info".

<img src="media/IoTConnect/connection-info.png" alt="drawing"/>

Note the following values in the device Connection info screen which we will use for the device runtime configuration in the next steps:

- Your Unique Device ID that you used to create the device will be used as thing_name.
- Host, which will be used for the mqtt_endpoint value.

<img src="media/IoTConnect/connection-info-screen.png" alt="drawing"/>

- Enter the following commands on the serial terminal to configure the MQTT connection:
  - conf set thing_name your-device-id
    - To take advantage of the GPS location feature, the user should assign their device one of the following names to correspond with a unique GPS location: ml-ai-demo-01, ml-ai-demo-02, ml-ai-demo-03, or ml-ai-demo-04.
  - conf set mqtt_endpoint your-endpoint
  - conf set mqtt_port 8883
- Enter the following commands to set up the WiFi connection for your device:
  - conf set wifi_ssid your-wifi-ssid
  - conf set wifi_credential your-wifi-password
- Verify values by entering the **conf get** command and examining the output.
- Enter **conf commit**. Note that must commit the changes so that they take effect.
- Enter **reset** to reset the device.

The device should connect at this point, and you should be able to see data in the Telemetry and Latest Value tabs in IoTConnect.

## Retraining

The solution provides the retraining functionality. If some sounds are not recognized properly, user can send retrain command to the device from the IoTConnect. The Device will upload not properly recognized audio file to the AWS with the proper classification, which is provided in the command issued by the user.

The AWS infrastructure will start the retraining process, which will tale ~40 minutes. After that the AWS will push the new ML model prepared for the firmware to the GitHub repository, branch "retrained-model".

The push will trigger GitHub action, which will build the firmware and create the OTA Update in the IoTConnect for the device.

### Prerequisites

If we put unrecognized audio sample to the training process - it is not guaranteed that after that ML model will be able to recognize this file. However, if we really want the model to recognize specific audio sample - we can put this single sample a few time to the model. For that purpose the TRAINING_SAMPLE_REPEAT_NUMBER exists in GitHub repository. The higher this number, the higher the probability that this sample will be recognized after the training. Values should be between 1 and 100. To change this number go to GitHub repo Settings -> Secrets and variables -> Actions -> Variables and press on the pencil. Change the value and press "Update variable".

<img src="media/GitHub/var.png" alt="drawing"/>

### Command

To send retrain command to the device, in IoTConnect go to the device Info tab

<img src="media/IoTConnect/device.png" alt="drawing"/>

Open Commands and in the "Command Execution" fill the fields:

- Device Template: "soundclass"
- Selected devices: "soundclass"
- Command: "Start ML retraining process"
- Parameter Value:
  - 1 for Alarm
  - 2 for Bark
  - 3 for Liquid
  - 4 for Race car and auto racing
  - 5 for Vehicle horn and car horn and honking

Press "Execute Command"

<img src="media/IoTConnect/retrain-cmd.png" alt="drawing"/>

Sending this command to the device will trigger the retrain process.

### Checking Progress

Retraning process include a few steps. The retraining itself could me monitored in the CodeBuild.

Go to CodeBuild in AWS.

<img src="media/AWS/aws-codebuild.png" alt="drawing"/>

Go to "Build projects" and press on the "SagemakePipeline" project.

<img src="media/AWS/sgmkr-ppln.png" alt="drawing"/>

You will see the Build run in Progress. This step usually takes ~30 minutes.

After Sagemaker Pipeline is finished, the "PushCode" code build will be triggered, which will push new model to the GitHub repository, branch "retrained-model". This step usually takes ~30 seconds.

After code is pushed, GitHub action Build Firmware and create OTA Update will be triggered. Go to GitHub repository -> Actions -> Build Firmware and create OTA Update and press on the run in progress.

<img src="media/GitHub/build-fw.png" alt="drawing"/>

There you can press on the job in progress and it's log. This step usually takes ~4 minutes.

After this action is finished, the OTA Update will be created for the device in IoTConnect. Go to IoTConnect, the device Info tab

<img src="media/IoTConnect/device.png" alt="drawing"/>

Go to Firmware -> OTA Updates

<img src="media/IoTConnect/ota-updates.png" alt="drawing"/>

There you will see OTA Update History including the new update.

### Manual FW Flashing

If OTA updates are not working, you can manually flash the firmware. In repository go to Actions -> Build Firmware and create OTA Update and press on the latest workflow run.

<img src="media/GitHub/build-fw.png" alt="drawing"/>

Wait till the action is finished if it is still in progress. After that refresh the page and you will see "fw.bin" in artifacts. Download it and unzip.

<img src="media/GitHub/fw-bin.png" alt="drawing"/>

- Connect the board with a Micro USB to a PC cable located on the top of the board. Do not connect the USB-C port or the micro USB port at the bottom.
- Open the STM32CubeProgrammer and connect it to the board by clicking the *Connect* button on the top right.
- Click the *Erasing&Programming* button (second button on the left sidebar)
  - Click *Browse* and navigate to your unzipped .bin file.
  - *Start Address* should be auto-detected to 0x08000000.
  - Click the *Start Programming* button (see the screenshot below).
- Once flashing is complete Disconnect the board from the programmer and re-plug the device.

<img src="media/programmer-flash.png" alt="drawing"/>

## ML Audio Classifier Sensor Nodes

### Program and Provision

Follow the [Quickstart Guide](https://github.com/avnet-iotconnect/iotc-freertos-stm32-u5-ml-demo/blob/main/QUICKSTART.md) to flash the STM32U5 Discovery Kit and provision audio classifier devices into IoTConnect.

### Device Commands

**set-confidence-threshold**

- **Purpose:** This command sets a global confidence threshold that an audio event must exceed to be considered valid. It's essential for reducing false positives and ensuring that the system only reacts to events with a high probability of accuracy.
- **Usage:** `set-confidence-threshold [threshold_value]`
- **Example:** `set-confidence-threshold 75`
- **Explanation:** Here, the threshold is set at 75%. Any audio event classified below this confidence level will not trigger an alert or action.

**set-confidence-offsets**

- **Purpose:** Adjusts the confidence offsets for different types of audio events, allowing for customized sensitivity settings for each sound classification. This command is crucial for tailoring the device's response based on specific sound characteristics.
- **Usage:** `set-confidence-offsets [Alarm], [Bark], [Liquid], [Race_car_and_auto_racing], [Vehicle_horn_and_car_horn_and_honking], [other]`
- Default Values:
  - Alarm: 0
  - Bark: -49
  - Liquid: -19
  - Race car and auto racing: 39
  - Vehicle horn, car horn, and honking: 27
  - Other: -25
- **Example:** `set-confidence-offsets 0, -45, -19, 39, 27, -25`
- **Explanation:** This example sets the confidence offsets where notably, the offset for "Bark" is adjusted from -49 to -45. This adjustment slightly increases the likelihood that barking sounds are classified as positive detections, reflecting changes made to enhance detection accuracy for this specific sound type.

**set-device-location**

- **Purpose:** Updates or sets the geographical location of the device. This command is particularly useful for mobile or relocated devices where location-specific data can significantly impact the context and relevance of audio event classifications.
- **Usage:** `set-device-location [latitude] [longitude]`
- **Example:** `set-device-location 34.0522 -118.2437`
- **Explanation:** This example sets the device location to specific coordinates (latitude 34.0522, longitude -118.2437), which could be crucial for applications that rely on geographical data for functionality or reporting.

**set-inactivity-timeout**

- **Purpose:** This command configures a timeout period that begins after an audio event is detected. During this time, the device suppresses further alerts of the same type, displaying a "no activity" status instead. This helps prevent overlapping alerts when sounds occur in quick succession.
- **Usage:** `set-inactivity-timeout [units]`
- **Example:** `set-inactivity-timeout 800`
- **Explanation:** In this example, the timeout is set for 800 units, where each unit represents 1/100th of a second, totaling 8 seconds. If the device detects an audio event, it will not issue another alert for the same event type for the next 8 seconds, thereby managing the frequency of alerts effectively.

## Audio Samples

Audio clips to use for this demo can be downloaded [here](https://saleshosted.z13.web.core.windows.net/demo/st/iotc-freertos-stm32-u5-ml-demo/audio-samples.zip) These clips have been extracted from the FDS50K libraries at [Freesound.org](https://annotator.freesound.org/fsd/release/FSD50K) and edited as follows:

- Stereo to Mono: The conversion from stereo to mono ensures that the same audio clip is played by each of the speakers, regardless if the left or right channel is selected.
- Converted to .wav files: Not required as VLC player can play various encoding types, but for the purpose of using identical codecs, converted to ensure audio settings are constant when the sample is played.
- Clip sample Edits: When required, dead-space has been removed from the front-end of the sample. And in some cases, multiple samples have been extracted from the same clip when the sounds are vastly different.

This curated sample group represents the types of activities that would be of interest in an urban environment at a street corner and includes barking, water rushing, honking, car racing, and alarms. Sounds that depict violence were not chosen because of various venue's content policies and includes gunshots, explosions, car break-ins (glass shattering), fights/arguments, and screaming.

[![Event Notifications](https://camo.githubusercontent.com/f22a76da09e3923873aa9ed22ba1e61c9c0d196b3190837213a27842eb326d6e/68747470733a2f2f73616c6573686f737465642e7a31332e7765622e636f72652e77696e646f77732e6e65742f6d656469612f73746d6963726f2f7265696e76656e7432332f726561646d652f4d4c7374617475736c6162656c732e676966)](https://camo.githubusercontent.com/f22a76da09e3923873aa9ed22ba1e61c9c0d196b3190837213a27842eb326d6e/68747470733a2f2f73616c6573686f737465642e7a31332e7765622e636f72652e77696e646f77732e6e65742f6d656469612f73746d6963726f2f7265696e76656e7432332f726561646d652f4d4c7374617475736c6162656c732e676966)

## Demonstration Setup

### Overview

The following section outlines the setup for demonstrating the Urban Sound Event Classifier. In this configuration, 2 audio classification devices are acoustically isolated from one another, each representing 2 physical locations on a dashboard map. This demo setup uses an edge server to play the audio samples. This edge server is connected to IoTConnect and uses the Python SDK to receive C2D commands, as well as send basic telemetry from the edge server. The edge server will play the selected audio sample when the appropriate command is received to a connected audio device. In addition, the edge server will play the audio sound on the chosen audio channel.

### Materials

- [STM32U5 Discovery Kit Board](https://www.avnet.com/shop/us/products/stmicroelectronics/b-u585i-iot02a-3074457345647217745)
- [Clear Pelican Case](https://www.amazon.com/gp/product/B0C73G2WXJ)
  - The purpose of the case is the following:
    - Group the demo components into and enclosed system that best represents a final product.
    - Isolate the sound to a specific Sensor Node, allowing a demonstration of 2 node locations with different audio classifications.
    - Keep the audio volume down, as sirens, alarms, and other distracting noises are being played.
      - *Note, eliminating outside noise to prevent false classifications is not the reason for the case. While this was the primary concern, the audio classification models work well in a noisy environment. Confidence levels can be adjusted to prevent false notifications. However, the audio sample must be heard by the Sensor Node, so the case does help this cause.*
- [Battery Pack](https://www.amazon.com/gp/product/B07CZDXDG8)
  - Battery pack provides power to the STM32U5 board, allowing the node to be completely wireless. Caution, the minimal current draw can cause the battery pack output to cycle, resetting the board. Plugging in the Bluetooth speakers increases the current draw and can prevent power cycling.

[Bluetooth Stereo Speakers](https://www.amazon.com/gp/product/B078SLF7YF)

## Audio Generator

### Verify Python Installation

Verify if Python3 is installed. If installed, you will want to link it to “python”, and if not, you will want to install python.

```
python3 –version
```

if yes, link python to python3:

```
sudo ln -s /usr/bin/python3 /usr/bin/python
```

If not, install python:

```
sudo apt update
sudo apt install python3
sudo ln -s /usr/bin/python3 /usr/bin/python
python --version
```

### Install External Python Library – psutil

**psutil (Python system and process utilities)** is a cross-platform library for retrieving information on running processes and system utilization (CPU, memory, disks, network, sensors) in Python. It is optional, but installed to provide information about the linux machine to IoTConnect and allows remote monitoring of the system.

```
pip install psutil
```

**Verify installation** by fetching the CPU usage and the number of logical CPUs in the system:

```
Python
> psutil.cpu_percent()
> psutil.cpu_count(logical=True)
> Ctrl-D (to exit Python shell)
```

### Install PulseAudio Control and VLC Player

**Install PulseAudio Control**: Used to manage audio syncs (left and right channels). This allows us to play the audio sample on the left speaker, right speaker, or both and allow the demo to isolate the audio to 1 sensor when 2 sensors are being represented.

```
sudo apt-get install pulseaudio
```

**Install VLC Player:** VLC allows use to play audio through the command line and can also allow us to trigger an audio event through an IoTConnect command using systemd calls.

```
sudo apt-get install vlc
```

### Download the IoTConnect Python SDK

The IoTConnect Python SDK will be running on the Sound Generator. We will be using the IoTConnect Python SDK V1.1 which is referenced in the documentation site [here](https://docs.iotconnect.io/iotconnect/sdk/sdk-flavors/python/python-sdk-1-1/). This SDK is based on IoTConnect [Device Message version 2.1](https://docs.iotconnect.io/iotconnect/sdk/message-protocol/device-message-2-1/), which supports both Azure and AWS connectivity.

1. Create a Work directory: If such a directory does not exist, create one. for the purpose of this guide, directory “work” has been created at the user home directory.

```
mkdir work
cd work
```

2. Download and Extract the SDK package

```
wget -O iotc-python-sdk-std21-patch.zip https://github.com/avnet-iotconnect/iotc-python-sdk/archive/std21-patch.zip && unzip iotc-python-sdk-std21-patch.zip
```

3. Navigate to the SDK directory

```
cd iotc-python-sdk-std21-patch
```

4. Install the SDK libraries

```
pip3 install iotconnect-sdk.tar.gz
```

5. Download and unzip the sound generator custom script to the ‘sample’ directory

```
wget -O iotc_sound_generator.zip https://saleshosted.z13.web.core.windows.net/demo/st/iotc-freertos-stm32-u5-ml-demo/sound_generator/iotc_sound_generator.zip && unzip iotc_sound_generator.zip -d ./sample
```

### Create a Device and Export AWS Certificates from IoTConnect

1. Select Device from the left-hand tool bar
2. Open "soundgener" device
3. Download the AWS-generated device certificates.
   - Select “Connection Info” from the device you just created.
   - Select the certificate icon to download the certificate files
4. Unzip the certs and move them into the “aws_cert” folder in the cloned repository. In my case, the device display name is “MCLi7”, and the certificates are stored in the generated zip file “MCLi7-certificates.zip” mple/aws_cert

### Gather your Device Credentials

Make note of the following Device credentials

- UniqueID
  - This is the device id you provided when you created the device. It is also noted in the Device List and as the ClientID in the connection info.
  - Copy and store the UniqueID for future use.
- SID (Python SDK ID)
  - You can get get the SID from the IoTConnect UI portal "Settings -> Key Vault -> SDK Identities -> select language Python and Version 2.1"
  - On the toolbar, navigate to settings -> key vault
  - Select SDK Identities -> select language Python and Version 2.1". Copy and store the Identity key for future use.

## Modify the sample application

1. Open the iotc_sound_generator.py script in an editor and modify the following:
   - Enter your UniqueID and SID values
   - Enter the correct paths to your SSL Key and SSL Cert
   - Save the Python script

## Run and Test the Script

1. To run the Script

```
python iotc_sound_generator.py
```

2. In the terminal, the device should load its certificates and connect to IoTConnect. You will see telemetry from the terminal every 5 seconds when it is connected.

3. Go to the device in IoTConnect, and enter the command window view. Here you will be able to enter the command, such as z_channel, by selecting the command and entering a parameter value. Execute the command and see if the the values are accepted in the terminal window.

4. Using this same window, select an Audio sample, such as Bark, and enter a value 1-14 and see if it plays through your speakers. If it does not, make sure the speaker is selected correctly in your Ubuntu Settings, and test with the VLC player GUI interface.