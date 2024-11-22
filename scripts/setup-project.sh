#!/bin/bash

set -e

this_dir=$(dirname "${0}")

cd ${this_dir}/..

mlpath=./models/ml-source-ablrv

wsdir=./stm32

rm -rf aws-stm32-ml-at-edge-accelerator
echo Clone aws-stm32 repo
git clone https://github.com/aws-samples/aws-stm32-ml-at-edge-accelerator
pushd aws-stm32-ml-at-edge-accelerator >/dev/null
  git reset --hard b8271253a2d811a6db184fef35fcdd94d505b848
popd >/dev/null

cp -rn aws-stm32-ml-at-edge-accelerator/stm32/* ./stm32/
rm -rf aws-stm32-ml-at-edge-accelerator

mkdir -p ./stm32/Middleware/STM32_AI_Library/Inc
mkdir -p ./stm32/Middleware/STM32_AI_Library/Lib

echo Applying the model at ${mlpath}...
cp -rf ${mlpath}/stm32ai_files/Inc/* ./stm32/Middleware/STM32_AI_Library/Inc/
cp -f ${mlpath}/stm32ai_files/Lib/NetworkRuntime730_CM33_GCC.a ./stm32/Middleware/STM32_AI_Library/Lib/NetworkRuntime800_CM33_GCC.a
cp -rf ${mlpath}/C_header/* ./stm32/Projects/Common/dpu/
cp -rf ${mlpath}/stm32ai_files/network*.c ./stm32/Projects/Common/X-CUBE-AI/App/
cp -rf ${mlpath}/stm32ai_files/network*.h ./stm32/Projects/Common/X-CUBE-AI/App/
echo Done.