#!/bin/bash
docker network create --driver bridge gooseka || true
docker build --tag gooseka_controller gooseka-controller && docker run -it --privileged --network gooseka -v /dev/input:/dev/input --device=/dev/ttyUSB0 --env GOOSEKA='BENCHY' gooseka_controller