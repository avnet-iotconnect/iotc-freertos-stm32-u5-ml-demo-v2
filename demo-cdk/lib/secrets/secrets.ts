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

        // Define CfnParameters for secrets
        const stUsername = new CfnParameter(this, "stUsername", {
            type: "String",
            description: "The ST Dev Cloud username"});

        const stPassword = new CfnParameter(this, "stPassword", {
            type: "String",
            description: "The ST Dev Cloud password"});

        const iotConnectUsername = new CfnParameter(this, "iotConnectUsername", {
            type: "String",
            description: "The IoTConnect username"});

        const iotConnectPassword= new CfnParameter(this, "iotConnectPassword", {
            type: "String",
            description: "The IoTConnect password"});

        const iotConnectSolutionKey = new CfnParameter(this, "iotConnectSolutionKey", {
            type: "String",
            description: "The IoTConnect solution key"});

        const iotConnectEntity = new CfnParameter(this, "iotConnectEntity", {
            type: "String",
            description: "The IoTConnect Entity"});

        new aws_secretsmanager.Secret(this, 'stUsernameSecret', {
            secretName: config.stUsernameSecret,
            secretStringValue: SecretValue.unsafePlainText(stUsername.valueAsString),
        });

        new aws_secretsmanager.Secret(this, 'stPasswordSecret', {
            secretName: config.stPasswordSecret,
            secretStringValue: SecretValue.unsafePlainText(stPassword.valueAsString),
        });

        new aws_secretsmanager.Secret(this, 'iotConnectUsernameSecret', {
            secretName: config.iotConnectUsernameSecret,
            secretStringValue: SecretValue.unsafePlainText(iotConnectUsername.valueAsString),
        });

        new aws_secretsmanager.Secret(this, 'iotConnectPasswordSecret', {
            secretName: config.iotConnectPasswordSecret,
            secretStringValue: SecretValue.unsafePlainText(iotConnectPassword.valueAsString),
        });

        new aws_secretsmanager.Secret(this, 'iotConnectSolutionKeySecret', {
            secretName: config.iotConnectSolutionKeySecret,
            secretStringValue: SecretValue.unsafePlainText(iotConnectSolutionKey.valueAsString),
        });

        new aws_secretsmanager.Secret(this, 'iotConnectEntitySecret', {
            secretName: config.iotConnectEntitySecret,
            secretStringValue: SecretValue.unsafePlainText(iotConnectEntity.valueAsString),
        });
    }
}
