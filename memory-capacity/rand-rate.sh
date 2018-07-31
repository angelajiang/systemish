drop_shm
for i in `seq 0 47`; do
	tid=`expr $i + 1`
	numactl --cpunodebind 0 --membind 0 ./rand-rate $tid &
done
