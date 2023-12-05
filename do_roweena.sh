#!/usr/local/bin/bash

TOP=$(expr 90 / 5)
BOTTOM=$((TOP-2))

CMD=""
if [ "$1" = "docker" ]; then
    CMD="docker-compose -f docker-compose.yml up & sleep 10 &&"
fi

CMD="$CMD php ./ffxiv_craft.php -r all $BOTTOM $TOP > roweena.csv"

if [ "$1" = "docker" ]; then
    CMD="$CMD && docker-compose -f docker-compose.yml down"
fi

eval $CMD
