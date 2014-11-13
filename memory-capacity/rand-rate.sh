shm-rm.sh
for i in `seq 0 7`; do
	tid=`expr $i + 1`
	core=`expr 2 \* $i`
	sudo numactl --physcpubind 0,2,4,6,8,10,12,14 --membind 0 ./rand-rate $tid &
done
