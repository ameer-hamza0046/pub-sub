#!/bin/bash

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <number_of_clients> <server_ip> <server_port>"
    exit 1
fi

NUM_CLIENTS=$1
SERVER_IP=$2
SERVER_PORT=$3
gcc client-pub.c -o client-pub
if [ $? -ne 0 ]; then
    echo "Compilation of client-pub.c failed"
    exit 1
fi
gcc client-sub.c -o client-sub
if [ $? -ne 0 ]; then
    echo "Compilation of client-sub.c failed"
    exit 1
fi


echo "Starting $NUM_CLIENTS pubs and $NUM_CLIENTS subs to connect to $SERVER_IP:$SERVER_PORT"


for ((i = 1; i <= $NUM_CLIENTS; i++)); do
    echo "Starting Publisher #$i"
    ./client-pub $SERVER_IP $SERVER_PORT $i &
    echo "Starting Subscriber #$i"
    ./client-sub $SERVER_IP $SERVER_PORT $i &
done

wait
echo "All clients have finished."
