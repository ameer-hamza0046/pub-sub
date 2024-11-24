NUM_SUBS=$1
NUM_PUBS=$2
SERVER_IP=$3
SERVER_PORT=$4
MSGS=$5

for ((i = 1; i <= $NUM_PUBS; i++)); do
    echo "Starting Publisher #$i"
    ./client-pub.out $SERVER_IP $SERVER_PORT $i $MSGS &
done

wait