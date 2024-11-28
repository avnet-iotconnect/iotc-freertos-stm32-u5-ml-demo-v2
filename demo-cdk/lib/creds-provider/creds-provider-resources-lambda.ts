import * as cdk from 'aws-cdk-lib';
import {
    CfnParameter,
    aws_secretsmanager,
    SecretValue,
    aws_apigateway,
    aws_iam,
    aws_lambda,
    aws_sqs,
    aws_lambda_event_sources
  } from 'aws-cdk-lib';
import { Construct } from 'constructs';

export class CredsProviderResourcesLambdaConstruct extends Construct {
    constructor(scope: Construct, id: string, webhookEndpoint: string) {
        super(scope, id);

        const iotConnectUsername = this.node.tryGetContext('iotConnectUsername');
        const iotConnectPassword = this.node.tryGetContext('iotConnectPassword');
        const iotConnectSolutionKey = this.node.tryGetContext('iotConnectSolutionKey');
        const iotConnectEntity = this.node.tryGetContext('iotConnectEntity');

        

        // This Lamdba will create additional resources during deploy - Api Key for S3 endpoints and rule in IoTConnect
        const createResourcesLambdaLambda = new aws_lambda.Function(this, 'CredsProviderLambda', {
            runtime: aws_lambda.Runtime.PYTHON_3_12,
            code: aws_lambda.Code.fromAsset(
                'lambdas/creds_provider',
                {
                    bundling: {
                    image: aws_lambda.Runtime.PYTHON_3_12.bundlingImage,
                    command: [
                        'bash', '-c',
                        'pip install -r requirements.txt -t /asset-output && cp -au . /asset-output'
                    ],
                    },
                }
            ),
            handler: 'creds_provider.creds_provider_handler',
            role: credsProviderLambdaRole,  // Attach the role with the necessary permissions
            memorySize: 128,
            timeout: cdk.Duration.seconds(60),
            environment: {
                IOTCONNECT_USERNAME: iotConnectUsername,
                IOTCONNECT_PASSWORD: iotConnectPassword,
                IOTCONNECT_SOLUTION_KEY: iotConnectSolutionKey
            },
            layers: [credsProviderLayer]
        });
}
