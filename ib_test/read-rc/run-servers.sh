num_servers=2
size=8000000

for i in `seq 1 $num_servers`; do
	port=`expr 3185 + $i`
	ib_read_bw -F --size=$size --port=$port &
done
