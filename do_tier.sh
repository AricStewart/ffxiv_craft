#!/usr/local/bin/bash

TOP=$(expr 80 / 5)
BOTTOM=$((TOP-5))

CMD=""
if [ "$1" = "docker" ]; then
    CMD="docker-compose -f docker-compose.yml up & sleep 10 && "
fi

CMD="$CMD php ./ffxiv_craft.php -t all $BOTTOM $TOP > tier.csv && \
tail -n 20 tier.csv > tier_fixed.csv && \
php ./ffxiv_required.php -f tier_fixed.csv 20 > tier_req.txt"

if [ "$1" = "docker" ]; then
    CMD="$CMD && docker-compose -f docker-compose.yml down"
fi

eval $CMD
