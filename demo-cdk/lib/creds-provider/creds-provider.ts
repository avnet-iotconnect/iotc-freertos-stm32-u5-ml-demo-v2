import * as cdk from 'aws-cdk-lib';
import {
    CfnParameter,
    aws_secretsmanager,
    SecretValue,
    aws_apigateway,
    aws_iam,
    aws_lambda
  } from 'aws-cdk-lib';
import { Construct } from 'constructs';

export class CredsProviderConstruct extends Construct {
    constructor(scope: Construct, id: string) {
        super(scope, id);

        const iotConnectUsername = this.node.tryGetContext('iotConnectUsername');
        const iotConnectPassword = this.node.tryGetContext('iotConnectPassword');
        const iotConnectSolutionKey = this.node.tryGetContext('iotConnectSolutionKey');
        const iotConnectEntity = this.node.tryGetContext('iotConnectEntity');

        const credsProviderLambdaRole = new aws_iam.Role(this, 'CredsProviderLambdaRole', {
            assumedBy: new aws_iam.ServicePrincipal('lambda.amazonaws.com')
        });

        credsProviderLambdaRole.addManagedPolicy(
            aws_iam.ManagedPolicy.fromAwsManagedPolicyName("service-role/AWSLambdaBasicExecutionRole")
        );

        // Create the Lambda function for basic authentication
        const credsProviderLambda = new aws_lambda.Function(this, 'CredsProviderLambda', {
            runtime: aws_lambda.Runtime.PYTHON_3_12,
            code: aws_lambda.Code.fromAsset('../../iot-connect'),
            handler: 'creds_provider.creds_provider_handler',
            role: credsProviderLambdaRole,  // Attach the role with the necessary permissions
            memorySize: 128,
            timeout: cdk.Duration.seconds(2),
            environment: {
                IOTCONNECT_USERNAME: iotConnectUsername,
                IOTCONNECT_PASSWORD: iotConnectPassword,
                IOTCONNECT_SOLUTION_KEY: iotConnectSolutionKey
            },
        });

        // // Create API Gateway for basic authentication
        // const apiGateway = new aws_apigateway.RestApi(this, 'CredentialsWebhook', {
        //     restApiName: 'CretsWebhookApi',
        //     description: 'Webhook to provide credentials to the IoT Connect device.',
        // });

        // const webhookResource = apiGateway.root.addResource('webhook');
        // // Lambda integration for basic auth
        // const lambdaIntegration = new aws_apigateway.LambdaIntegration();

        // webhookResource.addMethod('POST', lambdaIntegration, {
        //     authorizationType: aws_apigateway.AuthorizationType.NONE,
        // });
    }
}
