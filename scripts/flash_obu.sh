#!/bin/bash
DEV=/dev/ttyUSB1

docker build --tag arduino-cli-esp32 arduino-cli-esp32
docker run -it --device=$DEV -v $(pwd)/gooseka-obu:/github/workspace arduino-cli-esp32 upload gooseka esp32:esp32:esp32doit-devkit-v1 $DEV