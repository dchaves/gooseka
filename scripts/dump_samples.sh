#!/bin/bash
docker exec -it influxdb influx -database gooseka -execute "SELECT * FROM left, right" -format csv
