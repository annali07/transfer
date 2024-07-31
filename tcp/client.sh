#!/bin/bash
#!/bin/bash

# Variables
FARNET1_USER="your_farnet1_username"
FARNET1_IP="farnet1_ip_address"
FARNET1_SCRIPT="/home/annali/transfer/tcp/server.sh"
DPU_CLIENT_BINARY="./client"
DPU_CLIENT_ARGS="192.168.100.2 8080 8 2 1 1000"

# Function to SSH into farnet1 and run a script, then exit
ssh_into_host() {
  exit
  cd $FARNET1_SCRIPT
}

# Run SSH into farnet1 in the background
ssh_into_host &
# Capture the PID of the background process
FARNET1_PID=$!

# Wait for the background process to complete
wait $FARNET1_PID

# After the background process (SSH to farnet1) completes, continue on the DPU
echo "SSH to host completed. Now starting the client on the DPU..."
cd client
gcc -o client client.c
./client 192.168.100.2 8080 8 2 1 1000 