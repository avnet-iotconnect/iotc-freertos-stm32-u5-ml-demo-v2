// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: MIT-0

import { Construct } from 'constructs';
import  * as cdk from 'aws-cdk-lib';
import {
  SecretValue,
  Stack,
  RemovalPolicy,
  aws_s3,
  aws_secretsmanager,
  aws_iam,
  aws_s3_assets,
  aws_codebuild,
  aws_kms,
  aws_logs,
  aws_sagemaker,
  Duration,
  aws_lambda_nodejs,
  aws_lambda,
  aws_cloudformation,
  aws_events_targets,
  triggers,
  aws_ssm,
  Token,
} from 'aws-cdk-lib';
import { ComputeType } from 'aws-cdk-lib/aws-codebuild';
import * as apigatewayv2 from 'aws-cdk-lib/aws-apigatewayv2';
import * as integrations from 'aws-cdk-lib/aws-apigatewayv2-integrations';
import * as logs from 'aws-cdk-lib/aws-logs';

type SageMakerPipelineProps = {
  project: aws_sagemaker.CfnProject;
};

export class SagmakerPipeline extends Construct {
  public readonly retrainUrl: string;
  constructor(scope: Construct, id: string, props: SageMakerPipelineProps, s3ApiKeySecret: aws_secretsmanager.Secret) {
    super(scope, id);
    const { project } = props;

    const config = this.node.tryGetContext('config');
    // const { region } = Stack.of(this);
    console.log(cdk.Stack.of(this).region,'cdk.Stack.of(this).region')
    const region = cdk.Stack.of(this).region;

    const mlOutputBucket = new aws_s3.Bucket(this, 'MlOutput', {
      versioned: true,
      enforceSSL: true,
      encryption: aws_s3.BucketEncryption.S3_MANAGED,
      blockPublicAccess: aws_s3.BlockPublicAccess.BLOCK_ALL,
      removalPolicy: RemovalPolicy.DESTROY,
      autoDeleteObjects: true,
      serverAccessLogsBucket: new aws_s3.Bucket(this, 'MlOutputAccessLog', {
        versioned: true,
        enforceSSL: true,
        encryption: aws_s3.BucketEncryption.S3_MANAGED,
        blockPublicAccess: aws_s3.BlockPublicAccess.BLOCK_ALL,
        autoDeleteObjects: true,
        removalPolicy: RemovalPolicy.DESTROY,
      }),
    });

    const mlOpsCode = new aws_s3_assets.Asset(this, 'MlOpsAsset', {
      path: 'mlops',
    });

    const dataSetsBucket = new aws_s3.Bucket(this, 'DataSetsBucket', {
      versioned: true,
      enforceSSL: true,
      encryption: aws_s3.BucketEncryption.S3_MANAGED,
      blockPublicAccess: aws_s3.BlockPublicAccess.BLOCK_ALL,
      removalPolicy: RemovalPolicy.DESTROY,
      autoDeleteObjects: true,
      serverAccessLogsBucket: new aws_s3.Bucket(this, 'DataSetsAccessLog', {
        versioned: true,
        enforceSSL: true,
        encryption: aws_s3.BucketEncryption.S3_MANAGED,
        blockPublicAccess: aws_s3.BlockPublicAccess.BLOCK_ALL,
        autoDeleteObjects: true,
        removalPolicy: RemovalPolicy.DESTROY,
      }),
    });

    const sageMakerPipelineRole = new aws_iam.Role(this, 'ServiceCatalogProductRole', {
      assumedBy: new aws_iam.CompositePrincipal(
        new aws_iam.ServicePrincipal('sagemaker.amazonaws.com')
      ),
      managedPolicies: [
        aws_iam.ManagedPolicy.fromAwsManagedPolicyName('AmazonSageMakerFullAccess'),
        aws_iam.ManagedPolicy.fromAwsManagedPolicyName('SecretsManagerReadWrite'),
      ],
    });
    dataSetsBucket.grantReadWrite(sageMakerPipelineRole);

    const buildEncryptionKey = new aws_kms.Key(this, 'BuildEncryptionKey', {
      removalPolicy: RemovalPolicy.DESTROY,
      enableKeyRotation: true,
    });

    const build = new aws_codebuild.Project(this, 'MlBuild', {
      environment:  {
        computeType: ComputeType.X_LARGE,
        buildImage: aws_codebuild.LinuxBuildImage.STANDARD_7_0
      },
      timeout: Duration.minutes(120),
      encryptionKey: buildEncryptionKey,
      source: aws_codebuild.Source.s3({
        bucket: mlOpsCode.bucket,
        path: mlOpsCode.s3ObjectKey,
      }),
      artifacts: aws_codebuild.Artifacts.s3({
        bucket: mlOutputBucket,
        includeBuildId: false,
        packageZip: false,
      }),
      environmentVariables: {
        SAGEMAKER_PROJECT_NAME: { value: project.projectName },
        SAGEMAKER_PROJECT_ID: { value: project.attrProjectId },
        ARTIFACT_BUCKET: { value: dataSetsBucket.bucketName },
        SAGEMAKER_PIPELINE_NAME: { value: 'sagemaker-' + project.projectName },
        SAGEMAKER_PIPELINE_ROLE_ARN: { value: sageMakerPipelineRole.roleArn },
        AWS_REGION: { value: region },
        STDEVCLOUD_USERNAME_SECRET: { value: config.stUsernameSecret },
        STDEVCLOUD_PASSWORD_SECRET: { value: config.stPasswordSecret },
      },
      buildSpec: aws_codebuild.BuildSpec.fromAsset('lib/ml/buildspec.yml'),
      logging: {
        cloudWatch: {
          logGroup: new aws_logs.LogGroup(this, `MlBuildLogGroup`),
        },
      },
    });
    dataSetsBucket.grantReadWrite(build);
    mlOpsCode.bucket.grantReadWrite(build);
    build.role?.addManagedPolicy(
      aws_iam.ManagedPolicy.fromAwsManagedPolicyName('AmazonSageMakerFullAccess')
    );

    const retrainTriggerRole = new aws_iam.Role(this, 'RetrainTriggerRole', {
      assumedBy: new aws_iam.ServicePrincipal('lambda.amazonaws.com'),
      managedPolicies: [
        aws_iam.ManagedPolicy.fromAwsManagedPolicyName("service-role/AWSLambdaBasicExecutionRole"),
        aws_iam.ManagedPolicy.fromAwsManagedPolicyName('AmazonSSMFullAccess'),
        aws_iam.ManagedPolicy.fromAwsManagedPolicyName('AWSCodeBuildAdminAccess'),
        aws_iam.ManagedPolicy.fromAwsManagedPolicyName('AmazonS3FullAccess'),
      ],
    });

    const retrainTriggerFn = new aws_lambda_nodejs.NodejsFunction(this, 'retrain-trigger', {
      runtime: aws_lambda.Runtime.NODEJS_18_X,
      environment: {
        projectName: build.projectName,
        S3_KEY_SECRET_NAME: s3ApiKeySecret.secretName,
        datasetsBucket: dataSetsBucket.bucketName,
        REGION: cdk.Stack.of(this).region,
      },
      bundling: {
        esbuildArgs: {
          '--packages': 'bundle',
        },
      },
      role: retrainTriggerRole,
    });

    s3ApiKeySecret.grantRead(retrainTriggerRole);

    const httpApi = new apigatewayv2.HttpApi(this, 'HttpApi', {
      apiName: 'RetrainTriggerApi',
    });

    const retrainPath = '/retrain_trigger';
    httpApi.addRoutes({
      path: retrainPath,
      methods: [apigatewayv2.HttpMethod.POST],
      integration: new integrations.HttpLambdaIntegration(
          'RetrainTriggerIntegration',
          retrainTriggerFn
      ),
    });

    this.retrainUrl = Token.asString(httpApi.url).slice(0, -1) + retrainPath;

    // const key = new aws_kms.Key(this, 'KMS', {
    //   removalPolicy: RemovalPolicy.DESTROY,
    //   enableKeyRotation: true,
    // });
    // const mlBucketSecret = new aws_secretsmanager.Secret(this, 'MlOutputSecret', {
    //   secretName: 'MlBucketArn',
    //   secretStringValue: SecretValue.unsafePlainText(mlOutputBucket.bucketArn),
    //   encryptionKey: key,
    // });

    const pushModelRole = new aws_iam.Role(this, 'PushModelRole', {
        assumedBy: new aws_iam.ServicePrincipal('lambda.amazonaws.com'),
        managedPolicies: [
          aws_iam.ManagedPolicy.fromAwsManagedPolicyName("service-role/AWSLambdaBasicExecutionRole"),
          aws_iam.ManagedPolicy.fromAwsManagedPolicyName('AmazonS3FullAccess'),
        ],
    });

    const gitOwner = this.node.tryGetContext('gitOwner');
    const gitRepo = this.node.tryGetContext('gitRepo');

    // Lambda to push ml to the github repo
    const pushModelFn = new aws_lambda_nodejs.NodejsFunction(this, 'push-model', {
        runtime: aws_lambda.Runtime.NODEJS_18_X,
        environment: {
          GIT_OWNER: gitOwner,
          GIT_REPO: gitRepo,
        },
        bundling: {
          esbuildArgs: {
            '--packages': 'bundle',
          },
        },
        role: pushModelRole,
    });

    build.onBuildSucceeded('BuildSucceed', {
      target: new aws_events_targets.LambdaFunction(pushModelFn),
    });

    new triggers.Trigger(this, 'BuildTrigger', {
      handler: pushModelFn,
      invocationType: triggers.InvocationType.EVENT,
      executeAfter: [build],
    });

    const gitArn = this.node.tryGetContext('gitArn');

    // Fix hardcoded values
    const pushCode = new aws_codebuild.Project(this, 'FWBuild', {
        projectName: "PushCode",
        source: aws_codebuild.Source.gitHub({
            owner: gitOwner,
            repo: gitRepo,
        }),
        environmentVariables: {
            ML_OUTPUT_BUCKET: { value: mlOutputBucket.bucketName },
        },
        environment:  {
            computeType: ComputeType.SMALL,
            buildImage: aws_codebuild.LinuxBuildImage.STANDARD_7_0
        },
        buildSpec: aws_codebuild.BuildSpec.fromObject({
            version: '0.2',
            artifacts: {
              files: ['Projects/b_u585i_iot02a_ntz/Debug/b_u585i_iot02a_ntz.bin'],
            },
            phases: {
                build: {
                  commands: [
                    'ls -la',
                    'aws s3 cp s3://${ML_OUTPUT_BUCKET}/ml/tmp/ml/ newml --recursive --quiet',
                    'ls -la newml',
                    'git checkout retrained-model 2>/dev/null || git checkout -b retrained-model',
                    'git add --all',
                    'git commit -m "retrained model"',
                    'git push -u origin retrained-model'
                  ]
                }
            },
        }),
        logging: {
            cloudWatch: {
            logGroup: new aws_logs.LogGroup(this, `PushCodeLogGroup`),
            },
        },
    });

    const codeConnectionPolicy = new aws_iam.Policy(this, 'CodeConnectionPolicy', {
        statements: [
            new aws_iam.PolicyStatement({
            actions: ['codeconnections:GetConnectionToken', 'codeconnections:GetConnection'],
            resources: [gitArn],
            }),
        ],
    });
    pushCode.role!.attachInlinePolicy(codeConnectionPolicy);
    // create the policy before the project
    (pushCode.node.defaultChild as cdk.CfnResource).addDependency(codeConnectionPolicy.node.defaultChild as cdk.CfnResource);
  }
}