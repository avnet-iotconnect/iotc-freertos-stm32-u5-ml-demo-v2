import * as cdk from 'aws-cdk-lib';
import { Construct } from 'constructs';
import { FWBuildConstruct } from './fw-build/fw-build';
import { SecretsConstruct } from './secrets/secrets';

export class DemoCdkStack extends cdk.Stack {
  constructor(scope: Construct, id: string, props?: cdk.StackProps) {
    super(scope, id, props);

    new SecretsConstruct(this, 'SecretsConstruct');
    new FWBuildConstruct(this, 'FWBuildConstruct');
  }
}
