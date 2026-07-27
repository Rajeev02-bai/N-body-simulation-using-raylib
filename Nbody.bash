#!/bin/bash

# Build script for Nbody.c using raylib (Linux / Kali)

SOURCE="Nbody.c"
OUTPUT="out"

INCLUDE_DIR="/usr/local/include"
LIB_DIR="/usr/local/lib"

gcc "$SOURCE" -o "$OUTPUT" \
    -I "$INCLUDE_DIR" \
    -L "$LIB_DIR" \
    -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

if [ $? -eq 0 ]; then
    echo "Build succeeded: ./$OUTPUT"
else
    echo "Build failed."
    exit 1
fi
