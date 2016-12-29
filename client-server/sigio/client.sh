#!/bin/bash
# Usage: ./client.sh [destination host] [destination port]
# The destination host and port arguments are optional, and default to the
# presets below.

# A netcat-based UDP client

if [ "$#" -ne 0 ]; then
  echo "Illegal number of parameters"
  echo "Usage: ./client.sh"
  exit
fi

def_host=localhost
def_port=3490

HOST=${2:-$def_host}
PORT=${3:-$def_port}

# Use netcat to send $1 over IPv4 UDP (-4u) with a timeout of 1 second (-w1)
data=100000 # Start value of data to send
while true; do
  echo -n "$data" | nc -vvv -4u -w1 $HOST $PORT
  echo "Sent"
  sleep .1
  data=`expr $data + 1`
done

