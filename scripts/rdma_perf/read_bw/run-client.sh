# A function to echo in blue color
function blue() {
	es=`tput setaf 4`
	ee=`tput sgr0`
	echo "${es}$1${ee}"
}

if [ "$#" -ne 1 ]; then
    echo "Illegal number of parameters"
	echo "Usage: ./hugepages-create.sh <socket-id> <number of hugepages>"
	exit
fi

client_id=$1

blue "Pulling latest client script"
git pull

options="--size=32 --run_infinitely --duration=1 --post_list=16"
server="10.113.211.51"

# Connect to servers on IB port 1
ib_p1_port1=`expr 20000 + 2 \* $client_id`
ib_p1_port2=`expr 20000 + 2 \* $client_id + 1`
ib_p1_port3=`expr 20000 + 2 \* $client_id + 2`
ib_p1_port4=`expr 20000 + 2 \* $client_id + 3`
blue "Connecting to IB port 1 via socket ports $ib_p1_port1, $ib_p1_port2, $ib_p1_port3, $ib_p1_port4"

taskset -c 0 ib_read_bw $options --ib-port=1 --port=$ib_p1_port1 $server &
taskset -c 2 ib_read_bw $options --ib-port=1 --port=$ib_p1_port2 $server &
taskset -c 4 ib_read_bw $options --ib-port=1 --port=$ib_p1_port3 $server &
taskset -c 6 ib_read_bw $options --ib-port=1 --port=$ib_p1_port4 $server &

# Connect to servers on IB port 2
ib_p2_port1=`expr 30000 + 2 \* $client_id`
ib_p2_port2=`expr 30000 + 2 \* $client_id + 1`
ib_p2_port3=`expr 30000 + 2 \* $client_id + 2`
ib_p2_port4=`expr 30000 + 2 \* $client_id + 3`
blue "Connecting to IB port 2 via socket ports $ib_p2_port1, $ib_p2_port2, $ib_p2_port3, $ib_p2_port4"
taskset -c 8 ib_read_bw $options --ib-port=2 --port=$ib_p2_port1 $server &
taskset -c 10 ib_read_bw $options --ib-port=2 --port=$ib_p2_port2 $server &
taskset -c 12 ib_read_bw $options --ib-port=2 --port=$ib_p2_port3 $server &
taskset -c 14 ib_read_bw $options --ib-port=2 --port=$ib_p2_port4 $server &
