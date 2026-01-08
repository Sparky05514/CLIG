#!/bin/bash

# CLIG Setup Script

echo "Checking for requirements..."

# 1. Basic Build Tools
for cmd in gcc make pkg-config; do
    if ! command -v $cmd &> /dev/null; then
        echo "Error: $cmd is not installed. Please install it using your package manager (e.g., sudo pacman -S $cmd)"
        exit 1
    fi
done

# 2. Library Checks
echo "Checking for libraries..."
libraries=("ncurses" "libvlc")
for lib in "${libraries[@]}"; do
    if ! pkg-config --exists "$lib"; then
        echo "Warning: $lib development files not found by pkg-config."
        echo "On Arch Linux: sudo pacman -S $lib"
        # We don't exit here as it might be installed but not in pkg-config path
    fi
done

# 3. Python & Pokete Setup
if [ -d "pokete" ]; then
    echo "Setting up Pokete virtual environment..."
    if ! command -v python &> /dev/null; then
        echo "Error: python is not installed."
    else
        python -m venv pokete/venv
        ./pokete/venv/bin/pip install scrap_engine
        echo "Pokete virtual environment ready."
    fi
fi

# 4. Global Build
echo "Building all games and launcher..."
if [ -f "Makefile" ]; then
    make all
else
    # Fallback to loop if root Makefile is missing
    for dir in 2048 mania snake tetris type; do
        if [ -d "$dir" ] && [ -f "$dir/Makefile" ]; then
            echo "Building in $dir..."
            (cd "$dir" && make)
        fi
    done
    
    echo "Building launcher..."
    gcc -Wall -Wextra launcher.c -o launcher -lncurses
fi

echo "---------------------------------------"
echo "Setup complete! Run ./launcher to play."
echo "---------------------------------------"
