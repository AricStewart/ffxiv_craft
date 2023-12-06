#!/usr/local/bin/bash

TOP=$(expr 90 / 5)
BOTTOM=$((TOP-2))

PWD="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
CMD="cd $PWD"

if [ "$1" = "docker" ]; then
    CMD="$CMD && docker-compose -f docker-compose.yml up & sleep 10"
fi

CMD="$CMD && cd $PWD && php ./ffxiv_craft.php -r all $BOTTOM $TOP > roweena.csv"

if [ "$1" = "docker" ]; then
    CMD="$CMD && docker-compose -f docker-compose.yml down"
fi

eval $CMD
