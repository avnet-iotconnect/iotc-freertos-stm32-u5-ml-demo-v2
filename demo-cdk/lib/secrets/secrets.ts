import * as cdk from 'aws-cdk-lib';
import {
    CfnParameter,
    aws_secretsmanager,
    SecretValue
  } from 'aws-cdk-lib';
import { Construct } from 'constructs';

export class SecretsConstruct extends Construct {
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
    }
}
