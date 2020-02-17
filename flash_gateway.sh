DEV=/dev/ttyUSB0; docker run -v $(pwd)/controller:/sketchbook -it --device=$DEV arduino-cli upload -p $DEV --fqbn esp32:esp32:esp32doit-devkit-v1 esp32_gateway
