import * as cdk from 'aws-cdk-lib';
import { Construct } from 'constructs';
import { FWBuildConstruct } from './fw-build/fw-build';

export class DemoCdkStack extends cdk.Stack {
  constructor(scope: Construct, id: string, props?: cdk.StackProps) {
    super(scope, id, props);

    new FWBuildConstruct(this, 'FWBuildConstruct');
  }
}
