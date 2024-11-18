import * as cdk from 'aws-cdk-lib';
import { Construct } from 'constructs';
import { aws_lambda_nodejs, aws_lambda } from 'aws-cdk-lib';
import * as iam from 'aws-cdk-lib/aws-iam';
import * as iot from 'aws-cdk-lib/aws-iot';

export class FWBuildConstruct extends Construct {
    constructor(scope: Construct, id: string) {
        super(scope, id);

        // Create a dummy Lambda function
        const lambdaFunctionDummy = new aws_lambda_nodejs.NodejsFunction(this, 'dummy-lambda', {
            runtime: aws_lambda.Runtime.NODEJS_18_X,
            environment: {
                REGION: cdk.Stack.of(this).region
            },
        });

        lambdaFunctionDummy.addToRolePolicy(new iam.PolicyStatement({
            effect: iam.Effect.ALLOW,
            actions: [
                'logs:*'
            ],
            resources: ['*']
        }));
    }
}