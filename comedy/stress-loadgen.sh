#!/bin/bash

if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <number_of_clients> <server_ip> <server_port> <num_requests_per_client>"
    exit 1
fi

NUM_CLIENTS=$1
SERVER_IP=$2
SERVER_PORT=$3
NUM_REQUESTS=$4

gcc stress-client.c -o stress-client

if [ $? -ne 0 ]; then
    echo "Compilation of stress-client.c failed"
    exit 1
fi

echo "Starting $NUM_CLIENTS clients with $NUM_REQUESTS requests each to connect to $SERVER_IP:$SERVER_PORT"

for ((i = 1; i <= $NUM_CLIENTS; i++)); do
    ./stress-client $SERVER_IP $SERVER_PORT $NUM_REQUESTS &
done

wait
echo "All clients have finished."
