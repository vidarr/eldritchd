#!/bin/bash

LAST_BUILD=$(git tag -l "build-*" | tail -n 1 | sed -e "s/build-\(.*\)/\1/g")

if [ -z "$LAST_BUILD" ]; then
    LAST_BUILD=0
fi

LAST_BUILD=$(expr $LAST_BUILD + 1)
LAST_BUILD=$(printf "%06i" $LAST_BUILD)

git tag "build-${LAST_BUILD}"

echo $LAST_BUILD
