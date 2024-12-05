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
import { CredsProviderResourcesLambdaConstruct } from './creds-provider-resources-lambda';
import { Construct } from 'constructs';

export class CredsProviderConstruct extends Construct {
    constructor(scope: Construct, id: string, s3ApiKeySecret: aws_secretsmanager.Secret, retrainUrl: string) {
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
                'application/json': 'Action=SendMessage&MessageBody=$input.body',
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
                IOTCONNECT_SOLUTION_KEY: iotConnectSolutionKey,
                IOTCONNECT_ENTITY: iotConnectEntity,
                S3_ENDPOINT: retrainUrl,
                S3_KEY_SECRET_NAME: s3ApiKeySecret.secretName,
                REGION: cdk.Stack.of(this).region
            },
            layers: [credsProviderLayer]
        });

        s3ApiKeySecret.grantRead(credsProviderLambdaRole)

        credsProviderLambda.addEventSource(sqsEventSource);

        // Create API Gateway for basic authentication
        const apiGateway = new aws_apigateway.RestApi(this, 'CredentialsWebhook', {
            restApiName: 'CretsWebhookApi',
            description: 'Webhook to provide credentials to the IoT Connect device.',
        });

        const webhookResource = apiGateway.root.addResource('webhook');
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
        
        const webhookResourceEndpoint = apiGateway.url.slice(0, -1) + webhookResource.path; 
        new cdk.CfnOutput(this, 'webhookResourceEndpoint', {
            value: `${webhookResourceEndpoint}`,
            description: 'The URL for the webhookResource',
        });

        // Create additional resources
        const CredsProviderResourcesLambda = new CredsProviderResourcesLambdaConstruct(this, 'CredsProviderResourcesLambdaConstruct', s3ApiKeySecret, webhookResourceEndpoint);
        CredsProviderResourcesLambda.node.addDependency(apiGateway)
        CredsProviderResourcesLambda.node.addDependency(s3ApiKeySecret)
    }
}
