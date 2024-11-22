import * as cdk from 'aws-cdk-lib';
import { Construct } from 'constructs';
import { FWBuildConstruct } from './fw-build/fw-build';
import { SecretsConstruct } from './secrets/secrets';
import { MlConstruct } from './ml/ml';

export class DemoCdkStack extends cdk.Stack {
  constructor(scope: Construct, id: string, props?: cdk.StackProps) {
    super(scope, id, props);

    new SecretsConstruct(this, 'SecretsConstruct');
    new FWBuildConstruct(this, 'FWBuildConstruct');
    new MlConstruct(this, 'MlConstruct');
  }
}
