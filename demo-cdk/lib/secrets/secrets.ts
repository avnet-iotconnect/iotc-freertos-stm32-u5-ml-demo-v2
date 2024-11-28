import * as cdk from 'aws-cdk-lib';
import {
    CfnParameter,
    aws_secretsmanager,
    SecretValue
  } from 'aws-cdk-lib';
import { Construct } from 'constructs';

export class SecretsConstruct extends Construct {
    public readonly s3ApiKeySecret: aws_secretsmanager.Secret;

    constructor(scope: Construct, id: string) {
        super(scope, id);
        const config = this.node.tryGetContext('config');
        const stUsername = this.node.tryGetContext('stUsername');
        const stPassword = this.node.tryGetContext('stPassword');

        new aws_secretsmanager.Secret(this, 'stUsernameSecret', {
            secretName: config.stUsernameSecret,
            secretStringValue: SecretValue.unsafePlainText(stUsername),
        });

        new aws_secretsmanager.Secret(this, 'stPasswordSecret', {
            secretName: config.stPasswordSecret,
            secretStringValue: SecretValue.unsafePlainText(stPassword),
        });

        this.s3ApiKeySecret = new aws_secretsmanager.Secret(this, 's3ApiKeySecret', {
            secretName: config.s3ApiKeySecret,
            secretStringValue: SecretValue.unsafePlainText(config.s3ApiKeyPlaceHolder),
        });
    }
}
