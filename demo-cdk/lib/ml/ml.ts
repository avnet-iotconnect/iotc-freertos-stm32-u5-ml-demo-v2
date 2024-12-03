// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: MIT-0

import { Construct } from 'constructs';
import { Sagmaker } from './sagemaker';
import { SagmakerPipeline } from './sagemaker-pipeline';
import {  aws_secretsmanager, } from 'aws-cdk-lib';


export class MlConstruct extends Construct {
  public readonly retrainUrl: string;
  constructor(scope: Construct, id: string, s3ApiKeySecret: aws_secretsmanager.Secret) {
    super(scope, id);

    const { project } = new Sagmaker(this, 'Sagmaker');
    const sagmakerPipeline = new SagmakerPipeline(this, 'SagemakerPipeline', { project }, s3ApiKeySecret);
    this.retrainUrl = sagmakerPipeline.retrainUrl;
  }
}
