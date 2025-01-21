#!/bin/bash

# Define the path to the home directory dynamically
USER_HOME="$HOME"

# Ensure arguments for SERVER and NODE_ID are provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 SERVER NODE_ID"
    exit 1
fi

SERVER="$1"
NODE_ID="$2"

# Get the number of CPU cores
CORES=$(nproc)

# Wait for the network to be up
echo "Waiting for the network to come up"
echo "You have $CORES cores"

sleep 60

# Detect architecture
ARCH=$(uname -m)
if [[ "$ARCH" == "x86_64" ]]; then
    BINARY_URL="https://github.com/invpe/GRIDMAN/releases/latest/download/gmworker_linux64"
    BINARY_NAME="gmworker_linux64"
elif [[ "$ARCH" == "i686" || "$ARCH" == "i386" ]]; then
    BINARY_URL="https://github.com/invpe/GRIDMAN/releases/latest/download/gmworker_linux32"
    BINARY_NAME="gmworker_linux32"
elif [[ "$ARCH" == "aarch64" ]]; then
    BINARY_URL="https://github.com/invpe/GRIDMAN/releases/latest/download/gmworker_arm64"
    BINARY_NAME="gmworker_arm64"
elif [[ "$ARCH" == "armv7l" || "$ARCH" == "armhf" ]]; then
    BINARY_URL="https://github.com/invpe/GRIDMAN/releases/latest/download/gmworker_arm32"
    BINARY_NAME="gmworker_arm32"
else
    echo "Unsupported architecture: $ARCH"
    exit 1
fi

# Remove the old binary
rm -f "$USER_HOME/$BINARY_NAME"

# Download the latest binary
wget "$BINARY_URL" -O "$USER_HOME/$BINARY_NAME"
if [ $? -ne 0 ]; then
    echo "Failed to download binary from $BINARY_URL"
    exit 1
fi

# Make the binary executable
chmod +x "$USER_HOME/$BINARY_NAME"

# Start one instance per core in separate screen sessions
for ((i=0; i<CORES; i++)); do
    echo "Starting instance $i"
    screen -dmS gridmanworker_$i bash -c "cd $USER_HOME; while true; do ./$BINARY_NAME $SERVER $NODE_ID; done"
    screen -ls
    sleep 1
done
