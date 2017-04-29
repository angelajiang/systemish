if [ "$#" -ne 1 ]; then
    echo "Illegal number of parameters"
	echo "Usage: ./run-machine.sh <machine-id>"
	exit
fi

sudo pkill ib_write_bw
sudo pkill ib_read_bw

port=`expr $1 + 3185`
options="--size=16 --inline_size=16 --run_infinitely --duration=2 --connection=UC"

# Server names

# APT
server_name="node-1.RDMA.fawn.apt.emulab.net"

# Nome
# server_name="node-1.RDMA.fawn.nome.nx"

taskset -c 0 ib_write_bw -F --port=$port $options $server_name
