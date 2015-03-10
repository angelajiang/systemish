if [ "$#" -ne 1 ]; then
    echo "Illegal number of parameters"
	echo "Usage: ./hugepages-create.sh <socket-id> <number of hugepages>"
	exit
fi

client_id=$1

# Connect to servers on IB port 1
ib_read_bw --ib-port=1 --size=32 --run_infinitely --duration=1 --port=`expr 20000 + 2 \* client_id` 10.113.211.51 &
ib_read_bw --ib-port=1 --size=32 --run_infinitely --duration=1 --port=`expr 20000 + 2 \* client_id + 1` 10.113.211.51 &

# Connect to servers on IB port 2
ib_read_bw --ib-port=1 --size=32 --run_infinitely --duration=1 --port=`expr 30000 + 2 \* client_id` 10.113.211.51 &
ib_read_bw --ib-port=1 --size=32 --run_infinitely --duration=1 --port=`expr 30000 + 2 \* client_id + 1` 10.113.211.51 &
