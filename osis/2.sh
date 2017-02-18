#!/bin/bash

kill $(ps aux | grep $1 | awk '{print $2}' | tr '\n' ' ')
