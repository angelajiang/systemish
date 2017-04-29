source ~/.bash_profile

drop_shm
for i in `seq 0 68`; do
	tid=`expr $i + 1`
	sudo numactl --physcpubind 0-67 --membind 0 ./rand-rate $tid &
done
