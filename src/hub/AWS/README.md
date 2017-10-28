# Alexa Integration with AWS Lambda

> Note: The info here assumes an IoT Shadow Device already exists

## 1. Script configuration

Before the script is uploaded, the `Shadow Thing Name` and `Region` must be configured. The `Shadow Thing Name` is the name of the shadow thing created on AWS *(ex: lux-alexa)*. The region is the region in which the shadow exists *(ex: us-east-2)*.

## 2. Script packaging

Because this script depends on an external library `boto3`, this must be packaged with the script when uploaded.

The latest release can be found [here](https://github.com/boto/boto3/releases).

1. Download `boto3` and place it in the root directory of `AWS`. There should now be a path `AWS/boto3`.
2. Zip `boto3` and `lux_lambda.py`. There should be no root folder in the zip. This can be achieved on Windows by shift-clicking the `boto3` directory and the `lux_lambda.py` scripts, right clicking, and selecting `Send to > Compressed (zipped) folder`.
3. Upload to AWS Lambda via the `Upload a .ZIP file` option.
4. Ensure the runtime is set to `Python 3.6`
5. Set the handler to `lux_lambda.lambda_handler`

## 3. Configuring Alexa

1. Add the skills `LuxTurnOn` and `LuxTurnOff` with parameter `Light`
2. Add the utterances specified in `utterances.txt`
