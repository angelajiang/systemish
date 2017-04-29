# A function to echo in blue color
function blue() {
	es=`tput setaf 4`
	ee=`tput sgr0`
	echo "${es}$1${ee}"
}

sudo pkill ib_write_bw
sudo pkill ib_read_bw

options="--size=16 --inline_size=16 --post_list=16 --run_infinitely --duration=2 --connection=UD"

# Server names

# APT
for i in `seq 0 10`; do
	mc=`expr $i + 2`
	mc_name="node-$mc.RDMA.fawn.apt.emulab.net"
	core=$i
	port=`expr $i + 3185`

	blue "Starting WRITEs to machine $mc_name"

	taskset -c $core ib_send_bw -F --port=$port $options $mc_name &
	sleep 10
done
