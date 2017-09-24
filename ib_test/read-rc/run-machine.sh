num_clients=2  # Number of clients to run on this machine
size=8000000
server_name="akaliaNode-1.RDMA.fawn.apt.emulab.net"

for i in `seq 1 $num_clients`; do
	port=`expr 3185 + $i`
  ib_read_bw -F --size=$size --port=$port --run_infinitely --duration=1 --tx-depth=1 $server_name &
done


