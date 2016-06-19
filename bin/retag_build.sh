#!/bin/bash

CONFIG_FILE=$1
if [ -z "$CONFIG_FILE" ]; then
    echo "Need file to patch..."
    exit 1
fi

LAST_BUILD=$(git tag -l "build-*" | tail -n 1 | sed -e "s/build-\(.*\)/\1/g")

if [ -z "$LAST_BUILD" ]; then
    LAST_BUILD=0
fi

LAST_BUILD=$(expr $LAST_BUILD + 1)
LAST_BUILD=$(printf "%i" $LAST_BUILD)

git tag "build-${LAST_BUILD}"

TEMPFILE=$(tempfile)
cat $CONFIG_FILE | sed -e "s/#define BUILD_NUM.*/#define BUILD_NUM ${LAST_BUILD}/g" > $TEMPFILE
mv $TEMPFILE $CONFIG_FILE

exit 0
