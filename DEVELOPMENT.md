# Development Setup

## Firmware

* Execute [scripts/setup-project.sh](scripts/setup-project.sh). This script requires dos2unix for applying the patch. This will populate the files from the AWS original repo
and the [ml-source-fsd50k](models/ml-source-fsd50k) model files from the [models](models) directory.
* Open the STM32CubeIDE. When prompted for the workspace path navigate to the [stm32](stm32) directory.
Note that the workspace has to be located there, or otherwise dependencies will not work correctly. 
The stm32 directory name indirection is preserved from the original project in order to make easy transition
to the AWS AI and build framework.
* Select *File -> Open Projects From File System* from the menu. Navigate to the stm32 directory
in this repo and click *Open*.
* Uncheck all folders that appear in the Folders list and leave the *stm32/Projects/b_u585i_iot02s_ntz* directory checked.
* Click *Finish*.

## IoTConnect

Requirements: Python v3.12.

All source code for IoTConnect setup is written in Python and located in "iot-connect" and "demo-cdk/lambdas" directories.

It uses IoTConnect powered by AWS REST API https://docs.iotconnect.io/iotconnect/rest-api/master/?env=uat&pf=aws.

### Local Deployment

If you want to deploy the IoTConnect from your local PC, you will need the following prerequisites.

- **CERTIFICATE**: The certificate you captured from the device (see "Firmware Flashing and Getting Certificate" section in **[Demo Guide](DEMO.md)**).
- **ENTITY_NAME**: Your entity name in the IoTConnect account.
- **PASSWORD**: Your password for the IoTConnect account.
- **SOLUTION_KEY**: Your solution key in the IoTConnect account (see "IoTConnect Setup" section in **[Demo Guide](DEMO.md)**).
- **USERNAME**: Your login for the IoTConnect account.

Go to "iot-connect" directory and from there run the following commands:

- `pip install -r requirements.txt`

- `python ./iot_connect_setup.py "USERNAME" "PASSWORD" "SOLUTION_KEY" "ENTITY_NAME" "CERTIFICATE"`
  example:

  ```
  python .\iot_connect_setup.py "user@example.com" "exAmpLePsswd" "XXXXXXXyyyYYXYXYXYXYXYXYXYXYX" "Sample Company" "-----BEGIN CERTIFICATE-----
  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  -----END CERTIFICATE-----"
  ```

After this steps IoTConnect infrastructure will be partially deployed. The remaining part will be deployed from the AWS during the CDK deployment process.

## AWS

Requirements: Node.js v22, npm, AWS CLI.

This project uses AWS CDK IaC Templates for infrastructure management. All source code is located in the "demo-cdk" and "iot-connect/common" directories.

### Local Deployment

If you want to deploy the AWS CDK from your local PC, you will need the following prerequisites.

- **GIT_ARN**: The ARN of the GitHub connection in AWS (see "GitHub Connection" section in **[Demo Guide](DEMO.md)**).
- **GITHUB_REPOSITORY_OWNER**: The Git Hub repository owner.
- **GIT_REPOSITORY**: The Git Hub repository name without owner.
- **IOT_CONNECT_CERTIFICATE**: The certificate you captured from the device.
- **IOT_CONNECT_ENTITY_NAME**: Your entity name in the IoTConnect account.
- **IOT_CONNECT_PASSWORD**: Your password for the IoTConnect account.
- **IOT_CONNECT_SOLUTION_KEY**: Your solution key in the IoTConnect account.
- **IOT_CONNECT_USERNAME**: Your login for the IoTConnect account.
- **STDEVCLOUD_PASSWORD**: Your password for your [STM32Cube.AI Developer Cloud](https://stm32ai-cs.st.com/home) account (see "ST Account Setup" section in **[Demo Guide](DEMO.md)**).
- **STDEVCLOUD_USERNAME**: Your login for your [STM32Cube.AI Developer Cloud](https://stm32ai-cs.st.com/home) account (see "ST Account Setup" section in **[Demo Guide](DEMO.md)**).
- **TRAINING_SAMPLE_REPEAT_NUMBER**: Number - how many times to you want to put the unrecognized sample to the retraining process.

Configure your AWS Credentials, see https://docs.aws.amazon.com/cli/latest/userguide/getting-started-quickstart.html.

#### Bootstrap the AWS CDK

From the repository root run the following commands

```
mkdir -p "iot-connect-layer/python"
cp -R "iot-connect/common" "iot-connect-layer/python"
```

Go to "demo-cdk" directory and from there run the following commands:

```
npm install
npm i -g aws-cdk
cdk bootstrap aws://AWS_ACCOUNT_NUMBER/AWS_REGION --toolkit-stack-name CDKToolkit-StMicro --qualifier stmicro
```

After the bootstrap is finished, you can deploy the infrastructure.

#### Deploy the AWS Infrastructure

From "demo-cdk" directory run the following commands:

```
cdk synth --context stUsername="STDEVCLOUD_USERNAME" --context stPassword="STDEVCLOUD_PASSWORD" --context iotConnectUsername="IOT_CONNECT_USERNAME" --context iotConnectPassword="IOT_CONNECT_PASSWORD" --context iotConnectSolutionKey="IOT_CONNECT_SOLUTION_KEY" --context iotConnectEntity="IOT_CONNECT_ENTITY_NAME" --context gitOwner="GITHUB_REPOSITORY_OWNER" --context gitRepo="GITHUB_REPOSITORY" --context gitArn="GIT_ARN" --context sampleRptNum="TRAINING_SAMPLE_REPEAT_NUMBER"
cdk deploy --all --context stUsername="STDEVCLOUD_USERNAME" --context stPassword="STDEVCLOUD_PASSWORD" --context iotConnectUsername="IOT_CONNECT_USERNAME" --context iotConnectPassword="IOT_CONNECT_PASSWORD" --context iotConnectSolutionKey="IOT_CONNECT_SOLUTION_KEY" --context iotConnectEntity="IOT_CONNECT_ENTITY_NAME" --context gitOwner="GITHUB_REPOSITORY_OWNER" --context gitRepo="GITHUB_REPOSITORY" --context gitArn="GIT_ARN" --context sampleRptNum="TRAINING_SAMPLE_REPEAT_NUMBER"
```

Example

```
cdk synth --context stUsername="user@example.com" --context stPassword="exAmpLePsswd1" --context iotConnectUsername="user@example.com" --context iotConnectPassword="exAmpLePsswd2" --context iotConnectSolutionKey="XXXXXXXyyyYYXYXYXYXYXYXYXYXYX" --context iotConnectEntity="Sample Company" --context gitOwner="username" --context gitRepo="iotc-freertos-stm32-u5-ml-demo" --context gitArn="arn:aws:codeconnections:us-west-1:000011112222:connection/0000000f-f11f-1111-2222-3b33b3bb3bb3" --context sampleRptNum="5"
cdk deploy --all --context stUsername="user@example.com" --context stPassword="exAmpLePsswd1" --context iotConnectUsername="user@example.com" --context iotConnectPassword="exAmpLePsswd2" --context iotConnectSolutionKey="XXXXXXXyyyYYXYXYXYXYXYXYXYXYX" --context iotConnectEntity="Sample Company" --context gitOwner="username" --context gitRepo="iotc-freertos-stm32-u5-ml-demo" --context gitArn="arn:aws:codeconnections:us-west-1:000011112222:connection/0000000f-f11f-1111-2222-3b33b3bb3bb3" --context sampleRptNum="5"
```

