#!/bin/bash

# set this, if you have multiple projects compiling in parallel (should be unique)
PROJECT_ID="0"

THRILL_BUILDER_CONTAINER="thrill-builder"
THRILL_BUILDER_NAME=$THRILL_BUILDER_CONTAINER$PROJECT_ID

if [ ! "$(docker image ls | grep -q $THRILL_BUILDER_CONTAINER)" ]; then
  docker image build -t $THRILL_BUILDER_CONTAINER docker/
fi
docker run \
    --user "$UID":"$GID" \
    --mount type=bind,source="$(pwd)",target=/mnt/project \
    --name "$THRILL_BUILDER_NAME" \
    thrill-builder:latest \
    /bin/sh compile.sh
docker rm "$THRILL_BUILDER_NAME"
