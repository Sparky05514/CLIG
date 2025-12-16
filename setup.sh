#!/bin/bash

# CLIG Setup Script

echo "Checking for requirements..."

if ! command -v gcc &> /dev/null; then
    echo "Error: gcc is not installed."
    exit 1
fi

if ! command -v make &> /dev/null; then
    echo "Error: make is not installed."
    exit 1
fi

echo "Compiling games..."

# Find directories containing a Makefile
for dir in */; do
    if [ -f "${dir}Makefile" ]; then
        echo "Building in $dir..."
        (cd "$dir" && make)
        if [ $? -ne 0 ]; then
            echo "Error building in $dir"
        fi
    fi
done

echo "Compiling launcher..."
if [ -f "launcher.c" ]; then
    gcc launcher.c -o launcher -lncurses
    if [ $? -eq 0 ]; then
        echo "Launcher compiled successfully."
    else
        echo "Error compiling launcher."
        exit 1
    fi
else
    echo "launcher.c not found!"
fi

echo "Setup complete! Run ./launcher to play."
