import * as cdk from 'aws-cdk-lib';
import { Construct } from 'constructs';
import { FWBuildConstruct } from './fw-build/fw-build';
import { SecretsConstruct } from './secrets/secrets';
import { MlConstruct } from './ml/ml';
import { CredsProviderConstruct } from './creds-provider/creds-provider';

export class DemoCdkStack extends cdk.Stack {
  constructor(scope: Construct, id: string, props?: cdk.StackProps) {
    super(scope, id, props);
    console.log(cdk.Stack.of(this).region,'cdk.Stack.of(this).region22')

    const secretsConstruct = new SecretsConstruct(this, 'SecretsConstruct');
    new FWBuildConstruct(this, 'FWBuildConstruct');
    const mlConstruct = new MlConstruct(this, 'MlConstruct', secretsConstruct.s3ApiKeySecret);
    new CredsProviderConstruct(this, 'CredsProviderConstruct', secretsConstruct.s3ApiKeySecret, mlConstruct.retrainUrl);
  }
}
