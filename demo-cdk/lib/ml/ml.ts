// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: MIT-0

import { Construct } from 'constructs';
import { Sagmaker } from './sagemaker';
import { SagmakerPipeline } from './sagemaker-pipeline';


export class MlConstruct extends Construct {
  constructor(scope: Construct, id: string) {
    super(scope, id);

    const { project } = new Sagmaker(this, 'Sagmaker');
    new SagmakerPipeline(this, 'SagemakerPipeline', { project });
  }
}
