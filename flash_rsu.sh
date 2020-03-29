#!/bin/bash
DEV=/dev/ttyUSB0

cd arduino-cli-esp32
docker build --tag arduino-cli-esp32 .
cd ..
docker run -it --device=$DEV -v $(pwd)/gooseka-rsu:/github/workspace arduino-cli-esp32 upload esp32_gateway esp32:esp32:esp32doit-devkit-v1 $DEV