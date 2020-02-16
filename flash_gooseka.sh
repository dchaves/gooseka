DEV=/dev/ttyUSB1; docker run -v $(pwd):/sketchbook -it --device=$DEV arduino-cli upload -p $DEV --fqbn esp32:esp32:esp32doit-devkit-v1 gooseka
