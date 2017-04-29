# This script checks the connectivity between xia-router2 and
# xia-router0/1. The interface xgeN on xia-router2 *must* be 
# connected to IP 10.0.N.1.

function blue() {
	es=`tput setaf 4`
	ee=`tput sgr0`
	echo -e "${es}$1${ee}"
}

for i in `seq 0 7`; do
	blue "Checking connectivity from xge$i"
	for j in `seq $i $i`; do
		blue "\tChecking connectivity for 10.0.$j.1"
		ping -I xge$i 10.0.$j.1 &
		sleep 2
		pkill ping
	done
done
