if [ "$#" -ne 1 ]; then
    echo "Illegal number of parameters"
	echo "Usage: ./run-machine.sh <machine-id (0-based)>"
	exit
fi

sudo pkill ib_write_bw
sudo pkill ib_read_bw

port=`expr 3185 + $1`
ib_send_bw --connection=UD --port=$port &
