#!/usr/local/bin/bash

TOP=$(expr 90 / 5)
BOTTOM=$((TOP-2))

docker-compose -f docker-compose.yml up &

sleep 10 && \
php ./ffxiv_craft.php -r all $BOTTOM $TOP > out.csv && \
docker-compose -f docker-compose.yml down
