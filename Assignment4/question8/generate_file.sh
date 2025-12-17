#!/bin/bash

# assuming 1 MB means 1 MiB like in rest of the course

# usage: ./generate_file.sh [filename]
# filename can be left out to use default name "file_to_map.txt" like this:
# ./generate_file.sh

# default filename
FILENAME=${1:-"file_to_map.txt"}
EXPECTED_SIZE=1048576  # 1 MiB in bytes

# remove existing file if it exists
if [ -f "$FILENAME" ]; then
    rm -f "$FILENAME"
fi

echo "Generating 1 MiB file named '$FILENAME'..."

# generate file
dd if=/dev/zero of="$FILENAME" bs=$EXPECTED_SIZE count=1 status=none

# verify size
ACTUAL_SIZE=$(stat -c%s "$FILENAME")

if [ "$ACTUAL_SIZE" -eq "$EXPECTED_SIZE" ]; then
    echo "   Success: '$FILENAME' created."
    echo "   Size: $ACTUAL_SIZE bytes (1 MiB)"
else
    echo "   Error: File size is $ACTUAL_SIZE bytes, expected $EXPECTED_SIZE."
    exit 1
fi