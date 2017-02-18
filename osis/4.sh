#!/bin/bash

NFILES=$(find . -type f | wc -l)
TOTAL=$(du -cs | grep total | awk '{print $1}')
AVERAGE=$(bc -l <<< "scale=2; $TOTAL/$NFILES")" bytes"

if (( $TOTAL > 1024*1024)); then
	TOTAL=$(bc -l <<< "scale=2; $TOTAL/1024")" kb";
else
	TOTAL="$TOTAL bytes";
fi;

echo "Total size: $TOTAL   Average size: $AVERAGE   File count: $NFILES"
