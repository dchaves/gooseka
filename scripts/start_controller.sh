#!/bin/bash
cd gooseka-controller
docker build --tag gooseka_controller . && docker run -it --privileged --network gooseka -v /dev/input:/dev/input --device=/dev/ttyUSB0 --env GOOSEKA='BENCHY' gooseka_controller
cd ..