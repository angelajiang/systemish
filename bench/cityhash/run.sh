for i in `seq 0 7`; do
	taskset -c $i ./bench &
done
