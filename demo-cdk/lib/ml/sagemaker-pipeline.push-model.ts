import { CodeBuild, SSM } from 'aws-sdk';

const { GIT_OWNER, GIT_REPO }: any = process.env;

exports.handler = async (event: any) => {
  console.log(JSON.stringify(event));

  console.log("Push changes to github TBD = " + GIT_OWNER + "/" + GIT_REPO);
};