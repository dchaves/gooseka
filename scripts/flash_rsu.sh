#!/bin/bash
DEV="${1:-/dev/ttyUSB0}"

docker build --tag platformio platformio
docker run -it -v $(pwd)/gooseka-rsu:/project --device $DEV platformio run --target upload --upload-port $DEV