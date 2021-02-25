#!/bin/bash
docker build --tag platformio platformio
docker run -it -v $(pwd)/gooseka-obu:/project -v platformio:/root/.platformio platformio run