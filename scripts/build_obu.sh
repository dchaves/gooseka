#!/bin/bash
docker build --tag arduino-cli-esp32 arduino-cli-esp32
docker run -it -v $(pwd)/gooseka-obu:/github/workspace arduino-cli-esp32 compile gooseka esp32:esp32:esp32doit-devkit-v1 ESP32Servo LoRa