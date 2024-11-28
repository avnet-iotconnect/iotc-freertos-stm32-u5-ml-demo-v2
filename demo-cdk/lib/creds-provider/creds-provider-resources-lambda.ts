import * as cdk from 'aws-cdk-lib';
import {
    CfnParameter,
    aws_secretsmanager,
    SecretValue,
    aws_apigateway,
    aws_iam,
    aws_lambda,
    aws_sqs,
    aws_lambda_event_sources,
    custom_resources
  } from 'aws-cdk-lib';
import { Construct } from 'constructs';

export class CredsProviderResourcesLambdaConstruct extends Construct {
    constructor(scope: Construct, id: string, s3ApiKeySecret: aws_secretsmanager.Secret, webhookEndpoint: string) {
        super(scope, id);

        const config = this.node.tryGetContext('config');
        const iotConnectUsername = this.node.tryGetContext('iotConnectUsername');
        const iotConnectPassword = this.node.tryGetContext('iotConnectPassword');
        const iotConnectSolutionKey = this.node.tryGetContext('iotConnectSolutionKey');
        const iotConnectEntity = this.node.tryGetContext('iotConnectEntity');

        const createResourcesLambdaRole = new aws_iam.Role(this, 'CreateResourcesLambdaRole', {
            assumedBy: new aws_iam.ServicePrincipal('lambda.amazonaws.com')
        });

        createResourcesLambdaRole.addManagedPolicy(
            aws_iam.ManagedPolicy.fromAwsManagedPolicyName("service-role/AWSLambdaBasicExecutionRole")
        );

        s3ApiKeySecret.grantWrite(createResourcesLambdaRole);
        s3ApiKeySecret.grantRead(createResourcesLambdaRole);

        // Create the Lambda function for basic authentication
        const createResourcesLayer = new aws_lambda.LayerVersion(this, 'CreateResourcesLayer', {
            code: aws_lambda.Code.fromAsset('../iot-connect-layer'),
            compatibleRuntimes: [aws_lambda.Runtime.PYTHON_3_12]
        });

        // This Lamdba will create additional resources during deploy - Api Key for S3 endpoints and rule in IoTConnect
        const createResourcesLambda = new aws_lambda.Function(this, 'CreateResourcesLambda', {
            runtime: aws_lambda.Runtime.PYTHON_3_12,
            code: aws_lambda.Code.fromAsset(
                'lambdas/create_resources',
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
            handler: 'create_resources.create_resources_handler',
            role: createResourcesLambdaRole,  // Attach the role with the necessary permissions
            memorySize: 128,
            timeout: cdk.Duration.seconds(60),
            environment: {
                IOTCONNECT_USERNAME: iotConnectUsername,
                IOTCONNECT_PASSWORD: iotConnectPassword,
                IOTCONNECT_SOLUTION_KEY: iotConnectSolutionKey,
                IOTCONNECT_ENTITY: iotConnectEntity,
                WEBHOOK_ENDPOINT: webhookEndpoint,
                S3_KEY_SECRET_NAME: s3ApiKeySecret.secretName,
                KEY_PLACEHOLDER: config.s3ApiKeyPlaceHolder,
                AWS_REGION: cdk.Stack.of(this).region;
            },
            layers: [createResourcesLayer]
        });

        const lambdaCustomResource = new custom_resources.AwsCustomResource(this, 'LambdaCustomResource', {
            onCreate: {
                service: 'Lambda',
                action: 'invoke',
                parameters: {
                    FunctionName: createResourcesLambda.functionName,
                    InvocationType: 'RequestResponse',
                },
            },
            onUpdate: {  // Ensure the Lambda is invoked on updates as well
                service: 'Lambda',
                action: 'invoke',
                parameters: {
                  FunctionName: createResourcesLambda.functionName,
                  InvocationType: 'RequestResponse',
                },
            },
            // Define an explicit policy that allows invoking the Lambda function
            policy: custom_resources.AwsCustomResourcePolicy.fromStatements([
                new aws_iam.PolicyStatement({
                actions: ['lambda:InvokeFunction'],
                effect: aws_iam.Effect.ALLOW,
                resources: [createResourcesLambda.functionArn],
                }),
            ]),
            resourceType: 'Custom::LambdaCustomResourceAction',
        });
    }
}
