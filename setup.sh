#!/bin/bash

# CLIG Setup Script - Modernized

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Starting CLIG Setup...${NC}"

# OS Detection
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
else
    OS=$(uname -s)
fi

echo -e "Detected OS: ${YELLOW}$OS${NC}"

# Function to provide install instructions
get_install_cmd() {
    case $OS in
        ubuntu|debian|pop|mint)
            echo "sudo apt update && sudo apt install $1"
            ;;
        arch|manjaro)
            echo "sudo pacman -S $1"
            ;;
        fedora)
            echo "sudo dnf install $1"
            ;;
        *)
            echo "your package manager to install $1"
            ;;
    esac
}

# 1. Basic Build Tools
echo "Checking for requirements..."
for cmd in gcc make pkg-config; do
    if ! command -v $cmd &> /dev/null; then
        echo -e "${RED}Error: $cmd is not installed.${NC}"
        echo -e "Suggested command: $(get_install_cmd $cmd)"
        exit 1
    fi
done

# 2. Library Checks
echo "Checking for libraries..."
libraries=("ncurses")
for lib in "${libraries[@]}"; do
    if ! pkg-config --exists "$lib"; then
        echo -e "${YELLOW}Warning: $lib development files not found by pkg-config.${NC}"
        case $lib in
            ncurses) pkg="ncurses-devel (or libncurses-dev)";;
        esac
        echo -e "You might need to install: ${YELLOW}$pkg${NC}"
        echo -e "Try: $(get_install_cmd "$pkg")"
    fi
done

# 3. Python & Pokete Setup
if [ -d "pokete" ]; then
    echo "Setting up Pokete virtual environment..."
    PYTHON_CMD=""
    if command -v python3 &> /dev/null; then
        PYTHON_CMD="python3"
    elif command -v python &> /dev/null; then
        PYTHON_CMD="python"
    fi

    if [ -z "$PYTHON_CMD" ]; then
        echo -e "${RED}Error: Python 3 not found.${NC}"
        exit 1
    fi

    # Check for venv module
    if ! $PYTHON_CMD -m venv --help &> /dev/null; then
        echo -e "${RED}Error: python3-venv is missing.${NC}"
        echo -e "Try: $(get_install_cmd python3-venv)"
        exit 1
    fi

    $PYTHON_CMD -m venv pokete/venv
    ./pokete/venv/bin/pip install scrap_engine --quiet
    echo -e "${GREEN}Pokete virtual environment ready.${NC}"
fi

# 4. Global Build
echo -e "${GREEN}Building all games and launcher...${NC}"

declare -A build_status
GAMES=("2048" "snake" "tetris" "type")

# Build individual games
for game in "${GAMES[@]}"; do
    if [ -d "$game" ] && [ -f "$game/Makefile" ]; then
        echo -e "Building ${YELLOW}$game${NC}..."
        if (cd "$game" && make > /dev/null 2>&1); then
            build_status["$game"]="SUCCESS"
        else
            build_status["$game"]="FAILED"
        fi
    else
        build_status["$game"]="MISSING"
    fi
done

# Pokete Setup logic
if [ -d "pokete" ]; then
    if [ -f "pokete/pokete" ]; then
        chmod +x pokete/pokete
        build_status["pokete"]="READY (Script)"
    else
        build_status["pokete"]="MISSING SCRIPT"
    fi
fi

# Build launcher
echo -e "Building ${YELLOW}launcher${NC}..."
if gcc -Wall -Wextra launcher.c -o launcher -lncurses > /dev/null 2>&1; then
    build_status["launcher"]="SUCCESS"
else
    build_status["launcher"]="FAILED"
fi

# Summary Report
echo -e "\n${GREEN}--- Setup Summary ---${NC}"
for item in "${GAMES[@]}" "pokete" "launcher"; do
    status=${build_status[$item]}
    case $status in
        SUCCESS|READY*) color=$GREEN ;;
        *) color=$RED ;;
    esac
    printf "%-12s: [%b%-15s%b]\n" "$item" "$color" "$status" "$NC"
done
echo -e "${GREEN}----------------------${NC}"

if [ "${build_status["launcher"]}" == "SUCCESS" ]; then
    echo -e "Run ${YELLOW}./launcher${NC} to play."
else
    echo -e "${RED}Launcher build failed. Check dependencies (ncurses).${NC}"
fi


