import { CodeBuild, SSM } from 'aws-sdk';

const codebuild = new CodeBuild();
const ssm = new SSM();

const { assetHash, projectName, paramName, waitCompletionUrl }: any = process.env;

exports.handler = async (event: any) => {
  console.log(JSON.stringify(event));

  await codebuild.startBuild({ projectName }).promise();
};
