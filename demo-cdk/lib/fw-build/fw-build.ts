import * as cdk from 'aws-cdk-lib';
import { Construct } from 'constructs';
import {
    aws_logs,
    aws_codebuild
} from 'aws-cdk-lib';
import { CfnProject, Project, Source, FilterGroup, EventAction, ComputeType } from "aws-cdk-lib/aws-codebuild";
import * as iam from 'aws-cdk-lib/aws-iam';
import * as iot from 'aws-cdk-lib/aws-iot';

export class FWBuildConstruct extends Construct {
    constructor(scope: Construct, id: string) {
        super(scope, id);

        // Fix hardcoded values
        const fwBuild = new aws_codebuild.Project(this, 'FWBuild', {
            projectName: "AvnetStm32FWBuild",
            source: aws_codebuild.Source.gitHub({
              owner: "avnet-iotconnect",
              repo: "iotc-freertos-stm32-u5-ml-demo-v2",
              webhook: true,
              webhookFilters: [FilterGroup.inEventOf(EventAction.PULL_REQUEST_CREATED)],
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
        
        const cfnProject = fwBuild.node.defaultChild as CfnProject
        const triggers = cfnProject.triggers as CfnProject.ProjectTriggersProperty
        const filterGroups = triggers.filterGroups as CfnProject.WebhookFilterProperty[][]
        // remove wrong (PULL_REQUEST_CREATED) filter
        filterGroups[0].pop()
        // add correct (WORKFLOW_JOB_QUEUED) filter
        filterGroups[0].push({
            type: 'EVENT',
            pattern: 'WORKFLOW_JOB_QUEUED',
        })
    }
}