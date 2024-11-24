#!/bin/bash

if [ "$#" -ne 5 ]; then
    echo "Usage: $0 <number_of_subscribers> <number_of_publishers> <load-balancer_ip> <load-balancer_port> <num_messages_per_publisher>"
    exit 1
fi

NUM_SUBS=$1
NUM_PUBS=$2
SERVER_IP=$3
SERVER_PORT=$4
MSGS=$5
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
    echo "Starting Subscriber #$i"
    ./client-sub.out $SERVER_IP $SERVER_PORT $i &
done

# { time bash ./loadgen-pub.sh $NUM_SUBS $NUM_PUBS $SERVER_IP $SERVER_PORT $MSGS } >> timetaken.txt
# time bash ./loadgen-pub.sh $NUM_SUBS $NUM_PUBS $SERVER_IP $SERVER_PORT $MSGS
{ time bash ./loadgen-pub.sh $NUM_SUBS $NUM_PUBS $SERVER_IP $SERVER_PORT $MSGS; } 2> time_output.txt

wait
echo "All clients have finished."
