import { CodeBuild } from 'aws-sdk';

const codebuild = new CodeBuild();

const { projectName }: any = process.env;

exports.handler = async (event: any) => {
  console.log(JSON.stringify(event));

  await codebuild.startBuild({ projectName }).promise();
};