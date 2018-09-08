#!/bin/bash


SCRIPT='./lab.sh dw'
DOCKER_O="-it --rm -v $PWD:/app"
IMG_NAME='clang:latest'

docker run $DOCKER_O $IMG_NAME $SCRIPT
