FQBN=esp32:esp32:esp32doit-devkit-v1
SKETCHBOOK=${PWD}/"{$1:-sketchbook}"
SKETCH="{$2:-esp32_blink}"
DEV="{$3:-/dev/ttyUSB0}"
docker run -v $SKETCHBOOK:/sketchbook -it --device=$DEV arduino-cli upload -p $DEV --fqbn $FQBN $SKETCH
