#!/bin/bash

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <number_of_clients> <server_ip> <server_port>"
    exit 1
fi

NUM_CLIENTS=$1
SERVER_IP=$2
SERVER_PORT=$3
gcc client.c -o client

if [ $? -ne 0 ]; then
    echo "Compilation of client.c failed"
    exit 1
fi

echo "Starting $NUM_CLIENTS clients to connect to $SERVER_IP:$SERVER_PORT"


for ((i = 1; i <= $NUM_CLIENTS; i++)); do
    echo "Starting client #$i"
    ./client $SERVER_IP $SERVER_PORT $i &
done

wait
echo "All clients have finished."
