#!/bin/bash

# Variables
HOST_USER="$1"
HOST_IP="$2"
PORT="$3"
FILE_SIZE="$4"
THREADS="$5"
TOTAL_REQUESTS="$6"

# Function to SSH into farnet1 and run a script, then exit
ssh_into_host() {
    echo "Starting SSH into farnet1..."
    ssh $FARNET1_USER@$FARNET1_IP "bash -s" << 'EOF' &
    echo "Connected to farnet1..."
    cd transfer/tcp
    echo $FARNET1_SCRIPT
    pwd
    echo "Running server.sh..."
    ./server.sh
    exit
EOF
    echo "SSH session to farnet1 initiated."
}

# Run SSH into farnet1 in the background
ssh_into_host

# Give the server some time to start
sleep 20

# After the background process (SSH to farnet1) completes, continue on the DPU
# echo "SSH to host completed. Now starting the client on the DPU..."
# cd client
# gcc -o client client.c
# ./client $FARNET1_IP 8080 8 2 1 1000 