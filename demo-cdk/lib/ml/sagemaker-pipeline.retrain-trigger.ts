import { CodeBuild, SSM, SecretsManager } from 'aws-sdk';
import { S3Client, PutObjectCommand, GetObjectCommand } from '@aws-sdk/client-s3';

//change to real values
const datasetPath = 'train/datasets/FSD50K/';

// const apiKey = 'test-api-key';

const codebuild = new CodeBuild();
const ssm = new SSM();

const { projectName, S3_KEY_SECRET_NAME, REGION, datasetsBucket }: any = process.env;
const s3 = new S3Client({ region: REGION });
const secretsManager = new SecretsManager();


const createWavHeader = (dataLength: number, sampleRate = 16000, numChannels = 1, bitDepth = 16) => {
  const header = Buffer.alloc(44);

  // "RIFF" chunk descriptor
  header.write('RIFF', 0);
  header.writeUInt32LE(36 + dataLength, 4); // Chunk size: 36 + data size
  header.write('WAVE', 8);

  // "fmt " sub-chunk
  header.write('fmt ', 12);
  header.writeUInt32LE(16, 16); // Subchunk size: 16 for PCM
  header.writeUInt16LE(1, 20); // Audio format: 1 for PCM
  header.writeUInt16LE(numChannels, 22); // Number of channels
  header.writeUInt32LE(sampleRate, 24); // Sample rate
  header.writeUInt32LE(sampleRate * numChannels * (bitDepth / 8), 28); // Byte rate
  header.writeUInt16LE(numChannels * (bitDepth / 8), 32); // Block align
  header.writeUInt16LE(bitDepth, 34); // Bits per sample

  // "data" sub-chunk
  header.write('data', 36);
  header.writeUInt32LE(dataLength, 40); // Data chunk size

  return header;
};

exports.handler = async (event: any) => {
  try {
    const apiKeyValue = await secretsManager.getSecretValue({
        SecretId: S3_KEY_SECRET_NAME
    }).promise()
    const apiKey = apiKeyValue.SecretString
    console.log('API Key ' + apiKey);
    if (
        !event.headers['x-api-key'] ||
        event.headers['x-api-key'] !== apiKey
    ) {
        return {
            statusCode: 403,
            body: JSON.stringify({ message: 'Forbidden' }),
        };
    }
    const contentType = event.headers['content-type'] || event.headers['Content-Type'];
    const soundClasses = event.headers['sound-classes'];

    if (!contentType || !soundClasses || soundClasses.length === 0 || contentType !== 'audio/wav') {
      return {
        statusCode: 400,
        body: JSON.stringify({ message: 'Invalid' }),
      };
    }


    const isBase64Encoded = event.isBase64Encoded || false; // Confirm if payload is Base64
    const audioFile = isBase64Encoded
        ? Buffer.from(event.body, 'base64') // Decode Base64 to binary
        : event.body;

    const timestamp = Date.now();
    const audioFileName = `${timestamp}.wav`;

    // Helper function to fetch file from S3
    const fetchFromS3 = async (key: string): Promise<string> => {
        const params = { Bucket: datasetsBucket, Key: `${datasetPath}${key}` };
        const data = await s3.send(new GetObjectCommand(params));
        return new Promise((resolve, reject) => {
            let csvData = '';
            // @ts-ignore
            data.Body.on('data', (chunk) => {
              csvData += chunk.toString('utf-8');
            });
            // @ts-ignore
            data.Body.on('end', () => resolve(csvData));
            // @ts-ignore
            data.Body.on('error', (err) => reject(err));
          });
    };

    // 1. Load vocabulary.csv and dev.csv
    const vocabularyData = await fetchFromS3('FSD50K.ground_truth/vocabulary.csv');
    const devData = await fetchFromS3('FSD50K.ground_truth/dev.csv');

    // Parse vocabulary.csv
    const vocabulary: Record<string, string> = {};
    vocabularyData.split('\n').forEach((line) => {
        const [id, label, mid] = line.split(',');
        vocabulary[label] = mid;
    });

    // Validate sound-classes
    const labels = soundClasses.split(',');
    const mids = [];
    for (const label of labels) {
        if (!vocabulary[label]) {
        throw new Error(`Label "${label}" not found in vocabulary`);
        }
        mids.push(vocabulary[label]);
    }

    const wavHeader = createWavHeader(audioFile.length);
    const wavFile = Buffer.concat([wavHeader, audioFile]);

    // Save audio file to S3
    const audioParams = {
        Bucket: datasetsBucket,
        Key: `${datasetPath}FSD50K.dev_audio/${audioFileName}`,
        Body: wavFile,
        ContentType: contentType,
    };
    await s3.send(new PutObjectCommand(audioParams));

    // Compose new row for dev.csv
    const newRow = `${timestamp},"${soundClasses}","${mids.join(',')}",train\n`;

    // Append new row to dev.csv
    const updatedDevData = devData + newRow;
    const devParams = {
        Bucket: datasetsBucket,
        Key: `${datasetPath}FSD50K.ground_truth/dev.csv`,
        Body: updatedDevData,
    };
    
    await s3.send(new PutObjectCommand(devParams));
    await codebuild.startBuild({ projectName }).promise();
    return {
        statusCode: 200,
        body: JSON.stringify({ message: 'File uploaded successfully'}),
    };
  } catch (error) {
      console.log('Error ');
      console.error('Error uploading file:', error);
      return {
          statusCode: 500,
          body: JSON.stringify({ message: 'Internal server error' }),
      };
  }
};
