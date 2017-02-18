#!/bin/bash

USER=$(id -un)
DATE=$(date)
PWD=$(pwd)
NPROC=$(ps aux | wc -l)

echo "User: $USER   Date: $DATE   Number of processes: $NPROC"
