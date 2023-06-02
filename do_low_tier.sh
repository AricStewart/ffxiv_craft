#!/usr/local/bin/bash

TOP=$(expr 75 / 5)
BOTTOM=$((TOP-5))

docker-compose -f docker-compose.yml up &

sleep 10 && \
php ffxiv_craft.php -t all $BOTTOM $TOP > out.csv && \
docker-compose -f docker-compose.yml down
