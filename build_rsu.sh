#!/bin/bash
cd arduino-cli-esp32
docker build --tag arduino-cli-esp32 .
cd ..
docker run -it -v $(pwd)/gooseka-rsu:/github/workspace arduino-cli-esp32 compile esp32_gateway esp32:esp32:esp32doit-devkit-v1 ESP32Servo LoRa