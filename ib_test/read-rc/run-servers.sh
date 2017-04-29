sudo pkill ib_read_bw
sudo pkill ib_write_bw

for i in `seq 0 10`; do
	port=`expr 3185 + $i`
	ib_read_bw --connection=RC --port=$port &
done
