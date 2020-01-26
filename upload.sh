DEV=/dev/ttyUSB0; docker run -v /home/dchaves/Workspace/gooseka:/sketchbook -it --device=/dev/ttyUSB1 arduino-cli upload -p /dev/ttyUSB1 --fqbn esp32:esp32:esp32doit-devkit-v1 lora_test 
