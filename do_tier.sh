#!/usr/local/bin/bash

TOP=$(expr 80 / 5)
BOTTOM=$((TOP-5))

docker-compose -f docker-compose.yml up &

sleep 10 && \
php ./ffxiv_craft.php -t all $BOTTOM $TOP > out.csv && \
tail -n 20 out.csv > out_fixed.csv && \
php ./ffxiv_required.php -f out_fixed.csv 20 > out_req.txt && \
docker-compose -f docker-compose.yml down

#docker-compose run --rm php php ./ffxiv_craft.php -t all $BOTTOM $TOP > out.csv && \
