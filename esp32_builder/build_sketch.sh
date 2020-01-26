FQBN=esp32:esp32:esp32doit-devkit-v1
SKETCHBOOK=${PWD}/$1
SKETCH=$2
docker run -v $SKETCHBOOK:/sketchbook -it arduino-cli compile --fqbn $FQBN $SKETCH
