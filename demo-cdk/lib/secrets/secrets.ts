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
        const iotConnectUsername = this.node.tryGetContext('iotConnectUsername');
        const iotConnectPassword = this.node.tryGetContext('iotConnectPassword');
        const iotConnectSolutionKey = this.node.tryGetContext('iotConnectSolutionKey');
        const iotConnectEntity = this.node.tryGetContext('iotConnectEntity');

        new aws_secretsmanager.Secret(this, 'stUsernameSecret', {
            secretName: config.stUsernameSecret,
            secretStringValue: SecretValue.unsafePlainText(stUsername),
        });

        new aws_secretsmanager.Secret(this, 'stPasswordSecret', {
            secretName: config.stPasswordSecret,
            secretStringValue: SecretValue.unsafePlainText(stPassword),
        });

        new aws_secretsmanager.Secret(this, 'iotConnectUsernameSecret', {
            secretName: config.iotConnectUsernameSecret,
            secretStringValue: SecretValue.unsafePlainText(iotConnectUsername),
        });

        new aws_secretsmanager.Secret(this, 'iotConnectPasswordSecret', {
            secretName: config.iotConnectPasswordSecret,
            secretStringValue: SecretValue.unsafePlainText(iotConnectPassword),
        });

        new aws_secretsmanager.Secret(this, 'iotConnectSolutionKeySecret', {
            secretName: config.iotConnectSolutionKeySecret,
            secretStringValue: SecretValue.unsafePlainText(iotConnectSolutionKey),
        });

        new aws_secretsmanager.Secret(this, 'iotConnectEntitySecret', {
            secretName: config.iotConnectEntitySecret,
            secretStringValue: SecretValue.unsafePlainText(iotConnectEntity),
        });
    }
}
