#!/bin/bash
# Quick build script for Nbody.c using raylib
gcc Nbody.c -o out -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

if [ $? -eq 0 ]; then
    echo "Build succeeded: ./out"
else
    echo "Build failed."
    exit 1
fi
