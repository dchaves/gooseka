#!/bin/bash
docker build --tag platformio platformio
docker run -it -v $(pwd)/gooseka-rsu:/project -v platformio:/root/.platformio platformio run