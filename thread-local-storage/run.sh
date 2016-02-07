# A function to echo in blue color
function blue() {
	es=`tput setaf 4`
	ee=`tput sgr0`
	echo "${es}$1${ee}"
}

for i in `seq 0 10`; do
	blue "Running TLS version"
	numactl --physcpubind=0,2,4,6,8,10,12,14,16,18,20,22,24,26 --membind=0 ./tls --num-threads 8 --use-tls 1

	blue "Running non-TLS version"
	numactl --physcpubind=0,2,4,6,8,10,12,14,16,18,20,22,24,26 --membind=0 ./tls --num-threads 8 --use-tls 0
done
