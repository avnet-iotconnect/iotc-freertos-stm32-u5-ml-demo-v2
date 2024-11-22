import * as cdk from 'aws-cdk-lib';
import { Construct } from 'constructs';
import {
    aws_iam,
    aws_logs,
    aws_codebuild
} from 'aws-cdk-lib';
import { CfnProject, EventAction, FilterGroup, ComputeType } from "aws-cdk-lib/aws-codebuild";
import * as iam from 'aws-cdk-lib/aws-iam';
import * as iot from 'aws-cdk-lib/aws-iot';

export class FWBuildConstruct extends Construct {
    constructor(scope: Construct, id: string) {
        super(scope, id);

        const gitOwner = this.node.tryGetContext('gitOwner');
        const gitRepo = this.node.tryGetContext('gitRepo');
        const gitArn = this.node.tryGetContext('gitArn');

        // Fix hardcoded values
        const fwBuild = new aws_codebuild.Project(this, 'FWBuild', {
            projectName: "AvnetStm32FWBuild",
            source: aws_codebuild.Source.gitHub({
              owner: gitOwner,
              repo: gitRepo,
              webhook: true,
              webhookFilters: [FilterGroup.inEventOf(EventAction.WORKFLOW_JOB_QUEUED)],
            }),
            environment: {
              buildImage: aws_codebuild.LinuxBuildImage.fromDockerRegistry(
                'public.ecr.aws/y2t8c1e9/cube_ide_image:latest'
              ),
              computeType: ComputeType.SMALL,
            },
            logging: {
              cloudWatch: {
                logGroup: new aws_logs.LogGroup(this, `IoTBuildLogGroup`),
              },
            },
          });
        
        // fwBuild.role!.addManagedPolicy(aws_iam.ManagedPolicy.fromAwsManagedPolicyName('AdministratorAccess'));

        fwBuild.addToRolePolicy(new iam.PolicyStatement({
            effect: iam.Effect.ALLOW,
            resources: [gitArn],
            actions: ["codeconnections:GetConnectionToken", "codeconnections:GetConnection"],
        }));

        (fwBuild.node.defaultChild as CfnProject).addPropertyOverride('Source.Auth', {
            Type: 'CODECONNECTIONS',
            Resource:
                gitArn,
        })
    }
}