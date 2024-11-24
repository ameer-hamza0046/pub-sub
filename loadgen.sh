#!/bin/bash

if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <number_of_subscribers> <number_of_publishers> <server_ip> <server_port>"
    exit 1
fi

NUM_SUBS=$1
NUM_PUBS=$2
SERVER_IP=$3
SERVER_PORT=$4
gcc client-pub.c -o client-pub.out -lpthread
if [ $? -ne 0 ]; then
    echo "Compilation of client-pub.c failed"
    exit 1
fi
gcc client-sub.c -o client-sub.out -lpthread
if [ $? -ne 0 ]; then
    echo "Compilation of client-sub.c failed"
    exit 1
fi


echo "Starting $NUM_PUBS pubs and $NUM_SUBS subs to connect to $SERVER_IP:$SERVER_PORT"


for ((i = 1; i <= $NUM_PUBS; i++)); do
    echo "Starting Publisher #$i"
    ./client-pub.out $SERVER_IP $SERVER_PORT $i &
done
for ((i = 1; i <= $NUM_PUBS; i++)); do
    echo "Starting Subscriber #$i"
    ./client-sub.out $SERVER_IP $SERVER_PORT $i &
done

wait
echo "All clients have finished."
