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
import { isProxy } from 'util/types';

export class CredsProviderConstruct extends Construct {
    constructor(scope: Construct, id: string) {
        super(scope, id);

        const iotConnectUsername = this.node.tryGetContext('iotConnectUsername');
        const iotConnectPassword = this.node.tryGetContext('iotConnectPassword');
        const iotConnectSolutionKey = this.node.tryGetContext('iotConnectSolutionKey');
        const iotConnectEntity = this.node.tryGetContext('iotConnectEntity');

        const apiQueue = new aws_sqs.Queue(
            this, "ApigwSqsLambdaQueue",
            {
                visibilityTimeout: cdk.Duration.seconds(300)
            }
        );

        const sqsEventSource = new aws_lambda_event_sources.SqsEventSource(apiQueue);

        const apiGatewaySqsRole = new aws_iam.Role(this, 'ApiGatewaySqsRole', {
            assumedBy: new aws_iam.ServicePrincipal('apigateway.amazonaws.com')
        });

        apiGatewaySqsRole.addManagedPolicy(
            aws_iam.ManagedPolicy.fromManagedPolicyArn(
                this, 'ApiGwPushCwPolicy', 'arn:aws:iam::aws:policy/service-role/AmazonAPIGatewayPushToCloudWatchLogs'
            )
        )

        apiGatewaySqsRole.attachInlinePolicy(
            new aws_iam.Policy(this, 'apiGatewaySqsRoleInlinePolicy',
                {
                    statements: [
                        new aws_iam.PolicyStatement({
                            effect: aws_iam.Effect.ALLOW,
                            actions: [
                                'sqs:SendMessage',
                                'sqs:ReceiveMessage',
                                'sqs:PurgeQueue',
                                'sqs:DeleteMessage',
                            ],
                            resources: [
                                apiQueue.queueArn
                            ]
                        })
                    ]
                }
            )
        )

        const sqsIntegration = new aws_apigateway.AwsIntegration({
            service: 'sqs',
            path: `${process.env.CDK_DEFAULT_ACCOUNT}/${apiQueue.queueName}`,
            integrationHttpMethod: 'POST',
            options: {
              credentialsRole: apiGatewaySqsRole,
              requestParameters: {
                'integration.request.header.Content-Type': `'application/x-www-form-urlencoded'`,
              },
              requestTemplates: {
                'application/json': `Action=SendMessage&MessageBody=$util.urlEncode("$method.request.querystring.message")`,
              },
              integrationResponses: [
                {
                  statusCode: '200',
                },
                {
                  statusCode: '400',
                },
                {
                  statusCode: '500',
                }
              ]
            },
          });

        const credsProviderLambdaRole = new aws_iam.Role(this, 'CredsProviderLambdaRole', {
            assumedBy: new aws_iam.ServicePrincipal('lambda.amazonaws.com')
        });

        credsProviderLambdaRole.addManagedPolicy(
            aws_iam.ManagedPolicy.fromAwsManagedPolicyName("service-role/AWSLambdaBasicExecutionRole")
        );

        // Create the Lambda function for basic authentication
        const credsProviderLayer = new aws_lambda.LayerVersion(this, 'CredsProviderLayer', {
            code: aws_lambda.Code.fromAsset('../iot-connect-layer'),
            compatibleRuntimes: [aws_lambda.Runtime.PYTHON_3_12]
        });

        const credsProviderLambda = new aws_lambda.Function(this, 'CredsProviderLambda', {
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

        credsProviderLambda.addEventSource(sqsEventSource);

        // Create API Gateway for basic authentication
        const apiGateway = new aws_apigateway.RestApi(this, 'CredentialsWebhook', {
            restApiName: 'CretsWebhookApi',
            description: 'Webhook to provide credentials to the IoT Connect device.',
        });

        const webhookResource = apiGateway.root.addResource('webhook');
        // Lambda integration for basic auth
        // const lambdaIntegration = new aws_apigateway.LambdaIntegration(
        //     credsProviderLambda,
        //     {
        //         proxy: true
        //     }
        // );

        // webhookResource.addMethod('POST', lambdaIntegration, {
        //     authorizationType: aws_apigateway.AuthorizationType.NONE,
        // });

        webhookResource.addMethod('POST', sqsIntegration, {
            methodResponses: [
                {
                    statusCode: '400',
                },
                { 
                    statusCode: '200',
                },
                {
                    statusCode: '500',
                }
            ]
        });
    }
}
