#!/usr/bin/env node
import 'source-map-support/register';
import * as cdk from 'aws-cdk-lib';
import { DemoCdkStack } from '../lib/demo-cdk-stack';

console.log(process.env.CDK_DEFAULT_ACCOUNT,'process.env.CDK_DEFAULT_ACCOUNT')

const app = new cdk.App();
new DemoCdkStack(app, 'DemoCdkStack', {
  env: { account: process.env.CDK_DEFAULT_ACCOUNT, region: process.env.CDK_DEFAULT_REGION },
});